#pragma once

#include <string>
#include <string_view>

void quoteArg(std::string &cmd, std::string_view arg);
void appendArg(std::string &cmd, std::string_view arg);
bool runCommand(const char *cmd);
