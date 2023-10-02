#include "../headers/Server.hpp"
#include "../headers/Channel.hpp"
#include "../headers/User.hpp"
#include "StaticFunctions.hpp"
#include <sys/select.h>
#include <string>
#include <sstream>


Server::Server(std::size_t port, std::string password)
{
	_port = port;
	_password = password;
	_channelNumber = 0;
	for (int i = 0; i < 1000; i++) {
        _socket.push_back(-1);
    }

	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd < 0)
    {
        std::cerr << "socket error" << std::endl;
        return ;
    }
    int opt = 1;
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        std::cerr << "setsockopt error" << std::endl;
        return ;
    }
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(port);
	if (bind(_serverFd,(struct sockaddr*)&_address, sizeof(_address)) < 0) 
    {
        std::cerr << "bind error" << std::endl;
        return ;
    }
    if (listen(_serverFd, 3) < 0)
    {
        std::cerr << "listen error" << std::endl;
        return ;
    }
}

void Server::launch()
{
	char buff[1024];	

    std::memset(buff, 0, 1024);
	_socket[0] = _serverFd;

	while (true)
	{
		fd_set rfds = addNewSocket();
		if (_serverFd == -1)
			return ;
		for (int i = 1; i < 1000; i++)
		{
			int valread = 0;
			Parser *Parsedcmd;
			if (_socket[i] > 0 && FD_ISSET(_socket[i], &rfds))
			{
				valread = recv(_socket[i], buff, 1024, 0);
				if (valread <= 0)
					continue ;
				Parsedcmd = new Parser(_users, _socket[i], buff);
				std::memset(buff, 0, 1024);
			}
			else
				continue ;
			for (std::size_t j = 0; j < Parsedcmd->getArgs().size(); j++)
			{
				switch (Parsedcmd->getArgs()[j].first)
				{
					case JOIN:{
						if (isUserCorrectlyConnected(i) == false)
						{
							StaticFunctions::SendToFd(_socket[i], "You don't have permission to execute commands", "", 0);
							continue;
						}
						joinChannel(Parsedcmd->getArgs()[j], i);
						break;
					}

					case PASS:{	
						checkPass(Parsedcmd->getArgs()[j], i);
						break;
					}

					case NICK:{
						setNickname(i, Parsedcmd->getArgs()[j]);
						break;
					}

					case USER:{
						setUsername(i, Parsedcmd->getArgs()[j]);
						break;
					}

					case PART:{
						leaveChannel(i, Parsedcmd->getArgs()[j]);
						break;
					}

					case QUIT:{
						std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
						if (usrIt != _users.end())
							(*usrIt)->setFd(-1);
						close(_socket[i]);
						_socket[i] = -1;
						break;
					}
					
					//	creer privmsgChannel et privmsg
					case PRIVMSG:
					{
						messageChannel(i, Parsedcmd->getArgs()[j], Parsedcmd->getOperator());
						break;
					}

					default:
						break;
				}
			}
		}	
	}	
}	

void Server::checkPass(std::pair<Command, std::string>cmd, int i)
{	
	if (isUserAuthenticated(i) == true)
	{	
		StaticFunctions::SendToFd(_socket[i], "You're already authenticated", "", 0);
		return	;
	}
	if (cmd.second == _password)
	{
		StaticFunctions::SendToFd(_socket[i], "Password OK\r\n", "Now authenticate you with /NICK /USER", 0);
		std::list<User *>::iterator it = _users.end();
		int id;
		if (_users.end() != _users.begin())
		{
			it--;
			id = (*it)->getId();
		}
		else
			id = 0;
		_users.push_back(new User(id, "", "",_socket[i]));
	}
	else
		StaticFunctions::SendToFd(_socket[i], ":Password incorrect", "", 0);
}


void Server::joinChannel(std::pair<Command, std::string>cmd, int i)
{
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	std::list<Channel *>::iterator it = find(_channels.begin(), _channels.end(), cmd.second);
	if (it != _channels.end())
	{
		StaticFunctions::SendToFd(_socket[i], "You joined channel ", cmd.second, 0);
		(*it)->addUser(*usrIt);
	}
	else if (it == _channels.end())
	{
		Channel *c = new Channel(_channelNumber, cmd.second, *usrIt);
		_channelNumber++;
		_channels.push_back(c);
		it--;
	}
	std::string message = ":" + (*usrIt)->getNickname() + " JOIN " + (*it)->getName() + "\r\n";
	send(_socket[i], message.c_str(), message.size(), 0);
}

void Server::leaveChannel(int i, std::pair<Command, std::string> cmd)
{
	std::vector<std::string> v = Parser::SplitCmd(cmd.second, " ");
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	std::list<Channel *>::iterator it = find(_channels.begin(), _channels.end(), v[0]);
	std::string message;
	if (it == _channels.end())
		message = cmd.second + " :No such channel\r\n";
	else
	{
		(*it)->leaveUser(*usrIt);
		message = ":" + (*usrIt)->getNickname() + " PART " + (*it)->getName() + "\r\n";
	}
	send(_socket[i], message.c_str(), message.size(), 0);
	std::cout << message << std::endl;
}

