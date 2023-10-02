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
		std::vector<std::string> getArgs();
		std::vector<std::pair<std::string, std::string> > getJoinArgs();
		User * getOperator();
	private:
		std::vector<std::string> SplitCmd(std::string str, const char *cs);
		Parser();
		Parser(Parser const &parser);
		Parser &operator=(Parser const &parser);
		Command _cmd;
		std::vector<std::string> _args;
		std::vector<std::pair<std::string, std::string> > _joinArgs;
		User *_op;
		std::string _fullCmd;
};

#endif