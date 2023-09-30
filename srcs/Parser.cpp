#include "Parser.hpp"
#include <cstring>

Command getCmdEnum(std::string arg)
{
	std::string validCmd[] = {"PASS", "USER", "NICK", "JOIN", "MODE", "WHO"};
	size_t size = sizeof(validCmd) / sizeof(std::string);
	size_t i = 0;
	for (; i < size; i++)
	{
		if (validCmd[i] == arg)
			return (Command)i;
	}
	return NONE;
	
}

Parser::Parser(std::list<User *> usrs, int fd, std::string fullCmd)
{
	fullCmd = fullCmd.substr(0, fullCmd.size() - 2);
	std::cout << fullCmd << std::endl;
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
	cmd = getCmdEnum(vstrings[0]);
	if (cmd < 0)
	{
		std::cerr << "Unknown command: \"" << vstrings[0] << "\"\n";
		return;
	}
	else if (cmd == 3)
	{
		std::vector<std::string> tmp;
		std::vector<std::string>::iterator it = vstrings.begin();
		args = SplitCmd(*++it, ",");
		if (++it != vstrings.end())
			args2 = SplitCmd(*it, ",");
	}
	else
	{
		std::vector<std::string>::iterator it = vstrings.begin();
		std::vector<std::string>::iterator ite = vstrings.end();
		while (it != ite)
		{
			args.push_back(*it);
			it++;
		}
	}
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(usrs, fd);
	if (usrIt == usrs.end())
	{
		std::cerr << "Unknown user fd: \"" << fd << "\"\n";
		return;
	}
	op = *usrIt;
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

Parser &Parser::operator=(const Parser &parser)
{
	if (this == &parser)
		return *this;
	return *this;
}
