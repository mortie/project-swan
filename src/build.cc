#include <swan/log.h>
#include <swan/util.h>
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
#include <thread>
#include <cstring>
#include <sha1/sha1.hpp>
#include <process.hpp>

#ifdef SWAN_DYNLIB_EXT
#define DYNLIB_EXT SWAN_DYNLIB_EXT
#elif defined(__APPLE__)
#define DYNLIB_EXT ".dylib"
#else
#define DYNLIB_EXT ".so"
#endif

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define ASAN_ENABLED
#endif
#elif defined(__SANITIZE_ADDRESS__)
#define ASAN_ENABLED
#endif

namespace TPL = TinyProcessLib;

namespace SwanBuild {

struct BuildInfo {
	std::string compiler = SWAN_CXX_PATH;
	std::string modPath;
	std::string swanPath;
	std::vector<std::string> cflags;
	std::vector<std::string> ldflags;
	std::vector<std::string> includes;
	std::vector<std::string> libs;
	std::string buildID;
	int concurrency = std::thread::hardware_concurrency();
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

static std::string cmdToString(const std::span<const std::string> &cmd)
{
	std::string cmdStr = "";
	for (const auto &part: cmd) {
		if (cmdStr != "") {
			cmdStr += ' ';
		}
		cmdStr += '\'';
		cmdStr += part;
		cmdStr += '\'';
	}
	return cmdStr;
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

std::vector<std::string> buildCommand(
	const SourceFile &f, const BuildInfo &info)
{
	std::vector<std::string> cmd;
	switch (f.type) {
	case SourceType::SOURCE:
		cmd.push_back(info.compiler);
		for (auto &flag: info.cflags) {
			cmd.push_back(flag);
		}
#ifdef ASAN_ENABLED
		cmd.push_back("-fsanitize=address");
#endif
#if SWAN_CXX_IS_CLANG
		cmd.push_back("-g");
		cmd.push_back("-include-pch");
		cmd.push_back(Swan::cat(info.modPath, "/.swanbuild/swan.h.pch"));
#endif

		for (const auto &include: info.includes) {
			cmd.push_back("-I" + include);
		}

		cmd.push_back("-c");
		cmd.push_back("-o");
		cmd.push_back(f.outPath);
		cmd.push_back(f.srcPath);
		return cmd;

	case SourceType::HEADER:
		return cmd;

	case SourceType::PROTO:
		cmd.push_back("./bin/capnp");
		cmd.push_back("compile");

		for (const auto &include: info.includes) {
			cmd.push_back("-I" + include);
		}

		cmd.push_back("--src-prefix=" + f.srcDir);
		cmd.push_back("--output=./bin/capnpc-c++:" + f.outDir);
		cmd.push_back(f.srcPath);
		return cmd;
	}

	abort();
}

static std::shared_ptr<cpptoml::table> parseToml(const char *path)
{
	std::ifstream f(path);
	if (f.fail()) {
		throw std::runtime_error(Swan::cat("Could not find '", path, "'"));
	}

	cpptoml::parser parser(f);
	return parser.parse();
}

template<typename T>
static T getToml(cpptoml::table &table, const char *key) {
	auto obj = table.get_as<T>(key);
	if (!obj) {
		throw std::runtime_error(Swan::cat("Missing key '", key, "'"));
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
		Swan::warn << "Failed to read build manifest: " << ex.what() << '\n';
		return true;
	}
}

static void iterateSources(
	const std::string &srcRoot,
	const std::string &objRoot,
	std::string path,
	std::vector<SourceFile> &sources)
{
	auto fullPath = Swan::cat(srcRoot, path);
	for (auto const &entry: std::filesystem::directory_iterator(fullPath)) {
		auto name = entry.path().filename().string();
		if (entry.is_directory()) {
			iterateSources(srcRoot, objRoot, Swan::cat(path, name, "/"), sources);
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
			outDir = Swan::cat(objRoot, sanitizedPath);
			break;
		}

		auto objPath = Swan::cat(objRoot, sanitizedPath, name, objExt);
		std::optional<std::filesystem::file_time_type> objLastWrite;
		if (std::filesystem::exists(objPath)) {
			objLastWrite = std::filesystem::last_write_time(objPath);
		}

		sources.push_back({
			.srcName = name,
			.srcPath = Swan::cat(srcRoot, path, name),
			.srcLastWrite = std::filesystem::last_write_time(entry.path()),
			.srcDir = Swan::cat(srcRoot, path),
			.outPath = std::move(objPath),
			.outLastWrite = objLastWrite,
			.outDir = std::move(outDir),
			.type = type,
			.isOutdated = false,
		});
	}
}

static bool runCommand(const std::vector<std::string> &cmd)
{
	std::string output;

	auto receiveOutput = [&](const char *data, size_t len) {
		output += std::string_view(data, len);
	};

	TPL::Process proc(
		cmd, "",
		receiveOutput, // stdout
		receiveOutput // stderr
	);

	int status = proc.get_exit_status();
	if (status != 0) {
		Swan::warn << "Command failed with code " << status << "!";
		std::cerr << "$ " << cmdToString(cmd) << '\n';
		std::cerr << output << '\n';

		return false;
	}

	if (output != "") {
		Swan::info << "Command exited with output:";
		std::cerr << "$ " << cmdToString(cmd) << '\n';
		std::cerr << output << '\n';
	}

	return true;
}

static bool compile(const SourceFile &f, const BuildInfo &info)
{
	std::filesystem::create_directories(f.outDir);

	if (f.type == SourceType::HEADER) {
		std::fstream(f.outPath, std::fstream::out).close();
		return true;
	}

	auto cmd = buildCommand(f, info);
	if (!runCommand(cmd)) {
		return false;
	}

	if (f.type == SourceType::PROTO) {
		return compile({
			.srcName = Swan::cat(f.srcName, ".c++"),
			.srcPath = Swan::cat(f.outDir, "/", f.srcName, ".c++"),
			.srcLastWrite = {},
			.srcDir = f.outDir,
			.outPath = f.outPath,
			.outLastWrite = {},
			.outDir = f.outDir,
			.type = SourceType::SOURCE,
			.isOutdated = false,
		}, info);
	}

	return true;
}

static bool compilePCH(const BuildInfo &info, std::string_view pchPath)
{
#if SWAN_CXX_IS_CLANG
	std::vector<std::string> cmd;
	cmd.push_back(info.compiler);
	for (const auto &flag: info.cflags) {
		cmd.push_back(flag);
	}
	for (const auto &flag: info.includes) {
		cmd.push_back(Swan::cat("-I", flag));
	}
#ifdef ASAN_ENABLED
	cmd.push_back("-fsanitize=address");
#endif
	cmd.push_back("-xc++-header");
	cmd.push_back("-c");
	cmd.push_back("-o");
	cmd.push_back(std::string(pchPath));
	cmd.push_back(Swan::cat(info.swanPath, "/include/swan/swan.h"));
	return runCommand(cmd);
#else
	std::fstream(std::string(pchPath), std::fstream::out).close();
	return true;
#endif
}

static bool link(
	std::string_view out,
	std::span<const SourceFile> sources,
	const BuildInfo &info)
{
	std::vector<std::string> cmd;
	cmd.push_back(info.compiler);
#ifdef ASAN_ENABLED
	cmd.push_back("-fsanitize=address");
#endif
	cmd.push_back("-shared");
	cmd.push_back("-Llib");
	cmd.push_back("-o");
	cmd.push_back(std::string(out));
	for (const auto &flag: info.cflags) {
		cmd.push_back(flag);
	}

	for (const auto &source: sources) {
		if (source.type != SourceType::SOURCE) {
			continue;
		}

		cmd.push_back(source.outPath);
	}

	for (const auto &flag: info.ldflags) {
		cmd.push_back(flag);
	}

	for (const auto &lib: info.libs) {
		cmd.push_back(lib);
	}

	return runCommand(cmd);
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

			// Clang gets angry if the mtime of a header is newer
			// than the mtime of the PCH, so add the mtime to the hash
			auto mtime = std::filesystem::last_write_time(entry.path());
			using Sec = std::chrono::duration<int64_t>;
			auto mtimeSec = std::chrono::duration_cast<Sec>(
				mtime.time_since_epoch()).count();
			sha.update(std::to_string(mtimeSec));
		}
	}
}

static std::string hashFiles(std::string path)
{
	SHA1 sha;

	// We need to store whether asan is enabled or not
#ifdef ASAN_ENABLED
	sha.update("ASAN=1;");
#else
	sha.update("ASAN=0;");
#endif

	hashFiles(std::move(path), sha);
	return sha.final();
}

static void buildCompileDB(
	std::span<const SourceFile> sources, std::ostream &os, const BuildInfo &info)
{
	os << "[\n";

	std::string scratch;

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
		auto cmd = buildCommand(source, info);

		scratch.clear();
		quoteJSON(scratch, cmdToString(cmd));
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

static bool buildMod(const BuildInfo &info)
{
	auto startTime = std::chrono::steady_clock::now();

	auto modToml = parseToml(Swan::cat(info.modPath, "/mod.toml").c_str());
	auto modName = getToml<std::string>(*modToml, "name");
	auto modNamespace = getToml<std::string>(*modToml, "namespace");
	auto modVersion = getToml<std::string>(*modToml, "version");
	auto modLocked = modToml->get_as<bool>("locked");

	if (modLocked && *modLocked) {
		std::cerr << "Mod " << modName << " is locked. Not recompiling.\n";
		return true;
	}

	auto pchPath = Swan::cat(info.modPath, "/.swanbuild/swan.h.pch");
	auto manifestPath = Swan::cat(info.modPath, "/.swanbuild/manifest.toml");
	bool allOutdated = isOutdated(manifestPath, info);

	if (!std::filesystem::exists(Swan::cat(info.modPath, "/.swanbuild/mod.so"))) {
		allOutdated = true;
	}

	bool pchOutdated = allOutdated;
	if (allOutdated) {
		std::cerr << modName << " v" << modVersion << " needs to be recompiled.\n";
	}

	auto compileDBPath = Swan::cat(info.modPath, "/compile_commands.json");

	std::vector<SourceFile> sources;
	iterateSources(
		Swan::cat(info.modPath, "/proto/"),
		Swan::cat(info.modPath, "/.swanbuild/proto/"),
		"", sources);
	iterateSources(
		Swan::cat(info.modPath, "/src/"),
		Swan::cat(info.modPath, "/.swanbuild/obj/"),
		"", sources);

	if (!allOutdated) {
		bool someOutdated = false;
		for (auto &f: sources) {
			bool outdated =
				!f.outLastWrite ||
				(f.srcLastWrite - *f.outLastWrite).count() >= 0;
			if (!outdated) {
				continue;
			}

			std::cerr << f.srcPath << " outdated!\n";
			someOutdated = true;
			f.isOutdated = true;
			if (f.type != SourceType::SOURCE) {
				allOutdated = true;
			}
		}

		if (!someOutdated) {
			std::cerr << modName << " v" << modVersion << " is up to date.\n";
			return true;
		}
	}

	std::cerr << "Compiling " << modName << " v" << modVersion << "...\n";
	std::filesystem::create_directory(Swan::cat(info.modPath, "/.swanbuild"));

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
		std::cerr << "Failed to write " << compileDBPath << '\n';
		return false;
	}

	if (pchOutdated || !std::filesystem::exists(pchPath)) {
		std::cerr << "* Building PCH...\n";
		if (!compilePCH(info, pchPath)) {
			std::cerr << "Failed to compile PCH\n";
			return false;
		}
	}

	for (auto &f: sources) {
		if (failed) {
			break;
		}

		if (!allOutdated && !f.isOutdated) {
			continue;
		}

		// Check headers and proto without paralellization.
		// We do this with headers because they're a no-op.
		// We do this with protos because protos are a pre-requisite
		// for compiling source files.
		if (f.type == SourceType::HEADER || f.type == SourceType::PROTO) {
			if (!compile(f, info)) {
				failed = true;
				break;
			}
			continue;
		}

		while (numThreads >= info.concurrency) {
			cv.wait(lock);
		}

		numThreads += 1;
		std::cerr << "* Building " << f.srcName << "...\n";
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
		return false;
	}

	std::cerr << "* Linking...\n";
	link(Swan::cat(info.modPath, "/.swanbuild/mod.so"), sources, info);

	std::shared_ptr<cpptoml::table> newManifestRoot = cpptoml::make_table();
	newManifestRoot->insert("swan", info.buildID);
	std::fstream newManifest(manifestPath, std::fstream::out);
	newManifest << *newManifestRoot;
	if (newManifest.fail()) {
		std::filesystem::remove(manifestPath);
		std::cerr << "Failed to write " << manifestPath << '\n';
		return false;
	}

	auto delta = std::chrono::duration<double>(
		std::chrono::steady_clock::now() - startTime).count();
	Swan::info << "Compiled " << modName << " in " << delta << "s.\n";
	return true;
}

bool build(const char *modPath, const char *swanPath)
{
	std::vector<std::string> includes = {
		Swan::cat(modPath, "/proto"),
		Swan::cat(modPath, "/src"),
		Swan::cat(modPath, "/.swanbuild/proto"),
		Swan::cat(swanPath, "/include"),
		Swan::cat(swanPath, "/include/proto"),
	};

	std::vector<std::string> libs = {
		Swan::cat(swanPath, "/lib/libswan" DYNLIB_EXT),
		Swan::cat(swanPath, "/lib/libcygnet" DYNLIB_EXT),
		Swan::cat(swanPath, "/lib/libimgui" DYNLIB_EXT),
		Swan::cat(swanPath, "/lib/libscisasm" DYNLIB_EXT),
		Swan::cat(swanPath, "/lib/libscisavm" DYNLIB_EXT),
		Swan::cat(swanPath, "/lib/libkj" DYNLIB_EXT),
		Swan::cat(swanPath, "/lib/libcapnp" DYNLIB_EXT),
	};

	std::vector<std::string> cflags = {
		"-std=c++20",
		"-Wall",
		"-Werror",
		"-fPIC",
		"-O2",
	};

	return buildMod({
		.modPath = std::string(modPath),
		.swanPath = std::string(swanPath),
		.cflags = std::move(cflags),
		.ldflags = {},
		.includes = std::move(includes),
		.libs = std::move(libs),
		.buildID = hashFiles(Swan::cat(swanPath, "/include")),
	});
}

}
