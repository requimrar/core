// commands.cpp
// Copyright (c) 2019, zhiayang
// Licensed under the Apache License Version 2.0.

#include <sstream>

#include "defs.h"
#include "repl.h"
#include "ztmu.h"

#include "sst.h"
#include "ir/type.h"

namespace repl
{
	static void print_help();
	static void print_type(const std::string& expr);

	void runCommand(const std::string& s)
	{
		if(s == "q")
		{
			repl::log("exiting repl");

			exit(0);
		}
		else if(s == "reset")
		{
			repl::setupEnvironment();
			repl::log("environment reset");
		}
		else if(s == "help" || s == "?")
		{
			print_help();
		}
		else if(s.find("t ") == 0)
		{
			print_type(s.substr(2));
		}
		else
		{
			repl::error("invalid command '%s'", s);
		}
	}






	static void print_type(const std::string& line)
	{
		bool needmore = false;
		auto stmt = repl::parseAndTypecheck(line, &needmore);
		if(needmore)
		{
			repl::error("':t' does not support continuations");
		}
		else if(!stmt)
		{
			repl::error("invalid expression");
		}
		else if(auto expr = dcast(sst::Expr, *stmt))
		{
			zpr::println("%s%s%s: %s", COLOUR_GREY_BOLD, line, COLOUR_RESET, expr->type);
		}
		else
		{
			repl::error("'%s' is not an expression", (*stmt)->readableName);
		}
	}



	static std::vector<std::string> helpLines = {
		zpr::sprint(""),
		zpr::sprint("%s*%s overview %s*%s", COLOUR_GREEN_BOLD, COLOUR_RESET, COLOUR_GREEN_BOLD, COLOUR_RESET),
		zpr::sprint("\u203e\u203e\u203e\u203e\u203e\u203e\u203e\u203e\u203e\u203e\u203e\u203e"),
		zpr::sprint("The repl accepts Flax expressions and statements; press enter to evaluate the currently entered"
			" input. If the input was incomplete (eg. ending with a '{'), then the repl will enter a multi-line continuation"
			" mode. In either case, use the standard keybindings (arrow keys, home/end, etc.) to navigate."),
		zpr::sprint(""),
		zpr::sprint("Any definitions (eg. variables, functions) will be treated as if they were declared at global"
			" scope, while expressions and statements (eg. loops, arithmetic) will be treated as if they were"
			" written in a function body."),
		zpr::sprint(""),
		zpr::sprint("Expressions with values (eg. 3 + 1) will be given monotonic identifiers (eg. %s_0%s, %s_1%s) that"
			" can be used like any other identifier in code.", COLOUR_BLUE, COLOUR_RESET, COLOUR_BLUE, COLOUR_RESET),
		zpr::sprint(""),
		zpr::sprint("Commands begin with ':', and modify the state of the repl or perform other meta-actions."),

		zpr::sprint(""),
		zpr::sprint("%s*%s commands %s*%s", COLOUR_GREEN_BOLD, COLOUR_RESET, COLOUR_GREEN_BOLD, COLOUR_RESET),
		zpr::sprint("\u203e\u203e\u203e\u203e\u203e\u203e\u203e\u203e\u203e\u203e\u203e\u203e"),
		zpr::sprint(" :? / :help    -  display help (this listing)"),
		zpr::sprint(" :q            -  quit the repl"),
		zpr::sprint(" :reset        -  reset the environment, discarding all existing definitions"),
		zpr::sprint(" :t <expr>     -  display the type of an expression"),
	};

	static void print_help()
	{
		auto split_words = [](std::string& s) -> std::vector<std::string_view> {
			std::vector<std::string_view> ret;

			size_t word_start = 0;
			for(size_t i = 0; i < s.size(); i++)
			{
				if(s[i] == ' ')
				{
					ret.push_back(std::string_view(s.c_str() + word_start, i - word_start));
					word_start = i + 1;
				}
				else if(s[i] == '-')
				{
					ret.push_back(std::string_view(s.c_str() + word_start, i - word_start + 1));
					word_start = i + 1;
				}
			}

			ret.push_back(std::string_view(s.c_str() + word_start));
			return ret;
		};

		constexpr const char* LEFT_MARGIN = " ";
		constexpr const char* RIGHT_MARGIN = " ";

		auto tw = ztmu::getTerminalWidth();
		tw = std::min(tw, tw - strlen(LEFT_MARGIN) - strlen(RIGHT_MARGIN));

		auto disp_len = ztmu::displayedTextLength;

		for(auto& l : helpLines)
		{
			size_t remaining = tw;

			// sighs.
			auto ss = std::stringstream();
			ss << LEFT_MARGIN;

			// each "line" is actually a paragraph. we want to be nice, so pad on the right by a few spaces
			// and hyphenate split words.

			// first split into words
			auto words = split_words(l);
			for(const auto& word : words)
			{
				// printf("w: '%s' -- ", std::string(word).c_str());

				auto len = disp_len(word);
				if(remaining >= len)
				{
					ss << word << (word.back() != '-' ? " " : "");

					// printf("norm (%zu)", remaining);
					remaining -= len;

					if(remaining > 0)
					{
						remaining -= 1;
					}
					else
					{
						ss << "\n" << LEFT_MARGIN;
						remaining = tw;
					}

					// printf("(%zu)\n", remaining);
				}
				else if(remaining < 3 || len < 5)
				{
					// for anything less than 5 chars, put it on the next line -- don't hyphenate.
					ss << "\n" << LEFT_MARGIN << word << (word.back() != '-' ? " " : "");

					// printf("next (%zu)", remaining);
					remaining = tw - (len + 1);
					// printf("(%zu)\n", remaining);
				}
				else
				{
					// split it.
					ss << word.substr(0, remaining - 2) << "-" << "\n";
					ss << LEFT_MARGIN << word.substr(remaining - 2) << " ";

					// printf("split (%zu)", remaining);
					remaining = tw - word.substr(remaining - 2).size();
					// printf("(%zu)\n", remaining);
				}
			}

			zpr::println(ss.str());
		}
	}

}
