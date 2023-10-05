#include "Server.hpp"
#include "Channel.hpp"
#include "User.hpp"
#include "StaticFunctions.hpp"
#include "Define.hpp"

#include <sys/select.h>
#include <string>


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
						joinChannel(Parsedcmd->getArgs()[j], i);
						break;
					}

					case PASS:{	
						checkPass(Parsedcmd->getArgs()[j], i);
						break;
					}

					case NICK:{
						setNickname(Parsedcmd->getArgs()[j], i);
						break;
					}

					case USER:{
						setUsername(Parsedcmd->getArgs()[j], i);
						break;
					}

					case PART:{
						leaveChannel(Parsedcmd->getArgs()[j], i);
						break;
					}

					case TOPIC:{
						setTopic(Parsedcmd->getArgs()[j], i);
						break;
					}

					case QUIT:{
						quitServer(Parsedcmd->getArgs()[j], i);
						break;
					}

					case KICK:{
						kickUser(Parsedcmd->getArgs()[j], i);
						break;
					}
					
					case MODE: {
						changeModeChannel(Parsedcmd->getArgs()[j], i);
						break;
					}

					case PRIVMSG:
					{
						messageChannel(Parsedcmd->getArgs()[j], i ,Parsedcmd->getOperator());
						break;
					}

					default:
						break;
				}
			}
		}	
	}	
}

void Server::changeModeChannel(std::pair<Command, std::string>cmd, int i)
{
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	std::vector<std::string> cmdSplit = Parser::SplitCmd(cmd.second, " ");
	std::list<Channel *>::iterator it = find(_channels.begin(), _channels.end(), cmdSplit[0]);
	if (it == _channels.end())
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NOSUCHCHANNEL(cmdSplit[0]), "", 0);
		return ;
	}
	(*it)->updateFlag(cmdSplit, *usrIt);
}

void Server::kickUser(std::pair<Command, std::string>cmd, int i)
{
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	std::vector<std::string> v = Parser::SplitCmd(cmd.second, " ");
	std::list<Channel*>::iterator chan = find(_channels.begin(), _channels.end(), v[0]);
	if (chan == _channels.end())
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NOTONCHANNEL(v[0]), "", 0);
		return	;
	}
	if (v[1].size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NEEDMOREPARAMS(static_cast<std::string>("KICK")), "", 0);
		return	;
	}
	(*chan)->kickUser(*usrIt, v[1]);
	
}

void Server::quitServer(std::pair<Command, std::string>cmd, int i)
{
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	for(std::size_t nbChan = 0; nbChan < (*usrIt)->getNbChannel(); nbChan++)
	{
		std::size_t id = (*usrIt)->getChanId(nbChan);
		std::list<Channel *>::iterator chan = StaticFunctions::findChannelById(_channels, id);
		std::list<User *> userChan = (*chan)->getUsers();
		std::list<User *>::iterator itUserChan = userChan.begin();
		for (; itUserChan != userChan.end(); itUserChan++)
		{
			if (_socket[i] != (*itUserChan)->getFd())
			{
				std::string message = ":" + (*usrIt)->getNickname() + " PRIVMSG " + (*chan)->getName() + " is exiting the network with the message : \"" + cmd.second + "\"\r\n";
				send((*itUserChan)->getFd(), message.c_str(), message.size(), 0);
			}
		}
		(*chan)->leaveUser(*usrIt);
		if ((*chan)->getUsers().empty())
			_channels.erase(chan);
	}
	_users.erase(usrIt);
	close(_socket[i]);
	_socket[i] = -1;
}

void Server::checkPass(std::pair<Command, std::string>cmd, int i)
{	
	if (isUserAuthenticated(i) == true)
	{	
		StaticFunctions::SendToFd(_socket[i], ERR_ALREADYREGISTERED, "", 0);
		return	;
	}
	if (cmd.second == _password)
	{
		//StaticFunctions::SendToFd(_socket[i], "Password OK\r\n", "Now authenticate you with /NICK /USER", 0);
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
		StaticFunctions::SendToFd(_socket[i], ERR_PASSWDMISMATCH, "", 0);
}


void Server::joinChannel(std::pair<Command, std::string>cmd, int i)
{
	if (isUserCorrectlyConnected(i) == false)
		return ;
	std::vector<std::string> v = Parser::SplitCmd(cmd.second, " ");
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	std::list<Channel *>::iterator it = find(_channels.begin(), _channels.end(), v[0]);
	if (it != _channels.end())
	{
		if (!(*it)->getPassword().empty() && v[1].empty())
		{
			StaticFunctions::SendToFd(_socket[i], (*it)->getName() + " need password", "", 0);
			return;
		}
		if ((*it)->getPassword() != v[1])
		{
			StaticFunctions::SendToFd(_socket[i], "Incorrect password", "", 0);
			return;
		}
		if ((*it)->getUserLimit() == 0)
			(*it)->addUser(*usrIt);
		else if ((*it)->getUserLimit() != 0 && (*it)->getUsers().size() + 1 < (*it)->getUserLimit())
			(*it)->addUser(*usrIt);
		else
			StaticFunctions::SendToFd(_socket[i], ERR_CHANNELISFULL(cmd.second), "", 0);
	}
	else if (it == _channels.end())
	{
		std::string empty = "";
		Channel *c = new Channel(_channelNumber, v[0], *usrIt);
		_channelNumber++;
		if (!v[1].empty())
			c->setPassword(v[1]);
		else
			c->setPassword(empty);
		_channels.push_back(c);
		(*usrIt)->addFlag(c->getId(), 'o');
		it--;
	}
}

void Server::leaveChannel(std::pair<Command, std::string>cmd, int i)
{
	std::vector<std::string> v = Parser::SplitCmd(cmd.second, " ");
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	std::list<Channel *>::iterator it = find(_channels.begin(), _channels.end(), v[0]);
	if (it == _channels.end())
		StaticFunctions::SendToFd(_socket[i], ERR_NOSUCHCHANNEL(v[0]), "", 0);
	else
	{
		(*usrIt)->disconnectChannel((*it));
		StaticFunctions::SendToFd(_socket[i], ":" + (*usrIt)->getNickname() + " PART " + (*it)->getName(), "", 0);
	}
	if ((*it)->getUsers().empty())
		_channels.erase(it);
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
		StaticFunctions::SendToFd(_socket[i], ERR_NOTREGISTERED, "", 0);
		return	false;
	}
	if ((*usrIt)->getUsername().size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], "You didn't set your username", "", 0);
		return	false;
	}
	if ((*usrIt)->getNickname().size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NONICKNAMEGIVEN, "", 0);
		return	false;
	}
	return	true;
}

