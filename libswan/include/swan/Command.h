#pragma once

#include <span>
#include <vector>
#include <swan/util.h>

#include "common.h"

namespace Swan {

using CommandHandler = void (*)(Ctx &ctx, std::span<CowStr> params, std::string &out);

struct CommandSpec {
	std::vector<const char *> pattern;
	const char *help;
	CommandHandler handler;
};

struct CommandSet {
	std::string name;
	std::vector<CommandSpec> commands;
};

// Tokenize a command string into whitespace-separated parts.
// Populates 'out'.
// The elements in 'out' may contain sub-views into the input 'string'.
bool tokenizeCommand(
	std::string_view string,
	std::vector<CowStr> &out);

// Match a tokenized argv against a pattern.
// Populates 'out'.
// The elements in 'out' reference the input 'argv'.
bool matchCommand(
	std::span<const char *> pattern,
	std::span<CowStr> argv,
	std::vector<CowStr> &out);

}