fd_set Server::addNewSocket()
{
	fd_set rfds;
	int addrlen = sizeof(_address);

	FD_ZERO(&rfds);
	for (int i = 0; i < 1000; i++)
	{
		if (_socket[i] >= 0)
			FD_SET(_socket[i], &rfds);
	}
	switch (select(FD_SETSIZE,  &rfds, NULL, NULL, NULL))
	{
		case 0:
			break ;
		case -1:
		{
			std::cout << "Error select" << std::endl;
			closeAllSocket();
			close(_serverFd);
			_serverFd = -1;
			break ;
		}
		default:
		{
			if (FD_ISSET(_serverFd, &rfds))
			{
				int newSocket = accept(_serverFd, (struct sockaddr*)&_address, (socklen_t*)&addrlen);
				if (newSocket < 0)
				{
    				std::cerr << "accept error" << std::endl;
					_serverFd = -1;
    			    break ;
    			}
				else if (newSocket >= 0)
				{
					for (int i = 1; i < 1024; i++)
					{
						if (_socket[i] < 0)
						{
							_socket[i] = newSocket;
							StaticFunctions::SendToFd(_socket[i], "Enter password with /PASS <pass>", "", 0);
							break ;
						}
					}
				}
			}
		}
	}
	return (rfds);
}

void Server::closeAllSocket()
{
	for (int i = 0; i < 1024; i++)
	{
		if (_socket[i] >= 0)
		{
			close(_socket[i]);
			_socket[i] = -1;
		}
	}
}

Channel * Server::getChannel(std::string name)
{
	std::list<Channel *>::iterator it = _channels.begin();
	std::list<Channel *>::iterator ite = _channels.end();
	while(it != ite)
	{
		if ((*it)->getName() == name)
			return (*it);
		++it;
	}
	return (NULL);
}

bool	Server::isUserCorrectlyConnected(int i)
{
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	if (usrIt == _users.end())
	{
		StaticFunctions::SendToFd(_socket[i], "You are not authenticated", "", 0);
		return	false;
	}
	if ((*usrIt)->getUsername().size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], "You didn't set your username", "", 0);
		return	false;
	}
	if ((*usrIt)->getNickname().size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], "You didn't set your nickname", "", 0);
		return	false;
	}
	return	true;
}

bool	Server::isUserAuthenticated(int i)
{
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	if (usrIt == _users.end())
	{
		StaticFunctions::SendToFd(_socket[i], "You are not authenticated", "", 0);
		return	false;
	}
	return	true;
}

void	Server::setNickname(int i, std::pair<Command, std::string>cmd)
{
	if (isUserAuthenticated(i) == false)
		return	;
	std::list<User *>::iterator it = StaticFunctions::findByFd(_users, _socket[i]);
	if (cmd.second.size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], "Your nickname can't be empty", "", 0);
		return;
	}
	if ((*it)->getNickname().empty())
	{
		(*it)->setNickname(cmd.second);
		StaticFunctions::SendToFd(_socket[i], "NICK ", (*it)->getNickname(), 0);
		return ;
	}
	StaticFunctions::SendToFd(_socket[i], (*it)->getNickname() + " nick change by ", cmd.second, 0);
	(*it)->setNickname(cmd.second);
}

void	Server::setUsername(int i, std::pair<Command, std::string> cmd)
{
	if (isUserAuthenticated(i) == false)
		return;
	std::list<User *>::iterator it = StaticFunctions::findByFd(_users, _socket[i]);
	if ((*it)->getUsername().size() != 0)
	{
		StaticFunctions::SendToFd(_socket[i], "Your username is already set", "", 0);
		return;
	}
	if ((*it)->getNickname().size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], "Your nickname is not set", "", 0);
		return;
	}
	if (cmd.second.size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], "Your username can't be empty", "", 0);
		return;
	}
	(*it)->setUsername(cmd.second);
	StaticFunctions::SendToFd(_socket[i], ":irc.example.com 001 " + (*it)->getNickname() + " :Welcome to the IRC Network " + cmd.second, "", 0);
}

void	Server::messageChannel(int i, std::pair<Command, std::string> cmd, User *op)
{
	std::vector<std::string> v = Parser::SplitCmd(cmd.second, " ");
	Channel * myChan = getChannel(v[0]);
	if (myChan == NULL)
		return ;
	std::list<User *> usr = myChan->getUsers();
	std::list<User *>::iterator it = usr.begin();
	for (; it != usr.end(); ++it)
	{
		if (_socket[i] != (*it)->getFd())
		{
			std::string message = ":" + op->getNickname() + " PRIVMSG " + cmd.second + "\r\n";
			send((*it)->getFd(), message.c_str(), message.size(), 0);
		}
	}
}

struct sockaddr_in Server::getAdresse()
{
	return (_address);
}

int	Server::getServerFd()
{
	return (_serverFd);
}

Server::Server()
{
	return ;
}

Server::Server(Server const & server)
{
	_channels = server._channels;
}

Server & Server::operator=(Server const & server)
{
	if (this != &server)
	{
		_channels = server._channels;
	}
	return (*this);
}

Server::~Server()
{
	return ;
}
