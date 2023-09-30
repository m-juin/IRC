#include <vector>
#include <iostream>
#include <sstream>
#include <iterator>
#include "StaticFunctions.hpp"
#include "User.hpp"

#ifndef PARSER_HPP
#define PARSER_HPP

enum Command {NONE = -1, PASS, USER, NICK, JOIN, MODE, WHO};

struct Parser
{
	public:
		Parser(std::list<User *> usrs, int fd, std::string fullCmd);
		~Parser();
		Command cmd;
		std::vector<std::string> args;
		User *op;
		std::vector<std::string>::iterator GetJoinPasswords();
	private:
		std::vector<std::string> SplitCmd(std::string str, const char *cs);
		Parser();
		Parser(Parser const &parser);
		Parser &operator=(Parser const &parser);
};

#endif