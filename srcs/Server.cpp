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
	for (int i = 0 ;i < 1024;i++) {
        _socket.push_back(-1);
    }
	_socket[0] = _serverFd;
	while (true)
	{
		fd_set rfds = addNewSocket();
		if (_serverFd == -1)
			return ;
		for (int i = 1; i < 1024; i++)
		{
			int valread = 0;
			Parser *Parsedcmd;
			if (_socket[i] > 0 && FD_ISSET(_socket[i], &rfds))
			{
				valread = recv(_socket[i], buff, 1024, 0);
				//std::memset(buff, 0, 1024);
				// A delete test Parser:
				Parsedcmd = new Parser(_users, _socket[i], buff);
				std::vector<std::string>::iterator osef = Parsedcmd->args.begin();
				for (; osef != Parsedcmd->args.end(); osef++)
					std::cout << *osef << std::endl;
			}
			if (valread != 0 && Parsedcmd->cmd == 3)
			{
				//Check if pass is entered, username and nickname are set
				if (isUserCorrectlyConnected(i) == false)
				{
					StaticFunctions::SendToFd(_socket[i], "You don't have permission to execute commands", "", 0);
					continue;
				}
				joinChannel(buff, i);
				std::memset(buff, 0, 1024);
			}
			else if (valread != 0 && Parsedcmd->cmd == 0)
			{
				checkPass(buff, i);
				std::memset(buff, 0, 1024);
			}
			else if (valread != 0 && Parsedcmd->cmd == 2)
			{
				setNickname(i, Parsedcmd);
				std::memset(buff, 0, 1024);
			}
			else if (valread != 0 && Parsedcmd->cmd == 1)
			{
				setUsername(i, Parsedcmd);
				std::memset(buff, 0, 1024);

			}
			else if (valread != 0 && strcmp(buff, "END\r\n") == 0)
			{
				closeAllSocket();
				close(_serverFd);
				return ;
			}
			else if (valread != 0)
			{
				if (buff[8] == '#')
				{
					int k = 0;
					while (buff[k] != '#')
						k++;
					std::string value = &buff[k];
					k = 0;
					int j = 0;
					while (value[k] != ' ')
					{
						k++;
						if (value[k] == ' ')
						{
							while (value[k + j])
								j++;
							break ;	
						}
					}
					value = value.substr(0, value.size() - j);
					Channel * myChan = getChannel(value);
					if (myChan == NULL)
						return ;
					std::list<User *> usr = myChan->getUsers();
					std::list<User *>::iterator it = usr.begin();
					std::string buffer;
					for (; it != usr.end(); ++it)
					{
						if (_socket[i] != (*it)->getFd())
						{
							std::string test = &buff[8];
							std::string mess = test.substr(0, test.size() - 2);
							std::string message = ":" + (*it)->getNickname() + " PRIVMSG " + mess + "\r\n";
							send((*it)->getFd(), message.c_str(), message.size(), 0);
						}
					}
					
				}
				std::memset(buff, 0, 1024);
			}
			std::memset(buff, 0, 1024);
		}
	}
}

void Server::checkPass(char *buff, int i)
{
	// check si le user a deja tape le mot de passe
	if (isUserAuthenticated(i) == true)
	{
		StaticFunctions::SendToFd(_socket[i], "You're already authenticated", "", 0);
		return	;
	}
	if (strncmp(&buff[5], _password.c_str(), _password.size()) == 0)
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


void Server::joinChannel(char *buff, int i)
{
	std::string test = &buff[5];
	std::string nameChannel = test.substr(0, test.size() - 2);
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	std::list<Channel *>::iterator it = _channels.begin();
	for (; it != _channels.end(); ++it)
	{
		if (*it == nameChannel)
		{
			StaticFunctions::SendToFd(_socket[i], "You joined channel ", nameChannel, 0);
			(*it)->addUser(*usrIt);
			break;
		}
	}
	if (it == _channels.end())
	{
		Channel *c = new Channel(_channelNumber,  nameChannel, *usrIt);
		_channelNumber++;
		_channels.push_back(c);
		it--;
	}
	std::string message = ":" + (*usrIt)->getNickname() + " JOIN " + (*it)->getName() + "\r\n";
	send(_socket[i], message.c_str(), message.size(), 0);
}

fd_set Server::addNewSocket()
{
	fd_set rfds;
	int addrlen = sizeof(_address);

	FD_ZERO(&rfds);
	for (int i = 0; i < 1024; i++)
	{
		if (_socket[i] >= 0)
			FD_SET(_socket[i], &rfds);
	}
	switch (select(FD_SETSIZE,  &rfds, NULL, NULL, NULL))
	{
		case 0:
			break;
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

void	Server::setNickname(int i, Parser *cmd)
{
	// check si le user a tape le mot de passe
	if (isUserAuthenticated(i) == false)
		return	;
	std::list<User *>::iterator it = StaticFunctions::findByFd(_users, _socket[i]);
	//std::cout << (*it)->getId() << std::endl;
	std::vector<std::string>::iterator osef = cmd->args.begin();
	osef++;
	//Check if nickname is empty
	if ((*osef).size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], "Your nickname can't be empty", "", 0);
		return;
	}
	if ((*it)->getNickname().empty())
	{
		(*it)->setNickname(*osef);
		StaticFunctions::SendToFd(_socket[i], "NICK ", (*it)->getNickname(), 0);
		return ;
	}
	StaticFunctions::SendToFd(_socket[i], (*it)->getNickname() + " nick change by ", *osef, 0);
	(*it)->setNickname(*osef);
}

void	Server::setUsername(int i, Parser *cmd)
{
	// check si le user a tape le mot de passe
	// on ne peut pas changer son user name
	if (isUserAuthenticated(i) == false)
		return;
	std::list<User *>::iterator it = StaticFunctions::findByFd(_users, _socket[i]);
	//Check if username is already set, shouldn't do anything is this case
	if ((*it)->getUsername().size() != 0)
	{
		StaticFunctions::SendToFd(_socket[i], "Your username is already set", "", 0);
		return;
	}
	//Check if nickname is set or not
	if ((*it)->getNickname().size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], "Your nickname is not set", "", 0);
		return;
	}
	std::vector<std::string>::iterator osef = cmd->args.begin();
	osef++;
	if (!(*it)->getUsername().empty())
	{
		StaticFunctions::SendToFd(_socket[i], "You can't change your username", "", 0);
		return;
	}
	//Check if username is empty
	if ((*osef).size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], "Your username can't be empty", "", 0);
		return;
	}
	(*it)->setUsername(*osef);
	std::cout << "New client: " << (*it)->getFd() << " " << (*it)->getUsername() << " " << (*it)->getNickname() << std::endl;
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
