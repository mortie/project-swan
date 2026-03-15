#include "Command.h"
#include "swan/util.h"
#include <optional>
#include <string_view>

namespace Swan {

namespace {

class Reader {
public:
	Reader(std::string_view str): str_(str) {}

	char peek() {
		if (eof()) {
			return '\0';
		}

		return str_[index_];
	}

	void consume() {
		if (!eof()) {
			index_ += 1;
		}
	}

	bool eof() {
		return index_ >= str_.size();
	}

	size_t index() { return index_; }

	std::string_view view(size_t start, size_t end) {
		return std::string_view(str_.data() + start, end - start);
	}

private:
	std::string_view str_;
	size_t index_ = 0;
};

bool isWhitespace(char ch) {
	return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t';
}

std::optional<CowStr> parseString(Reader &r, char quote)
{
	// Skip the opening quote
	r.consume();

	size_t start = r.index();
	bool hasEscapes = false;

	// Scan through the string until the closing quote,
	// keeping track of whether we have escapes
	char ch;
	while (!r.eof() && (ch = r.peek()) != quote) {
		if (ch == '\\') {
			r.consume();
			hasEscapes = true;
		}
		r.consume();
	}

	if (r.eof()) {
		return std::nullopt;
	}

	size_t end = r.index();
	r.consume();

	// If we don't have escapes, we can just return the string view
	auto view = r.view(start, end);
	if (!hasEscapes) {
		return view;
	}

	// Since we have escapes, we need to build a whole new string
	std::string output;
	output.reserve(view.size());
	Reader r2(view);
	while (!r2.eof()) {
		char ch = r2.peek();
		if (ch == '\\') {
			r2.consume();
			ch = r2.peek();
			if (ch == '\\' || ch == '\'' || ch == '"') {
				output += ch;
			} else {
				return std::nullopt;
			}
		} else {
			output += ch;
		}
		r2.consume();
	}

	// NRVO doesn't apply here
	return std::move(output);
}

}

bool tokenizeCommand(
	std::string_view string,
	std::vector<CowStr> &out)
{
	Reader r(string);

	while (true) {
		while (!r.eof() && isWhitespace(r.peek())) {
			r.consume();
		}

		if (r.eof()) {
			return true;
		}

		// Quoted tokens
		char ch = r.peek();
		if (ch == '"' || ch == '\'') {
			auto part = parseString(r, ch);
			if (!part) {
				return false;
			}

			out.push_back(std::move(*part));
			continue;
		}

		// Un-quoted tokens
		size_t start = r.index();
		while (!r.eof() && !isWhitespace(r.peek())) {
			r.consume();
		}
		out.push_back(r.view(start, r.index()));
	}
}

bool matchCommand(
	std::span<const char *> pattern,
	std::span<CowStr> argv,
	std::vector<CowStr> &out)
{
	if (pattern.size() == 0 || argv.size() == 0) {
		return false;
	}

	size_t patternIndex = 0;
	size_t argvIndex = 0;
	while (patternIndex < pattern.size() && argvIndex < argv.size()) {
		std::string_view part = pattern[patternIndex++];
		std::string_view arg = argv[argvIndex++];

		bool isParam = part.starts_with("@");
		if (isParam && part.ends_with("...")) {
			out.push_back(arg);
			while (argvIndex < argv.size()) {
				out.push_back(argv[argvIndex++].str());
			}
			return true;
		} else if (isParam) {
			out.push_back(arg);
		} else if (arg != part) {
			return false;
		}
	}

	return argvIndex == argv.size();
}

}
