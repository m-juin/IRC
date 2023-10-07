#include "Parser.hpp"
#include <cstring>

Command getCmdEnum(std::string arg)
{
	std::string validCmd[] = {"PASS", "USER", "NICK", "JOIN", "MODE", "WHO", "PART", "QUIT", "PRIVMSG", "TOPIC", "KICK", "INVITE", "LIST"};
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
	std::vector<std::string> lines;
	if (fullCmd[fullCmd.size() - 1] == '\r')
		fullCmd = fullCmd.substr(0, fullCmd.size() - 2);
	else
		fullCmd = fullCmd.substr(0, fullCmd.size() - 1);
	/*if (fullCmd[0] == '/')
		fullCmd = fullCmd.substr(1, fullCmd.size() - 1);*/
	lines = SplitCmd(fullCmd, "\r\n");
	for (std::vector<std::string>::iterator i = lines.begin(); i != lines.end(); i++)
	{
		if ((*i)[0] == '/')
			continue;
		std::vector<std::string> words = SplitCmd(*i, " ");
		Command cmd = getCmdEnum(words[0]);
		if (cmd == JOIN || cmd == PART)
		{
			std::vector<std::string> channels;
			std::vector<std::string> pwds;
			if (words.size() > 1)
				channels = SplitCmd(words[1], ",");
			else
			{
				std::cerr << "/JOIN Error: need at least one channel to join\n";
				return ;
			}
			if (words.size() > 2)
				pwds = SplitCmd(words[2], ",");
			for (size_t y = 0; y < channels.size(); y++)
			{
				std::string msg;
				msg += channels[y];
				if (pwds.size() > y)
					msg += " " + pwds[y];
				_args.push_back(std::make_pair(cmd, msg));
			}
		}
		else if (cmd == TOPIC || cmd == PRIVMSG)
		{
			std::size_t pos = (*i).find_first_of(' ');
			if (pos == std::string::npos)
			{
				StaticFunctions::SendToFd(fd, ERR_NEEDMOREPARAMS(words[0]), 0);
				return;
			}
			_args.push_back(std::make_pair(cmd, (*i).substr(pos)));
		}
		else
		{
			std::string msg;
			size_t y = 1;
			while (y < words.size())
			{
				msg += words[y];
				y++;
				if (y < words.size())
					msg += " ";
			}
			_args.push_back(std::make_pair(cmd, msg));
		}
	}
	for (size_t i = 0; i < _args.size(); i++)
	{
		std::cout << "CMD = " << _args[i].first << "\n	ARGS = " << _args[i].second << std::endl;
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


std::vector<std::pair<Command, std::string> > Parser::getArgs()
{
	return _args;
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
