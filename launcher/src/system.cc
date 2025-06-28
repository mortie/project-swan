#include "system.h"

#include <iostream>
#include <sys/wait.h>
#include <spawn.h>

extern char **environ;

void quoteArg(std::string &cmd, std::string_view arg)
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

void appendArg(std::string &cmd, std::string_view arg)
{
	cmd += ' ';
	quoteArg(cmd, arg);
}

bool runCommand(const char *cmd)
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
