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
				if (valread == -1)
				{
					// Maybe we need to shut the server down, 
					// just delete this condition if you think we don't care
				}
				if (valread == 0)
				{
					connexionLost(i);
					continue ;
				}
				_buffers[_socket[i]] += buff;
				if ((_buffers[_socket[i]][_buffers[_socket[i]].size() - 1] == '\r') || _buffers[_socket[i]][_buffers[_socket[i]].size() - 1] == '\n')
				{
					Parsedcmd = new Parser(_users, _socket[i], _buffers[_socket[i]]);
					_buffers[_socket[i]] = "";
					std::memset(buff, 0, 1024);
				}
				else
				{
					std::memset(buff, 0, 1024);
					continue;
				}
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

					case INVITE:
					{
						inviteUser(Parsedcmd->getArgs()[j], i ,Parsedcmd->getOperator());
						break;
					}

					default:
						break;
				}
			}
			delete Parsedcmd;
		}	
	}	
}

void Server::closeConnexionUser(int i)
{
	_buffers[_socket[i]].clear();
	close(_socket[i]);
	_socket[i] = -1;
}

void Server::connexionLost(int i)
{
	if (isUserAuthenticated(i, false) == false)
	{	
		closeConnexionUser(i);
		return	;
	}
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	for(std::size_t nbChan = 0; nbChan < (*usrIt)->getNbChannel(); nbChan++)
	{
		std::size_t id = (*usrIt)->getChanId(nbChan);
		std::list<Channel *>::iterator chan = StaticFunctions::findChannelById(_channels, id);
		if (chan == _channels.end())
		{
			closeConnexionUser(i);
			return	;
		}
		(*chan)->leaveUser(*usrIt);
		(*chan)->sendToEveryuser(":" + (*usrIt)->getNickname() + " QUIT :Connexion Lost");
		if ((*chan)->getUsers().empty())
			_channels.erase(chan);
	}
	_users.erase(usrIt);
	closeConnexionUser(i);
}

void Server::changeModeChannel(std::pair<Command, std::string>cmd, int i)
{
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	std::vector<std::string> cmdSplit = Parser::SplitCmd(cmd.second, " ");
	std::list<Channel *>::iterator it = find(_channels.begin(), _channels.end(), cmdSplit[0]);
	if (it == _channels.end())
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NOSUCHCHANNEL((*usrIt)->getNickname(), cmdSplit[0]), 0);
		return ;
	}
	(*it)->updateFlag(cmd.second, *usrIt);
}



void Server::kickUser(std::pair<Command, std::string>cmd, int i)
{
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	std::vector<std::string> cmdSplit = Parser::SplitCmdNotLastParam(cmd.second, " ");
	std::list<Channel*>::iterator chan = find(_channels.begin(), _channels.end(), cmdSplit[0]);
	if (chan == _channels.end())
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NOTONCHANNEL((*usrIt)->getNickname(), cmdSplit[0]), 0);
		return	;
	}
	if (cmdSplit[1].size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NEEDMOREPARAMS(static_cast<std::string>("KICK")), 0);
		return ;	
	}
	std::vector<std::string> nameSplit = Parser::SplitCmd(cmdSplit[1], ",");
	for (std::size_t j = 0; j < nameSplit.size(); j++)
	{
		if (cmdSplit.size() <= 2)
			(*chan)->kickUser(*usrIt, nameSplit[j], (*usrIt)->getNickname());
		else
		{
			std::vector<std::string>::iterator begin = cmdSplit.begin();
			std::advance(begin, 2);
			(*chan)->kickUser(*usrIt, nameSplit[j], cmdSplit[2]);
		}
	}
	if ((*chan)->getUsers().empty())
		_channels.erase(chan);
	
}

void Server::quitServer(std::pair<Command, std::string>cmd, int i)
{
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	for(std::size_t nbChan = 0; nbChan < (*usrIt)->getNbChannel(); nbChan++)
	{
		std::size_t id = (*usrIt)->getChanId(nbChan);
		std::list<Channel *>::iterator chan = StaticFunctions::findChannelById(_channels, id);
		if (chan == _channels.end())
		{
			closeConnexionUser(i);
			return	;
		}
		(*chan)->leaveUser(*usrIt);
		(*chan)->sendToEveryuser(":" + (*usrIt)->getNickname() + " QUIT :" + cmd.second);
		if ((*chan)->getUsers().empty())
			_channels.erase(chan);
	}
	_users.erase(usrIt);
	closeConnexionUser(i);
}

