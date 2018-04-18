#include <regex>
#include <string>
#include <vector>
#include <cctype>
#include <utility>
#include <iomanip>
#include <iostream>
#include "replxx.hxx"
#include "../cpp-readline/src/Console.hpp"

using Replxx = replxx::Replxx;
namespace cr = CppReadline;
using ret = cr::Console::ReturnCode;


// prototypes
Replxx::completions_t hook_completion(std::string const& context, int index, void* user_data);
Replxx::hints_t hook_hint(std::string const& context, int index, Replxx::Color& color, void* user_data);
void hook_color(std::string const& str, Replxx::colors_t& colors, void* user_data);

Replxx::completions_t hook_completion(std::string const& context, int index, void* user_data) {
	auto* examples = static_cast<std::vector<std::string>*>(user_data);
	Replxx::completions_t completions;

	std::string prefix {context.substr(index)};
	for (auto const& e : *examples) {
		if (e.compare(0, prefix.size(), prefix) == 0) {
			completions.emplace_back(e.c_str());
		}
	}

	return completions;
}

Replxx::hints_t hook_hint(std::string const& context, int index, Replxx::Color& color, void* user_data) {
	auto* examples = static_cast<std::vector<std::string>*>(user_data);
	Replxx::hints_t hints;

	// only show hint if prefix is at least 'n' chars long
	// or if prefix begins with a specific character
	std::string prefix {context.substr(index)};
	if (prefix.size() >= 2 || (! prefix.empty() && prefix.at(0) == '.')) {
		for (auto const& e : *examples) {
			if (e.compare(0, prefix.size(), prefix) == 0) {
				hints.emplace_back(e.substr(prefix.size()).c_str());
			}
		}
	}

	// set hint color to green if single match found
	if (hints.size() == 1) {
		color = Replxx::Color::GREEN;
	}

	return hints;
}

void hook_color(std::string const& context, Replxx::colors_t& colors, void* user_data) {
	auto* regex_color = static_cast<std::vector<std::pair<std::string, Replxx::Color>>*>(user_data);

	// highlight matching regex sequences
	for (auto const& e : *regex_color) {
		size_t pos {0};
		std::string str = context;
		std::smatch match;

		while(std::regex_search(str, match, std::regex(e.first))) {
			std::string c {match[0]};
			pos += std::string(match.prefix()).size();

			for (size_t i = 0; i < c.size(); ++i) {
				colors.at(pos + i) = e.second;
			}

			pos += c.size();
			str = match.suffix();
		}
	}
}

unsigned info(const std::vector<std::string> &) {
	std::cout << "Welcome to the example console. This command does not really\n"
			  << "do anything aside from printing this statement. Thus it does\n"
			  << "not need to look into the arguments that are passed to it.\n";
	return ret::Ok;
}


