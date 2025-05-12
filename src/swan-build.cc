#include <atomic>
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
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

struct BuildInfo {
	std::string compiler = "clang++";
	std::string modPath;
	std::string cflags;
	std::string ldflags;
	std::vector<std::string> includes;
	std::vector<std::string> libs;
	std::string swanVersion;
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
		if (getToml<std::string>(*manifest, "swan") != info.swanVersion) {
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

		const char *objExt;
		switch (type) {
		case SourceType::SOURCE:
			objExt = ".o";
			break;
		case SourceType::HEADER:
			objExt = ".stamp";
			break;
		case SourceType::PROTO:
			objExt = ".o";
			break;
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
			.outDir = cat(objRoot, sanitizedPath),
			.type = type,
		});
	}
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

static void appendArg(std::string &cmd, std::string_view arg)
{
	cmd += ' ';
	quoteArg(cmd, arg);
}

static void compile(const SourceFile &f, const BuildInfo &info)
{
	std::filesystem::create_directories(f.outDir);

	std::string cmd;
	switch (f.type) {
	case SourceType::SOURCE:
		quoteArg(cmd, info.compiler);
		appendArg(cmd, "-std=c++20");
		appendArg(cmd, "-Wall");
		appendArg(cmd, "-Werror");
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
		std::fstream(f.outPath, std::fstream::out).close();
		return;
		break;

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

	if (std::system(cmd.c_str()) != 0) {
		throw std::runtime_error("Compile failed! Command: " + cmd);
	}

	if (f.type == SourceType::PROTO) {
		compile({
			.srcPath = cat(f.outDir, "/", f.srcName, ".c++"),
			.srcDir = f.outDir,
			.outPath = f.outPath,
			.outDir = f.outDir,
			.type = SourceType::SOURCE,
		}, info);
	}

	exit(0);
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

	if (std::system(cmd.c_str()) != 0) {
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

static void buildMod(const BuildInfo &info)
{
	auto modToml = parseToml(cat(info.modPath, "/mod.toml").c_str());
	auto modName = getToml<std::string>(*modToml, "name");
	auto modNamespace = getToml<std::string>(*modToml, "namespace");
	auto modVersion = getToml<std::string>(*modToml, "version");

	auto manifestPath = cat(info.modPath, "/.swanbuild/manifest.toml");
	bool allOutdated = isOutdated(manifestPath, info);
	if (allOutdated) {
		std::cerr << modName << " v" << modVersion << " needs to be recompiled.\n";
	}

	std::vector<SourceFile> sources;
	iterateSources(cat(info.modPath, "/proto/"), cat(info.modPath, "/.swanbuild/proto/"), "", sources);
	iterateSources(cat(info.modPath, "/src/"), cat(info.modPath, "/.swanbuild/obj/"), "", sources);

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
	std::condition_variable cv;

	for (auto &f: sources) {
		if (!allOutdated && !f.isOutdated) {
			continue;
		}

		if (f.type == SourceType::HEADER) {
			compile(f, info);
			continue;
		}

		while (numThreads >= info.concurrency) {
			cv.wait(lock);
		}

		std::cerr << "* Building " << f.outPath << "...\n";
		numThreads += 1;
		std::thread([&numThreads, &cv, &mut, &info, f]  {
			pid_t child = fork();
			if (child < 0) {
				throw std::runtime_error("Fork failed");
			} else if (child != 0) {
				int stat;
				waitpid(child, &stat, 0);
				if (WEXITSTATUS(stat) != 0) {
					throw std::runtime_error("Compile failed!");
				}
			} else {
				compile(f, info);
				exit(0);
			}

			mut.lock();
			numThreads -= 1;
			mut.unlock();
			cv.notify_all();
		}).detach();
	}

	while (numThreads > 0) {
		cv.wait(lock);
	}

	std::cerr << "* Linking...\n";
	link(cat(info.modPath, "/.swanbuild/mod.so"), sources, info);

	std::shared_ptr<cpptoml::table> newManifestRoot = cpptoml::make_table();
	newManifestRoot->insert("swan", info.swanVersion);
	std::fstream newManifest(manifestPath, std::fstream::out);
	newManifest << *newManifestRoot;
	if (newManifest.fail()) {
		std::filesystem::remove(manifestPath);
		throw std::runtime_error(cat("Failed to write ", manifestPath));
	}
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " <mod> <swan>\n";
		return 1;
	}

	std::vector<const char *> pkgs = {
		"kj",
		"capnp",
		"glfw3",
	};

	buildMod({
		.modPath = argv[1],
		.cflags = pkgconfig("--cflags", pkgs),
		.ldflags = pkgconfig("--libs", pkgs),
		.includes = {
			cat(argv[1], "/proto"),
			cat(argv[1], "/src"),
			cat(argv[1], "/.swanbuild/proto"),
			cat(argv[2], "/include"),
			cat(argv[2], "/libcygnet/include"),
			cat(argv[2], "/libswan/include"),
			cat(argv[2], "/libswan/proto"),
			cat(argv[2], "/build/libswan"),
			cat(argv[2], "/third-party/PerlinNoise"),
			cat(argv[2], "/third-party"),
		},
		.libs = {
			cat(argv[2], "/build/libswan/libswan.dylib"),
			cat(argv[2], "/build/libcygnet/libcygnet.dylib"),
		},
		.swanVersion = "0.0.1",
	});
}