void Server::checkPass(std::pair<Command, std::string>cmd, int i)
{	
	if (isUserAuthenticated(i, false) == true)
	{
		std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
		if (usrIt != _users.end())
			StaticFunctions::SendToFd(_socket[i], ERR_ALREADYREGISTERED((*usrIt)->getNickname()), 0);
		else
			StaticFunctions::SendToFd(_socket[i], ERR_ALREADYREGISTERED(static_cast<std::string>("*")), 0);
		return	;
	}
	if (cmd.second == _password)
	{
		std::list<User *>::iterator it = _users.end();
		int id;
		if (_users.end() != _users.begin())
		{
			it--;
			id = (*it)->getId();
		}
		else
			id = 0;
		_users.push_back(new User(id, "", "*",_socket[i]));
	}
	else
		StaticFunctions::SendToFd(_socket[i], ERR_PASSWDMISMATCH, 0);
}


void Server::joinChannel(std::pair<Command, std::string>cmd, int i)
{
	if (isUserCorrectlyConnected(i, true) == false)
		return ;
	std::vector<std::string> cmdSplit = Parser::SplitCmd(cmd.second, " ");
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	if (cmdSplit[0].find(':') != std::string::npos)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NOSUCHCHANNEL((*usrIt)->getNickname(), cmdSplit[0]), 0);
		return	;
	}
	std::list<Channel *>::iterator it = find(_channels.begin(), _channels.end(), cmdSplit[0]);
	if (it != _channels.end())
	{
		if (!(*it)->getPassword().empty() && (cmdSplit.size() <= 1 || (*it)->getPassword() != cmdSplit[1]))
		{
			StaticFunctions::SendToFd(_socket[i], ERR_BADCHANNELKEY((*usrIt)->getNickname(),(*it)->getName()), 0);
			return;
		}
		if ((*it)->getUserLimit() == 0)
			(*it)->addUser(*usrIt);
		else if ((*it)->getUserLimit() != 0 && (*it)->getUsers().size() + 1 < (*it)->getUserLimit())
			(*it)->addUser(*usrIt);
		else
			StaticFunctions::SendToFd(_socket[i], ERR_CHANNELISFULL((*usrIt)->getNickname(), cmd.second), 0);
	}
	else if (it == _channels.end())
	{
		std::string empty = "";
		Channel *c = new Channel(_channelNumber, cmdSplit[0], *usrIt);
		_channelNumber++;
		if (cmdSplit.size() > 1 && !cmdSplit[1].empty())
			c->setPassword(cmdSplit[1]);
		else
			c->setPassword(empty);
		_channels.push_back(c);
	}
}

void Server::leaveChannel(std::pair<Command, std::string>cmd, int i)
{
	std::vector<std::string> cmdSplit = Parser::SplitCmd(cmd.second, " ");
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	std::list<Channel *>::iterator it = find(_channels.begin(), _channels.end(), cmdSplit[0]);
	if (it == _channels.end())
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NOSUCHCHANNEL((*usrIt)->getNickname(), cmdSplit[0]), 0);
		return	;
	}
	else
	{
		std::string reason;
		if (cmdSplit.size() <= 1)
			reason = "Leaving";
		else
			reason = cmdSplit[1];
		(*it)->sendToEveryuser(":" + (*usrIt)->getNickname() + " PART " + (*it)->getName() + " " + reason);
		(*usrIt)->disconnectChannel((*it));
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
					for (int i = 1; i < 1000; i++)
					{
						if (_socket[i] < 0)
						{
							_socket[i] = newSocket;
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
	for (int i = 0; i < 1000; i++)
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

bool	Server::isUserCorrectlyConnected(int i, bool sendMessage)
{
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	if (usrIt == _users.end() || (*usrIt)->getUsername().size() == 0 || (*usrIt)->getNickname() == "*")
	{
		if(sendMessage == true)
			StaticFunctions::SendToFd(_socket[i], ERR_NOTREGISTERED((*usrIt)->getNickname()), 0);
		return	false;
	}
	return	true;
}

bool	Server::isUserAuthenticated(int i, bool printState)
{
	std::list<User *>::iterator usrIt = StaticFunctions::findByFd(_users, _socket[i]);
	if (usrIt == _users.end())
	{
		if (printState == true)
			StaticFunctions::SendToFd(_socket[i], ERR_NOTREGISTERED((*usrIt)->getNickname()), 0);
		return	false;
	}
	return	true;
}

void	Server::setNickname(std::pair<Command, std::string>cmd, int i)
{
	if (isUserAuthenticated(i, true) == false)
		return	;
	std::list<User *>::iterator it = StaticFunctions::findByFd(_users, _socket[i]);
	if (cmd.second.size() <= 1 || cmd.second.find_first_of(" \n\t\r\v\f#&:*") != std::string::npos || std::isdigit(cmd.second[0]))
	{
		StaticFunctions::SendToFd(_socket[i], ERR_ERRONEUSNICKNAME((*it)->getNickname(), cmd.second), 0);
		return	;
	}
	std::list<User *>::iterator usrIt = find(_users.begin(), _users.end(), cmd.second);
	if (usrIt != _users.end())
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NICKNAMEINUSE((*usrIt)->getNickname(), cmd.second), 0);
		return	;
	}
	if (cmd.second.size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NONICKNAMEGIVEN((*usrIt)->getNickname()), 0);
		return;
	}
	if ((*it)->getNickname() == "*")
	{
		(*it)->setNickname(cmd.second);
		StaticFunctions::SendToFd(_socket[i], "NICK " + (*it)->getNickname(), 0);
		if(isUserCorrectlyConnected(i, false) == true)
			StaticFunctions::SendToFd(_socket[i], RPL_WELCOME((std::string)">ALL", (*it)->getNickname()), 0);
		return ;
	}
	StaticFunctions::SendToFd(_socket[i], ":" + (*it)->getNickname() + " NICK " + cmd.second, 0);
	(*it)->setNickname(cmd.second);
	if(isUserCorrectlyConnected(i, false) == true)
		StaticFunctions::SendToFd(_socket[i], RPL_WELCOME((std::string)">ALL", (*it)->getNickname()), 0);
}