int main() {


    cr::Console c("\x1b[1;32mcb-cli\x1b[0m> ");

    // registering commands
    c.registerCommand(".info", info, "this command shows the manual for this application.");


    // words to be completed
	std::vector<std::string> examples;

	// highlight specific words
	// a regex string, and a color
	// the order matters, the last match will take precedence
	using cl = Replxx::Color;
	std::vector<std::pair<std::string, cl>> regex_color {
		// single chars
		{"\\`", cl::BRIGHTCYAN},
		{"\\'", cl::BRIGHTBLUE},
		{"\\\"", cl::BRIGHTBLUE},
		{"\\-", cl::BRIGHTBLUE},
		{"\\+", cl::BRIGHTBLUE},
		{"\\=", cl::BRIGHTBLUE},
		{"\\/", cl::BRIGHTBLUE},
		{"\\*", cl::BRIGHTBLUE},
		{"\\^", cl::BRIGHTBLUE},
		{"\\.", cl::BRIGHTMAGENTA},
		{"\\(", cl::BRIGHTMAGENTA},
		{"\\)", cl::BRIGHTMAGENTA},
		{"\\[", cl::BRIGHTMAGENTA},
		{"\\]", cl::BRIGHTMAGENTA},
		{"\\{", cl::BRIGHTMAGENTA},
		{"\\}", cl::BRIGHTMAGENTA},

		// color keywords
		{"color_black", cl::BLACK},
		{"color_red", cl::RED},
		{"color_green", cl::GREEN},
		{"color_brown", cl::BROWN},
		{"color_blue", cl::BLUE},
		{"color_magenta", cl::MAGENTA},
		{"color_cyan", cl::CYAN},
		{"color_lightgray", cl::LIGHTGRAY},
		{"color_gray", cl::GRAY},
		{"color_brightred", cl::BRIGHTRED},
		{"color_brightgreen", cl::BRIGHTGREEN},
		{"color_yellow", cl::YELLOW},
		{"color_brightblue", cl::BRIGHTBLUE},
		{"color_brightmagenta", cl::BRIGHTMAGENTA},
		{"color_brightcyan", cl::BRIGHTCYAN},
		{"color_white", cl::WHITE},
		{"color_normal", cl::NORMAL},

		// commands
		{"\\.help", cl::BRIGHTMAGENTA},
		{"\\.history", cl::BRIGHTMAGENTA},
		{"\\.quit", cl::BRIGHTMAGENTA},
		{"\\.exit", cl::BRIGHTMAGENTA},
		{"\\.clear", cl::BRIGHTMAGENTA},
		{"\\.prompt", cl::BRIGHTMAGENTA},

		// numbers
		{"[\\-|+]{0,1}[0-9]+", cl::YELLOW}, // integers
		{"[\\-|+]{0,1}[0-9]*\\.[0-9]+", cl::YELLOW}, // decimals
		{"[\\-|+]{0,1}[0-9]+e[\\-|+]{0,1}[0-9]+", cl::YELLOW}, // scientific notation

		// strings
		{"\".*?\"", cl::BRIGHTGREEN}, // double quotes
		{"\'.*?\'", cl::BRIGHTGREEN}, // single quotes
	};


    for(auto k: c.getRegisteredCommands()) {
        examples.push_back(k);
        regex_color.push_back({k, cl::BRIGHTMAGENTA});
    }

	// init the repl
	Replxx rx;
	rx.install_window_change_handler();

	// the path to the history file
	std::string history_file {"./replxx_history.txt"};

	// load the history file if it exists
	rx.history_load(history_file);

	// set the max history size
	rx.set_max_history_size(12);

	// set the max input line size
	rx.set_max_line_size(128);

	// set the max number of hint rows to show
	rx.set_max_hint_rows(8);

	// set the callbacks
	rx.set_completion_callback(hook_completion, static_cast<void*>(&examples));
	rx.set_highlighter_callback(hook_color, static_cast<void*>(&regex_color));
	rx.set_hint_callback(hook_hint, static_cast<void*>(&examples));

	// display initial welcome message
	std::cout
	<< "Welcome to confidential-bank client\n"
    << "version[confidential-bank]:CB-0.0.1\n"
	<< "Press 'tab' to view autocompletions\n"
	<< "Type '.help' for help \n"
	<< "Type '.quit' or '.exit' to exit\n\n";

	// set the repl prompt
	std::string prompt {"\x1b[1;32mcb-cli\x1b[0m> "};


    int retCode;

	// main repl loop
	for (;;) {
		// display the prompt and retrieve input from the user
        char const* cinput = rx.input(prompt);

		if (cinput == nullptr) {
			// reached EOF

			std::cout << "\n";
			break;
		}

		retCode = c.readLine(cinput);
        if(retCode == ret::Quit) {
            rx.history_add(std::string{cinput});
            break;
        } else
        {
			if(retCode >= 1)
				std::cout << "Error in reading commands. use '.help' command for more information." << std::endl;
            rx.history_add(std::string{cinput});
            continue;
        }
	}

	// save the history
	rx.history_save(history_file);

	std::cout << "\nExiting cb-cli\n";
}
