#include "Channel.hpp"
#include "Parser.hpp"
#include "User.hpp"

Channel::Channel(void)
{
	return;
}

Channel::Channel(std::size_t id, std::string name, User *user)
{
	this->_id = id;
	this->_name = name;
	this->_channelMod = "o";
	this->_usersLimit = 0;
	this->_users.push_back(user);
	user->connectChannel(this->_id);
	user->addFlag(_id, 'o');
	addUser(user);
}

Channel::~Channel()
{
}

Channel::Channel(Channel const & src)
{
	*this = src;
}

Channel & Channel::operator=(Channel const & src)
{
	if (this != &src)
	{
		this->_users = src._users;

		this->_password = src._password;
		this->_name = src._name;
		this->_topic = src._topic;
		this->_id = 0;
	}
	return	*this;
}

std::string Channel::getChannelMod() const
{
	return this->_channelMod;
}

std::list<User *> Channel::getUsers() const
{
	return this->_users;
}

std::string	Channel::getName() const
{
	return	this->_name;
}

std::string	Channel::getPassword() const
{
	return	this->_password;
}

std::size_t	Channel::getUserLimit() const
{
	return	this->_usersLimit;
}

std::string	Channel::getTopic() const
{
	return	this->_topic;
}

std::size_t	Channel::getId() const
{
	return	this->_id;
}

void		Channel::setName(std::string &name)
{
	this->_name = name;
}

void		Channel::setPassword(std::string &password)
{
	this->_password = password;
}

void		Channel::setUserLimit(std::size_t usr)
{
	this->_usersLimit = usr;
}

void		Channel::setTopic(std::string &topic)
{
	this->_topic = topic;
}

void		Channel::connectToChannel(User *user)
{
	user->connectChannel(this->_id);
	this->_users.push_back(user);
	std::list<User *>::iterator it = _users.begin();
	std::string usersNick;
	for (; it != _users.end(); it++)
	{
		StaticFunctions::SendToFd((*it)->getFd(),  ":" + user->getNickname() + " JOIN :" + this->getName(), "" , 0);
		if ((*it)->getFlags(_id).find('o') != std::string::npos)
			usersNick.append("@");
		usersNick.append((*it)->getNickname() + ' ');
	}
	if (!_topic.empty())
		StaticFunctions::SendToFd(user->getFd(), RPL_TOPIC(user->getNickname(), _name, _topic), "", 0);
	StaticFunctions::SendToFd(user->getFd(), RPL_NAMREPLY(_name, usersNick, user->getNickname()), "", 0);
	std::cout << RPL_NAMREPLY(_name, usersNick, user->getNickname()) << std::endl;
	StaticFunctions::SendToFd(user->getFd(), RPL_ENDOFNAMES(user->getNickname(), _name), "", 0);
}

void		Channel::addUser(User *user)
{
	if (this->_channelMod.find('i') == std::string::npos)
	{
		connectToChannel(user);
	}
	else
	{
		std::list<User *>::iterator it = StaticFunctions::findByFd(_invitedUsers, user->getFd()); 
		if (it == _invitedUsers.end())
			StaticFunctions::SendToFd(user->getFd(), ERR_INVITEONLYCHAN(this->getName()), "", 0);
		else
		{
			_invitedUsers.erase(it);
			connectToChannel(user);
		}
	}
}

bool	Channel::isUserOp(User *op)
{
	if (op->getFlags(this->_id).find('o') == std::string::npos)
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_CHANOPRIVSNEEDED(this->_name), "", 0);
		return	false;
	}
	return	true;
}

static bool	isValidFlag(const char c)
{
	const std::string validFlag = "pitlko";
	if (validFlag.find(c) != validFlag.npos)
		return true;
	return false;
}

