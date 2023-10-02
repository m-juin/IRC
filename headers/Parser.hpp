#include <vector>
#include <iostream>
#include <sstream>
#include <iterator>
#include "StaticFunctions.hpp"
#include "User.hpp"

#ifndef PARSER_HPP
#define PARSER_HPP

enum Command {NONE = -1, PASS, USER, NICK, JOIN, MODE, WHO, PART, QUIT, PRIVMSG, TOPIC, KICK, INVITE, OPER, LIST};

struct Parser
{
	public:
		Parser(std::list<User *> usrs, int fd, std::string fullCmd);
		~Parser();
		Command getCmd();
		std::vector<std::pair<Command, std::string> > getArgs();
		User * getOperator();
		static std::vector<std::string> SplitCmd(std::string str, const char *cs);
		
	private:
		
		Parser();
		Parser(Parser const &parser);
		Parser &operator=(Parser const &parser);
		std::vector<std::pair<Command, std::string> > _args;
		User *_op;
};

#endif