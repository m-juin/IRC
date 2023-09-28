#include "Parser.hpp"

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
	if (fullCmd.size() < 3)
	{
		std::cerr << "Unknown command: \"" << fullCmd << "\"\n";
		return;
	}
	std::stringstream ss(fullCmd);
	std::istream_iterator<std::string> begin(ss);
	std::istream_iterator<std::string> end;
	std::vector<std::string> vstrings(begin, end);
	std::copy(vstrings.begin(), vstrings.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
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
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(usrs, fd);
	if (usrIt == usrs.end())
	{
		std::cerr << "Unknown user fd: \"" << fd << "\"\n";
		return;
	}
	op = *usrIt;
	std::vector<std::string>::iterator it = vstrings.begin();
	std::vector<std::string>::iterator ite = vstrings.end();
	while (++it != ite)
	{
		args.push_back(*it);
	}
	std::cout << "User = " << op->getUserNickname() << ".\nCommand = " << cmd << ".\nArgs = ";
	it = args.begin();
	ite = args.end();
	while (it != ite)
	{
		std::cout << *it << std::endl;
		it++;
	}
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