void	Channel::updateFlag(std::vector<std::string> flags, User *op)
{
	if (flags.size() < 2)
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_NEEDMOREPARAMS(flags[0]), "", 0);
		return ;
	}
	if (isUserOp(op) == false)
		return ;
	if (!flags[1].empty() && flags[1].size() != 2)
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_NEEDMOREPARAMS(flags[1]), "", 0);
		return;
	}
	if (isValidFlag(flags[1][1]) == false)
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_UMODEUNKNOWNFLAG, "", 0);
		return ;
	}
	if (flags[1][0] == '+')
		addFlag(flags[1][1]);
	else if (flags[1][0] == '-')
		rmFlag(flags[1][1], op);

	// Check if channel is in o mode
	if (flags[1][0] == '+' && flags[1][1] == 'o' && !flags[2].empty())
		addOperator(op, flags[2]);
	else if (flags[1][0] == '-' && flags[1][1] == 'o' && !flags[2].empty())
		rmOperator(op, flags[2]);
	
	if (flags[1][0] == '+' && flags[1][1] == 'l' && !flags[2].empty()) // limit user
	{
		char *end = NULL;
		long i = std::strtol(flags[2].c_str(), &end, 10);
		if (end != NULL && i > 1000 && i < 0)
			StaticFunctions::SendToFd(op->getFd(), "+l need number", "", 0);
		else
		{
			setUserLimit(i);
			StaticFunctions::SendToFd(op->getFd(), ":" + op->getNickname() + " add limit " + flags[2] + " " + _name, "", 0);
		}
	}
	else if (flags[1][0] == '-' && flags[1][1] == 'l' && !flags[3].empty())
	{
		setUserLimit(0);
		StaticFunctions::SendToFd(op->getFd(), ":" + op->getNickname() + " delete limit " + _name, "", 0);
	}

	if (flags[1][0] == '+' && flags[1][1] == 'k' && !flags[3].empty()) // password channel
	{
		setPassword(flags[3]);
		StaticFunctions::SendToFd(op->getFd(), ":" + op->getNickname() + " add password " + flags[3] + " " + _name, "", 0);
	}
	else if (flags[1][0] == '-' && flags[1][1] == 'k' && !flags[3].empty())
	{
		std::string empty = "";
		setPassword(empty);
		StaticFunctions::SendToFd(op->getFd(), ":" + op->getNickname() + " delete password " + _name, "", 0);
	}
}

void	Channel::rmFlag(char flag, User *op)
{
	std::size_t idx = this->_channelMod.find(flag);
	if (idx != std::string::npos)
		this->_channelMod.erase(idx, 1);
	else
		StaticFunctions::SendToFd(op->getFd(), ERR_UMODEUNKNOWNFLAG, "", 0);
}

void	Channel::addFlag(char flag)
{
	if (this->_channelMod.find(flag) == std::string::npos)
		this->_channelMod += flag;
	// Je pense qu'il n'y a pas de message d'erreur
	/*else
		std::cerr << "Flag already set to this channel" << std::endl;*/
}

void	Channel::changeUserLimit(User *op, std::size_t limit)
{
	if (isUserOp(op) == false)
		return	;
	if (this->_channelMod.find('l') == std::string::npos)
	{
		this->_usersLimit = limit;	
	}
}

void	Channel::changeTopic(User *usr, std::string newTopic)
{
	if (this->_channelMod.find('t') != std::string::npos)
	{
		if (isUserOp(usr) == false)
			return	;
	}
	this->_topic = newTopic;
}

void		Channel::kickUser(User *op, std::string &name)
{
	std::list<User *>::iterator its = find(this->_users.begin(), this->_users.end(), name);
	if (its == _users.end())
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_USERNOTINCHANNEL(name, this->getName()), "", 0);
		return	;
	}
	if (isUserOp(op) == false)
			return	;
	std::list<User *>::iterator usrIt = _users.begin();
	for (; usrIt != _users.end(); usrIt++)
	{
		if (*usrIt != op)
			StaticFunctions::SendToFd((*usrIt)->getFd(),  ":" + op->getNickname() + " KICK " + getName() + " " + name, "", 0);
	}
	(*its)->disconnectChannel(this);
}

void		Channel::leaveUser(User *usr)
{
	std::list<User *>::iterator its = find(this->_users.begin(), this->_users.end(), usr);
	if (its == this->_users.end())
	{
		StaticFunctions::SendToFd(usr->getFd(), ERR_NOTONCHANNEL(this->getName()), "", 0);
		return	;
	}
	this->_users.erase(its);
}

void		Channel::addOperator(User *op, std::string &name)
{
	if (isUserOp(op) == false)
		return	;
	std::list<User *>::iterator its = find(this->_users.begin(), this->_users.end(), name);
	if (its == _users.end())
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_NOTONCHANNEL(this->getName()), "", 0);
		return	;
	}
	(*its)->addFlag(this->_id, 'o');
}

void		Channel::rmOperator(User *op, std::string &name)
{
	if (isUserOp(op) == false)
		return	;
	std::list<User *>::iterator its = find(this->_users.begin(), this->_users.end(), name);
	if (its == _users.end())
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_NOTONCHANNEL(this->getName()), "", 0);
		return	;
	}
	op->rmFlag(this->_id, 'o');
}

bool Channel::operator==(Channel const & src)
{
	return	this->_name == src._name;
}

bool operator==(Channel const * src, std::string const &name)
{
	return src->getName() == name;
}
