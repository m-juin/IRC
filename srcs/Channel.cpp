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

void		Channel::sendToEveryuser(std::string toSend)
{
	std::list<User *>::iterator it = _users.begin();
	for (; it != _users.end(); it++)
	{
		StaticFunctions::SendToFd((*it)->getFd(), toSend , 0);
	}
}

void		Channel::sendToEveryuserNotHimself(std::string toSend, User *him)
{
	std::list<User *>::iterator it = _users.begin();
	for (; it != _users.end(); it++)
	{
		if (*it != him)
			StaticFunctions::SendToFd((*it)->getFd(), toSend , 0);
	}
}

void		Channel::connectToChannel(User *user)
{
	if (std::find(this->_users.begin(), this->_users.end(), user) != this->_users.end())
		return ;
	user->connectChannel(this->_id);
	this->_users.push_back(user);
	std::list<User *>::iterator it = _users.begin();
	std::string usersNick;
	for (; it != _users.end(); it++)
	{
		StaticFunctions::SendToFd((*it)->getFd(),  ":" + user->getNickname() + " JOIN :" + _name , 0);
		if ((*it)->getFlags(_id).find('o') != std::string::npos)
			usersNick.append("@");
		usersNick.append((*it)->getNickname() + ' ');
	}
	if (!_topic.empty())
		StaticFunctions::SendToFd(user->getFd(), RPL_TOPIC(user->getNickname(), _name, _topic), 0);
	StaticFunctions::SendToFd(user->getFd(), RPL_NAMREPLY(_name, usersNick, user->getNickname()), 0);
	StaticFunctions::SendToFd(user->getFd(), RPL_ENDOFNAMES(user->getNickname(), _name), 0);
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
			StaticFunctions::SendToFd(user->getFd(), ERR_INVITEONLYCHAN(user->getNickname(), _name), 0);
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
		StaticFunctions::SendToFd(op->getFd(), ERR_CHANOPRIVSNEEDED(op->getNickname(), _name), 0);
		return false;
	}
	return true;
}

bool Channel::isFlagPresent(char flag)
{
	if (_channelMod.find(flag) == _channelMod.npos)
		return false;
	return true;
}

static bool	isValidFlag(const char c)
{
	const std::string validFlag = "pit";
	if (validFlag.find(c) != validFlag.npos)
		return true;
	return false;
}

void Channel::updateUserLimit(std::string cmd, int sign)
{
	if (sign == 1)
	{
		char *end = NULL;
		long i = std::strtol(cmd.c_str(), &end, 10);
		if (end != NULL && i > 1000 && i < 0)
			return	;
			/*StaticFunctions::SendToFd(op->getFd(), ":127.0.0.1 502 " + op->getNickname() + " " + _name + " :+l need number", 0);*/
		else
		{
			setUserLimit(i);
			//sendToEveryuser(":" + op->getNickname() + " MODE " + cmd);
		}
	}
	else
	{
		setUserLimit(0);
		//sendToEveryuser(":" + op->getNickname() + " MODE " + cmd);
	}
}

void Channel::updatePassword(std::string cmd, int sign)
{
	if (sign == 1)
	{
		setPassword(cmd);
		//sendToEveryuser(":" + op->getNickname() + " MODE " + cmd);
	}
	else if (sign == 2)
	{
		std::string empty = "";
		setPassword(empty);
		//sendToEveryuser(":" + op->getNickname() + " MODE " + cmd);
	}
}

void	Channel::updateFlag(std::string cmd, User *op)
{
	std::vector<std::string> flags = Parser::SplitCmd(cmd, " ");
	if (flags.size() < 2)
	{
		return ;
	}
	if (isUserOp(op) == false)
		return ;
	std::size_t i = 1;
	std::size_t	param = 2;
	int	sign = 0;
	std::string toSend = flags[0] + " " + flags[1][0];
	if (flags[1][0] != '+' && flags[1][0] != '-')
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_UMODEUNKNOWNFLAG(op->getNickname()), 0);
		return	;
	}
	if (flags[1][0] == '+')
		sign = 1;
	else if (flags[1][0] == '-')
		sign = 2;
	for (;i < flags[1].size();i++)
	{
		if (param > flags.size())
			return	;
		if (flags[1][i] == 'o')
		{
			if (sign == 1)
				addOperator(op, flags[param]);
			else 
				rmOperator(op, flags[param]);
			param++;
		}
		else if (flags[1][i] == 'k')
		{
			updatePassword(flags[param], sign);
			param++;
			if (sign == 2)
				rmFlag(flags[1][i], op);
			else 
				addFlag(flags[1][i]);
		}
		else if (flags[1][i] == 'l')
		{
			updateUserLimit(flags[param], sign);
			param++;
			if (sign == 2)
				rmFlag(flags[1][i], op);
			else 
				addFlag(flags[1][i]);
		}
		else if (isValidFlag(flags[1][i]) == true)
		{
			if (sign == 2)
				rmFlag(flags[1][i], op);
			else 
				addFlag(flags[1][i]);
		}
		else
		{
			StaticFunctions::SendToFd(op->getFd(), ERR_UMODEUNKNOWNFLAG(op->getNickname()), 0);
			break	;
		}
		toSend += flags[1][i];
	}
	for (std::size_t i = 2; i < param; i++)
		toSend += " " + flags[i];
	std::cout << toSend << std::endl;
	sendToEveryuser(":" + op->getNickname() + " MODE " + toSend);
}

