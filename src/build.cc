#include <algorithm>
#include <condition_variable>
#include <cpptoml.h>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <optional>
#include <sys/wait.h>
#include <spawn.h>
#include <thread>
#include <unistd.h>
#include <string.h>
#include <sha1/sha1.hpp>

#ifdef __APPLE__
#define DYNLIB_EXT ".dylib"
#else
#define DYNLIB_EXT ".so"
#endif

extern char **environ;

namespace SwanBuild {

struct BuildInfo {
	std::string compiler = "clang++";
	std::string modPath;
	std::string cflags;
	std::string ldflags;
	std::vector<std::string> includes;
	std::vector<std::string> libs;
	std::string buildID;
	int concurrency = 8;
};

enum class SourceType {
	SOURCE, HEADER, PROTO,
};

struct SourceFile {
	std::string srcName;
	std::string srcPath;
	std::filesystem::file_time_type srcLastWrite;
	std::string srcDir;
	std::string outPath;
	std::optional<std::filesystem::file_time_type> outLastWrite;
	std::string outDir;
	SourceType type;
	bool isOutdated;
};

// Concatinate strings
template<typename ...Args>
static inline std::string cat(Args &&... args)
{
	std::string_view strs[] = {std::forward<Args>(args)...};

	size_t size = 0;

	for (auto &s: strs) {
		size += s.size();
	}

	std::string buf;
	buf.resize(size);
	size_t idx = 0;
	for (auto &s: strs) {
		memcpy(&buf[idx], s.data(), s.size());
		idx += s.size();
	}

	return buf;
}

static void quoteArg(std::string &cmd, std::string_view arg)
{
	cmd.reserve(cmd.size() + arg.size() + 8);
	cmd += '\'';
	for (char ch: arg) {
		if (ch == '\'') {
			cmd += "'\"'\"'";
		} else {
			cmd += ch;
		}
	}
	cmd += '\'';
}

static void quoteJSON(std::string &json, std::string_view str)
{
	json.reserve(json.size() + str.size() + 8);
	json += '\"';
	for (char ch: str) {
		if (ch == '\"') {
			json += "\\\"";
		} else if (ch == '\\') {
			json += "\\\\";
		} else {
			json += ch;
		}
	}
	json += '\"';
}

static void appendArg(std::string &cmd, std::string_view arg)
{
	cmd += ' ';
	quoteArg(cmd, arg);
}

void buildCommand(std::string &cmd, const SourceFile &f, const BuildInfo &info)
{
	switch (f.type) {
	case SourceType::SOURCE:
		quoteArg(cmd, info.compiler);
		appendArg(cmd, "-std=c++20");
		appendArg(cmd, "-Wall");
		appendArg(cmd, "-Werror");
		appendArg(cmd, "-fPIC");
		appendArg(cmd, "-O2");
		cmd += info.cflags;

		for (const auto &include: info.includes) {
			cmd += " -I";
			quoteArg(cmd, include);
		}

		cmd += " -c -o";
		quoteArg(cmd, f.outPath);
		appendArg(cmd, f.srcPath);
		break;

	case SourceType::HEADER:
		return;

	case SourceType::PROTO:
		quoteArg(cmd, "capnp");
		appendArg(cmd, "compile");

		for (const auto &include: info.includes) {
			cmd += " -I";
			quoteArg(cmd, include);
		}

		appendArg(cmd, "--src-prefix=");
		quoteArg(cmd, f.srcDir);
		appendArg(cmd, "--output=c++:");
		quoteArg(cmd, f.outDir);
		appendArg(cmd, f.srcPath);
		break;
	}
}

static std::shared_ptr<cpptoml::table> parseToml(const char *path)
{
	std::ifstream f(path);
	if (f.fail()) {
		throw std::runtime_error(cat("Could not find '", path, "'"));
	}

	cpptoml::parser parser(f);
	return parser.parse();
}

template<typename T>
static T getToml(cpptoml::table &table, const char *key) {
	auto obj = table.get_as<T>(key);
	if (!obj) {
		throw std::runtime_error(cat("Missing key '", key, "'"));
	}

	return *obj;
}

static bool isOutdated(const std::string &manifestPath, const BuildInfo &info)
{
	try {
		std::ifstream f(manifestPath);
		if (f.fail()) {
			return true;
		}

		cpptoml::parser parser(f);
		auto manifest = parser.parse();
		if (getToml<std::string>(*manifest, "swan") != info.buildID) {
			return true;
		}

		return false;
	} catch (std::exception &ex) {
		std::cerr << "Failed to read build manifest: " << ex.what() << '\n';
		return true;
	}
}

static void iterateSources(
	const std::string &srcRoot,
	const std::string &objRoot,
	std::string path,
	std::vector<SourceFile> &sources)
{
	for (auto const &entry: std::filesystem::directory_iterator(cat(srcRoot, path))) {
		auto name = entry.path().filename().string();
		if (entry.is_directory()) {
			iterateSources(srcRoot, objRoot, cat(path, name, "/"), sources);
			continue;
		}

		SourceType type;
		if (name.ends_with(".cpp") || name.ends_with(".cc") || name.ends_with(".cpp")) {
			type = SourceType::SOURCE;
		} else if (name.ends_with(".h") || name.ends_with("hh") || name.ends_with(".hpp")) {
			type = SourceType::HEADER;
		} else if (name.ends_with(".capnp")) {
			type = SourceType::PROTO;
		} else {
			continue;
		}

		std::string sanitizedPath;
		sanitizedPath.reserve(path.size() + 10);
		for (char ch: path) {
			if (ch == '/' || ch == '\\') {
				sanitizedPath.push_back('_');
				sanitizedPath.push_back('_');
			} else {
				sanitizedPath.push_back(ch);
			}
		}

		const char *objExt;
		std::string outDir;
		switch (type) {
		case SourceType::SOURCE:
			objExt = ".o";
			outDir = objRoot;
			break;
		case SourceType::HEADER:
			objExt = ".stamp";
			outDir = objRoot;
			break;
		case SourceType::PROTO:
			objExt = ".o";
			outDir = cat(objRoot, sanitizedPath);
			break;
		}

		auto objPath = cat(objRoot, sanitizedPath, name, objExt);
		std::optional<std::filesystem::file_time_type> objLastWrite;
		if (std::filesystem::exists(objPath)) {
			objLastWrite = std::filesystem::last_write_time(objPath);
		}

		sources.push_back({
			.srcName = name,
			.srcPath = cat(srcRoot, path, name),
			.srcLastWrite = std::filesystem::last_write_time(entry.path()),
			.srcDir = cat(srcRoot, path),
			.outPath = std::move(objPath),
			.outLastWrite = objLastWrite,
			.outDir = std::move(outDir),
			.type = type,
			.isOutdated = true,
		});
	}
}

static bool runCommand(const char *cmd)
{
	pid_t pid = 0;
	const char *argv[] = {"/bin/sh", "-c", cmd, nullptr};
	if (posix_spawn(&pid, argv[0], nullptr, nullptr, (char *const *)argv, environ) < 0) {
		std::cerr << "Failed to run command: " << cmd << '\n';
		return false;
	}

	int stat = 0;
	waitpid(pid, &stat, 0);
	if (WIFEXITED(stat)) {
		int code = WEXITSTATUS(stat);
		if (code != 0) {
			std::cerr << "Command exited with non-zero code " << code << ": " << cmd << '\n';
			return false;
		}

		return true;
	}

	return false;
}

static bool compile(const SourceFile &f, const BuildInfo &info)
{
	std::filesystem::create_directories(f.outDir);

	if (f.type == SourceType::HEADER) {
		std::fstream(f.outPath, std::fstream::out).close();
		return true;
	}

	std::string cmd;
	buildCommand(cmd, f, info);

	if (!runCommand(cmd.c_str())) {
		return false;
	}

	if (f.type == SourceType::PROTO) {
		return compile({
			.srcName = cat(f.srcName, ".c++"),
			.srcPath = cat(f.outDir, "/", f.srcName, ".c++"),
			.srcLastWrite = {},
			.srcDir = f.outDir,
			.outPath = f.outPath,
			.outLastWrite = {},
			.outDir = f.outDir,
			.type = SourceType::SOURCE,
			.isOutdated = true,
		}, info);
	}

	return true;
}

static void link(
	std::string_view out,
	std::span<const SourceFile> sources,
	const BuildInfo &info)
{
	std::string cmd;
	quoteArg(cmd, info.compiler);
	cmd += " -shared -o";
	quoteArg(cmd, out);
	cmd += info.cflags;
	for (const auto &source: sources) {
		if (source.type != SourceType::SOURCE) {
			continue;
		}

		appendArg(cmd, source.outPath);
	}
	cmd += info.ldflags;

	for (const auto &lib: info.libs) {
		appendArg(cmd, lib);
	}

	if (!runCommand(cmd.c_str())) {
		throw std::runtime_error("Link failed! Command: " + cmd);
	}
}

std::string pkgconfig(const char *arg, std::vector<const char *> pkgs)
{
	if (pkgs.empty()) {
		return "";
	}

	std::string cmd = "pkg-config";
	appendArg(cmd, arg);
	for (auto &pkg: pkgs) {
		appendArg(cmd, pkg);
	}

	FILE *f = popen(cmd.c_str(), "r");
	if (!f) {
		throw std::runtime_error("Failed to run pkg-config command");
	}

	char buf[32];
	std::string cflags = " ";
	while (!feof(f)) {
		size_t n = fread(buf, 1, sizeof(buf), f);
		cflags.append(buf, n);
	}

	if (pclose(f) != 0) {
		throw std::runtime_error("pkg-config failed! Command: " + cmd);
	}

	for (size_t i = 0; i < cflags.size(); ++i) {
		if (cflags[i] == '\n') {
			cflags.resize(i);
			break;
		}
	}

	return cflags;
}

static void hashFiles(std::string path, SHA1 &sha)
{
	std::vector<std::filesystem::directory_entry> entries;
	for (auto entry: std::filesystem::directory_iterator(path)) {
		entries.push_back(std::move(entry));
	}
	std::sort(entries.begin(), entries.end());

	for (auto const &entry: entries) {
		if (entry.is_directory()) {
			hashFiles(entry.path().string(), sha);
		} else {
			sha.update(SHA1::from_file(entry.path().string()));
		}
	}
}

static std::string hashFiles(std::string path)
{
	SHA1 sha;
	hashFiles(std::move(path), sha);
	return sha.final();
}

static void buildCompileDB(
	std::span<const SourceFile> sources, std::ostream &os, const BuildInfo &info)
{
	os << "[\n";

	std::string scratch;
	std::string cmd;

	std::string cwd;
	quoteJSON(cwd, std::filesystem::current_path().string());

	bool first = true;
	for (const auto &source: sources) {
		if (source.type != SourceType::SOURCE) {
			continue;
		}

		if (!first) {
			os << ", ";
		}
		first = false;

		os << "{\n  \"directory\":" << cwd;
		cmd.clear();
		buildCommand(cmd, source, info);

		scratch.clear();
		quoteJSON(scratch, cmd);
		os << ",\n  \"command\":" << scratch;

		scratch.clear();
		quoteJSON(scratch, source.srcPath);
		os << ",\n  \"file\":" << scratch;

		scratch.clear();
		quoteJSON(scratch, source.outPath);
		os << ",\n  \"output\":" << scratch;

		os << "\n}";
	}

	os << "\n]";
}

static void buildMod(const BuildInfo &info)
{
	auto startTime = std::chrono::steady_clock::now();

	auto modToml = parseToml(cat(info.modPath, "/mod.toml").c_str());
	auto modName = getToml<std::string>(*modToml, "name");
	auto modNamespace = getToml<std::string>(*modToml, "namespace");
	auto modVersion = getToml<std::string>(*modToml, "version");

	auto manifestPath = cat(info.modPath, "/.swanbuild/manifest.toml");
	bool allOutdated = isOutdated(manifestPath, info);
	if (allOutdated) {
		std::cerr << modName << " v" << modVersion << " needs to be recompiled.\n";
	}

	auto compileDBPath = cat(info.modPath, "/compile_commands.json");

	std::vector<SourceFile> sources;
	iterateSources(
		cat(info.modPath, "/proto/"),
		cat(info.modPath, "/.swanbuild/proto/"),
		"", sources);
	iterateSources(
		cat(info.modPath, "/src/"),
		cat(info.modPath, "/.swanbuild/obj/"),
		"", sources);

	if (!allOutdated) {
		bool someOutdated = false;
		for (auto &f: sources) {
			if (!f.outLastWrite) {
				allOutdated = true;
				break;
			}

			if ((f.srcLastWrite - *f.outLastWrite).count() >= 0) {
				std::cerr << f.srcPath << " outdated!\n";
				someOutdated = true;
				f.isOutdated = true;
				if (f.type != SourceType::SOURCE) {
					allOutdated = true;
				}
			}
		}

		if (!someOutdated) {
			std::cerr << modName << " v" << modVersion << " is up to date.\n";
			return;
		}
	}

	std::cerr << "Compiling " << modName << " v" << modVersion << "...\n";
	std::filesystem::remove(manifestPath);

	std::mutex mut;
	std::unique_lock lock(mut);

	int numThreads = 0;
	bool failed = false;
	std::condition_variable cv;

	// Create compile_commands.json first,
	// so we make all of it even if there are compile errors
	std::fstream compileDB(compileDBPath, std::ios::out);
	buildCompileDB(sources, compileDB, info);
	compileDB.close();
	if (compileDB.fail()) {
		throw std::runtime_error(cat("Failed to write ", compileDBPath));
	}

	for (auto &f: sources) {
		if (failed) {
			break;
		}

		if (!allOutdated && !f.isOutdated) {
			continue;
		}

		if (f.type == SourceType::HEADER) {
			if (!compile(f, info)) {
				break;
			}
			continue;
		}

		while (numThreads >= info.concurrency) {
			cv.wait(lock);
		}

		std::cerr << "* Building " << f.outPath << "...\n";
		numThreads += 1;
		std::thread([&numThreads, &cv, &mut, &info, &failed, f]  {
			bool ok = compile(f, info);

			mut.lock();
			numThreads -= 1;
			if (!ok) {
				failed = true;
			}
			mut.unlock();
			cv.notify_all();
		}).detach();
	}

	while (numThreads > 0) {
		cv.wait(lock);
	}

	if (failed) {
		exit(1);
	}

	std::cerr << "* Linking...\n";
	link(cat(info.modPath, "/.swanbuild/mod.so"), sources, info);

	std::shared_ptr<cpptoml::table> newManifestRoot = cpptoml::make_table();
	newManifestRoot->insert("swan", info.buildID);
	std::fstream newManifest(manifestPath, std::fstream::out);
	newManifest << *newManifestRoot;
	if (newManifest.fail()) {
		std::filesystem::remove(manifestPath);
		throw std::runtime_error(cat("Failed to write ", manifestPath));
	}

	auto delta = std::chrono::duration<double>(
		std::chrono::steady_clock::now() - startTime).count();
	std::cout << "Compiled " << modName << " in " << delta << "s.\n";
}

void build(const char *modPath, const char *swanPath)
{
	std::vector<const char *> pkgs = {
		"kj",
		"capnp",
		"glfw3",
	};

	buildMod({
		.modPath = std::string(modPath),
		.cflags = pkgconfig("--cflags", pkgs),
		.ldflags = pkgconfig("--libs", pkgs),
		.includes = {
			cat(modPath, "/proto"),
			cat(modPath, "/src"),
			cat(modPath, "/.swanbuild/proto"),
			cat(swanPath, "/include"),
			cat(swanPath, "/include/proto"),
		},
		.libs = {
			cat(swanPath, "/lib/libswan" DYNLIB_EXT),
			cat(swanPath, "/lib/libcygnet" DYNLIB_EXT),
		},
		.buildID = hashFiles(cat(swanPath, "/include")),
	});
}

}