bool	Server::isUserAuthenticated(int i)
{
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	if (usrIt == _users.end())
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NOTREGISTERED, "", 0);
		return	false;
	}
	return	true;
}

void	Server::setNickname(std::pair<Command, std::string>cmd, int i)
{
	if (isUserAuthenticated(i) == false)
		return	;
	std::list<User *>::iterator it = StaticFunctions::findByFd(_users, _socket[i]);
	if (cmd.second.find_first_of(" \n\t\r\v\f#&:") != std::string::npos)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_ERRONEUSNICKNAME(cmd.second), "", 0);
		return	;
	}
	std::list<User *>::iterator usrIt = find(_users.begin(), _users.end(), cmd.second);
	if (usrIt != _users.end())
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NICKNAMEINUSE(cmd.second), "", 0);
		return	;
	}
	if (cmd.second.size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NONICKNAMEGIVEN, "", 0);
		return;
	}
	if ((*it)->getNickname().empty())
	{
		(*it)->setNickname(cmd.second);
		StaticFunctions::SendToFd(_socket[i], "NICK ", (*it)->getNickname() , 0);
		return ;
	}
	StaticFunctions::SendToFd(_socket[i], ":" + (*it)->getNickname() + " NICK ", cmd.second, 0);
	(*it)->setNickname(cmd.second);
}

void	Server::setUsername(std::pair<Command, std::string>cmd, int i)
{
	if (isUserAuthenticated(i) == false)
		return;
	std::list<User *>::iterator it = StaticFunctions::findByFd(_users, _socket[i]);
	if ((*it)->getUsername().size() != 0)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_ALREADYREGISTERED, "", 0);
		return;
	}
	if ((*it)->getNickname().size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NONICKNAMEGIVEN, "", 0);
		return;
	}
	if (cmd.second.size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NEEDMOREPARAMS(static_cast<std::string>("USER")), "", 0);
		return;
	}
	(*it)->setUsername(cmd.second);
	StaticFunctions::SendToFd(_socket[i], RPL_WELCOME((std::string)">ALL", (*it)->getNickname()), "", 0);
}

void	Server::messageChannel(std::pair<Command, std::string>cmd, int i, User *op)
{
	std::vector<std::string> v = Parser::SplitCmd(cmd.second, " ");
	if (v[0][0] != '#')
	{
		std::list<User *>::iterator it = find(_users.begin(), _users.end(), v[0]);
		if (it == _users.end())
		{
			StaticFunctions::SendToFd(_socket[i], ERR_NOSUCHNICK(v[0]), "", 0);
			return	;
		}
		std::string message = ":" + op->getNickname() + " PRIVMSG " + cmd.second + "\r\n";
		send((*it)->getFd(), message.c_str(), message.size(), 0);
		return	;
	}
	Channel * myChan = getChannel(v[0]);
	if (myChan == NULL)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NOSUCHCHANNEL(v[0]), "", 0);
		return ;
	}
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

void Server::setTopic(std::pair<Command, std::string>cmd, int i)
{
	std::vector<std::string> v = Parser::SplitCmd(cmd.second, " ");
	std::list<User *>::iterator currentUser = StaticFunctions::findByFd(_users, _socket[i]);
	Channel * myChan = getChannel(v[0]);
	if (myChan == NULL)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NOSUCHCHANNEL(v[0]), "", 0);
		return ;
	}
	std::list<User *> usr = myChan->getUsers();
	std::list<User *>::iterator it = usr.begin();
	if (myChan->getName() == cmd.second)
	{
		if (myChan->getTopic().empty())
		{
			StaticFunctions::SendToFd(_socket[i], RPL_NOTOPIC, "", 0);
			return ;
		}
		else
		{
			// localhost ou 127.0.0.1 ?
			std::string message = ":irc.example.com 332 " + (*it)->getNickname() + " " + cmd.second + " " + myChan->getTopic() + "\r\n";
			send(_socket[i], message.c_str(), message.size(), 0);
		}
		return ;
	}
	myChan->changeTopic(*currentUser, cmd.second);
	if (myChan->isUserOp(*currentUser) == false)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_CHANOPRIVSNEEDED(myChan->getName()), "", 0);
		return ;
	} 
	for (; it != usr.end(); ++it)
	{
		std::string message = ":" + (*currentUser)->getNickname() + " TOPIC " + cmd.second + "\r\n";
		send((*it)->getFd(), message.c_str(), message.size(), 0);
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