void	Channel::rmFlag(char flag, User *op)
{
	std::size_t idx = this->_channelMod.find(flag);
	if (idx != std::string::npos)
		this->_channelMod.erase(idx, 1);
	else
		StaticFunctions::SendToFd(op->getFd(), ERR_UMODEUNKNOWNFLAG(op->getNickname()), 0);
}

void	Channel::addFlag(char flag)
{
	if (this->_channelMod.find(flag) == std::string::npos)
		this->_channelMod += flag;
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

void	Channel::changeTopic(User *usr, std::string cmd)
{	
	std::vector<std::string> newTopic = Parser::SplitCmd(cmd, " ");
	if (newTopic.size() <= 1)
	{
		StaticFunctions::SendToFd(usr->getFd(), RPL_NOTOPIC(usr->getNickname(), _name), 0);
		return ;
	}
	if (this->_channelMod.find('t') != std::string::npos)
	{
		if (isUserOp(usr) == false)
			return	;
	}
	sendToEveryuser(":" + usr->getNickname() + " TOPIC " + newTopic[0] + " " + newTopic[1]);
	this->_topic = &newTopic[1][1];
}

void		Channel::kickUser(User *op, std::string &name, std::string reason)
{
	std::list<User *>::iterator its = find(this->_users.begin(), this->_users.end(), name);
	if (its == _users.end())
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_USERNOTINCHANNEL(name, this->getName()), 0);
		return	;
	}
	if (isUserOp(op) == false)
			return	;
	sendToEveryuser(":" + op->getNickname() + " KICK " + getName() + " " + name + " :" + reason);
	(*its)->disconnectChannel(this);
}

void		Channel::leaveUser(User *usr)
{
	std::list<User *>::iterator its = find(this->_users.begin(), this->_users.end(), usr);
	if (its == this->_users.end())
	{
		StaticFunctions::SendToFd(usr->getFd(), ERR_NOTONCHANNEL(usr->getNickname(), _name), 0);
		return	;
	}
	this->_users.erase(its);
}

void		Channel::addOperator(User *op, std::string name)
{
	if (isUserOp(op) == false)
		return	;
	std::list<User *>::iterator its = find(this->_users.begin(), this->_users.end(), name);
	if (its == _users.end())
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_NOTONCHANNEL(op->getNickname(), _name), 0);
		return	;
	}
	(*its)->addFlag(this->_id, 'o');
}

void		Channel::rmOperator(User *op, std::string name)
{
	if (isUserOp(op) == false)
		return	;
	std::list<User *>::iterator its = find(this->_users.begin(), this->_users.end(), name);
	if (its == _users.end())
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_NOTONCHANNEL(op->getNickname(), _name), 0);
		return	;
	}
	(*its)->rmFlag(this->_id, 'o');
}

void Channel::inviteUser(User *op, User *target)
{
	if (this->isFlagPresent('i') == true)
	{
		if (this->isUserOp(op) == true)
		{
			StaticFunctions::SendToFd(target->getFd(), RPL_INVITING(target->getNickname(), _name), 0);
			_invitedUsers.push_back(target);
			return;
		}
		else
		{
			StaticFunctions::SendToFd(op->getFd(), ERR_CHANOPRIVSNEEDED(op->getNickname(), _name), 0);
		}
	}
}

bool Channel::operator==(Channel const & src)
{
	return	this->_name == src._name;
}

bool operator==(Channel const * src, std::string const &name)
{
	return src->getName() == name;
}
