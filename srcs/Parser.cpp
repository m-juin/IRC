#include "Parser.hpp"
#include <cstring>

Command getCmdEnum(std::string arg)
{
	std::string validCmd[] = {"PASS", "USER", "NICK", "JOIN", "MODE", "WHO", "PART", "QUIT"};
	size_t size = sizeof(validCmd) / sizeof(std::string);
	for (size_t i = 0; i < size; i++)
	{
		if (validCmd[i] == arg)
			return (Command)i;
	}
	return NONE;
	
}

Parser::Parser(std::list<User *> usrs, int fd, std::string fullCmd)
{
	fullCmd = fullCmd.substr(0, fullCmd.size() - 2);
	if (fullCmd.size() < 3)
	{
		std::cerr << "Unknown command: \"" << fullCmd << "\"\n";
		return;
	}
	std::vector<std::string> vstrings = SplitCmd(fullCmd, " ");
	if (vstrings.size() < 1)
	{
		std::cerr << "Unknown command: \"" << fullCmd << "\"\n";
		return;
	}
	_cmd = getCmdEnum(vstrings[0]);
	if (_cmd < 0)
	{
		std::cerr << "Unknown command: \"" << vstrings[0] << "\"\n";
		return;
	}
	else if (_cmd == 3)
	{
		std::vector<std::string>::iterator it = vstrings.begin();
		std::vector<std::string> tmp = SplitCmd(*++it, ",");
		std::vector<std::string> tmp2;
		if (++it != vstrings.end())
			tmp2 = SplitCmd(*it, ",");
		std::size_t pos = 0;
		while (pos < tmp.size())
		{
			_joinArgs.push_back(std::pair<std::string, std::string>(tmp[pos], (pos < tmp2.size() ? tmp2[pos] : "")));
			pos++;
		}
	}
	else
	{
		std::vector<std::string>::iterator it = vstrings.begin();
		std::vector<std::string>::iterator ite = vstrings.end();
		while (++it != ite)
			_args.push_back(*it);
	}
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(usrs, fd);
	if (usrIt == usrs.end())
	{
		std::cerr << "Unknown user fd: \"" << fd << "\"\n";
		return;
	}
	_op = *usrIt;
}

std::vector<std::string> Parser::SplitCmd(std::string str, const char *cs)
{
	std::vector<std::string> Parsed;
	char *token = strtok((char *)(str.c_str)(), cs);
    while (token != NULL)
    {
        Parsed.push_back(static_cast<std::string>(token));
        token = strtok(NULL, cs);
    }
	return Parsed;
}

Parser::Parser()
{
}

Parser::Parser(const Parser &parser)
{
	(void)parser;
	return ;
}

Parser::~Parser()
{
}

Command Parser::getCmd()
{
	return _cmd;
}

std::vector<std::string> Parser::getArgs()
{
	return _args;
}

std::vector<std::pair<std::string, std::string> > Parser::getJoinArgs()
{
	return _joinArgs;
}

User *Parser::getOperator()
{
	return _op;
}

Parser &Parser::operator=(const Parser &parser)
{
	if (this == &parser)
		return *this;
	return *this;
}