void	Server::setUsername(std::pair<Command, std::string>cmd, int i)
{
	if (isUserAuthenticated(i, true) == false)
		return;
	std::list<User *>::iterator it = StaticFunctions::findByFd(_users, _socket[i]);
	if ((*it)->getUsername().size() != 0)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_ALREADYREGISTERED((*it)->getNickname()), 0);
		return;
	}
	if (cmd.second.size() == 0)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NEEDMOREPARAMS(static_cast<std::string>("USER")), 0);
		return;
	}
	(*it)->setUsername(cmd.second);
	if(isUserCorrectlyConnected(i, false) == true)
		StaticFunctions::SendToFd(_socket[i], RPL_WELCOME((std::string)">ALL", (*it)->getNickname()), 0);
}

void	Server::messageChannel(std::pair<Command, std::string>cmd, int i, User *op)
{
	std::vector<std::string> cmdSplit = Parser::SplitCmd(cmd.second, " ");
	if (cmdSplit[0][0] != '#')
	{
		std::list<User *>::iterator it = find(_users.begin(), _users.end(), cmdSplit[0]);
		if (it == _users.end())
		{
			StaticFunctions::SendToFd(_socket[i], ERR_NOSUCHNICK(cmdSplit[0]), 0);
			return	;
		}
		StaticFunctions::SendToFd((*it)->getFd(), ":" + op->getNickname() + " PRIVMSG " + cmd.second, 0);
		return	;
	}
	Channel * myChan = getChannel(cmdSplit[0]);
	if (myChan == NULL)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NOSUCHCHANNEL(op->getNickname(), cmdSplit[0]), 0);
		return ;
	}
	myChan->sendToEveryuserNotHimself(":" + op->getNickname() + " PRIVMSG " + cmd.second, op);
}

void Server::inviteUser(std::pair<Command, std::string> cmd, int i, User *op)
{
	std::vector<std::string> cmdSplit = Parser::SplitCmd(cmd.second, " ");
	if (cmdSplit.size() < 2)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NEEDMOREPARAMS(static_cast<std::string>("INVITE")), 0);
		return ;
	}
	std::list<User *>::iterator it = find(_users.begin(), _users.end(), cmdSplit[1]);
	if (it == _users.end())
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NOSUCHNICK(cmdSplit[1]), 0);
		return	;
	}
	Channel * myChan = getChannel(cmdSplit[0]);
	if (myChan == NULL)
	{
		StaticFunctions::SendToFd(_socket[i], RPL_INVITING(op->getNickname(), cmdSplit[1], cmdSplit[0]), 0);
		return ;
	}
	myChan->inviteUser(op, *it);
}

void Server::setTopic(std::pair<Command, std::string>cmd, int i)
{
	std::vector<std::string> cmdSplit = Parser::SplitCmd(cmd.second, " ");
	std::list<User *>::iterator currentUser = StaticFunctions::findByFd(_users, _socket[i]);
	Channel * myChan = getChannel(cmdSplit[0]);
	if (myChan == NULL)
	{
		StaticFunctions::SendToFd(_socket[i], ERR_NOSUCHCHANNEL((*currentUser)->getNickname(), cmdSplit[0]), 0);
		return ;
	}
	myChan->changeTopic(*currentUser, cmd.second);
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
