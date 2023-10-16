#include "Channel.hpp"
#include "Parser.hpp"
#include "User.hpp"
#include <cerrno>

Channel::Channel(void)
{
	return;
}

Channel::Channel(std::size_t id, std::string name, User *user)
{
	this->_id = id;
	this->_name = name;
	this->_channelMod = "";
	this->_usersLimit = 0;
	addUser(user, 1);	
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

void		Channel::connectToChannel(User *user, int opState)
{
	if (std::find(this->_users.begin(), this->_users.end(), user) != this->_users.end())
		return ;
	user->connectChannel(this->_id);
	if (opState == 1)
		user->addFlag(_id, 'o');
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

void		Channel::addUser(User *user, int opState)
{
	if (this->_channelMod.find('i') == std::string::npos)
	{
		connectToChannel(user, opState);
	}
	else
	{
		std::list<User *>::iterator it = StaticFunctions::findByFd(_invitedUsers, user->getFd()); 
		if (it == _invitedUsers.end())
			StaticFunctions::SendToFd(user->getFd(), ERR_INVITEONLYCHAN(user->getNickname(), _name), 0);
		else
		{
			_invitedUsers.erase(it);
			connectToChannel(user, opState);
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

bool	Channel::isInChannel(User *usr)
{
	std::list<User *>::iterator usrIt = find(_users.begin(), _users.end(), usr);
	if (usrIt == _users.end())
		return	false;
	return	true;
}

bool Channel::isFlagPresent(char flag)
{
	if (_channelMod.find(flag) == _channelMod.npos)
		return false;
	return true;
}

static bool	isValidFlag(const char c)
{
	const std::string validFlag = "itlko";
	if (validFlag.find(c) != validFlag.npos)
		return true;
	return false;
}

void	Channel::updateFlag(std::string cmd, User *op)
{
	std::vector<std::string> flags = Parser::SplitCmd(cmd, " ");
	if (flags.size() == 1)
	{
		StaticFunctions::SendToFd(op->getFd(), RPL_CHANNELMODEIS(op->getNickname(), _name, _channelMod), 0);
		return	;
	}
	if (isUserOp(op) == false)
		return ;
	if (!flags[1].empty() && flags[1].size() != 2)
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_NEEDMOREPARAMS(op->getNickname(), flags[1]), 0);
		return;
	}
	if (isValidFlag(flags[1][1]) == false)
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_UMODEUNKNOWNFLAG(op->getNickname()), 0);
		return ;
	}
	if (flags[1][1] == 'o')
	{
		if (flags.size() != 3)
		{
			StaticFunctions::SendToFd(op->getFd(), ERR_NEEDMOREPARAMS(op->getNickname(), static_cast<std::string>("MODE")), 0);
			return	;
		}
		if (flags[1][0] == '+')
			addOperator(op, flags[2]);
		else if (flags[1][0] == '-')
			rmOperator(op, flags[2]);
	}
	else if (flags[1][1] == 'l')
	{
		if (flags[1][0] == '-')
			setUserLimit(0);
		else if (flags.size() != 3)
		{
			StaticFunctions::SendToFd(op->getFd(), ERR_NEEDMOREPARAMS(op->getNickname(), static_cast<std::string>("MODE")), 0);
			return	;
		}
		if (flags[1][0] == '+')
		{
			char *end = NULL;
			long i = std::strtol(flags[2].c_str(), &end, 10);
			if (end[0] != 0 || i > 1000 || i < 0 || errno == ERANGE)
			{
				StaticFunctions::SendToFd(op->getFd(), ":127.0.0.1 502 " + op->getNickname() + " " + _name + " :+l need number", 0);
				return	;
			}
			else
				setUserLimit(i);
		}
	}
	else if (flags[1][1] == 'k')
	{
		if (flags[1][0] == '-')
		{
			std::string empty = "";
			setPassword(empty);
		}
		else if (flags.size() != 3)
		{
			StaticFunctions::SendToFd(op->getFd(), ERR_NEEDMOREPARAMS(op->getNickname(), static_cast<std::string>("MODE")), 0);
			return	;
		}
		if (flags[1][0] == '+')
			setPassword(flags[2]);
	}
	else
	{
		if (flags.size() > 2)
		{
			StaticFunctions::SendToFd(op->getFd(), ERR_NEEDMOREPARAMS(op->getNickname(), static_cast<std::string>("MODE")), 0);
			return	;
		}
		if (flags[1][0] == '+')
			addFlag(flags[1][1]);
		else
			rmFlag(flags[1][1], op);
	}
	sendToEveryuser(":" + op->getNickname() + " MODE " + cmd);
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
	if (cmd.empty() == true)
	{
		if (this->_topic.empty() == true)
			StaticFunctions::SendToFd(usr->getFd(), RPL_NOTOPIC(usr->getNickname(), _name), 0);
		else
			StaticFunctions::SendToFd(usr->getFd(), RPL_TOPIC(usr->getNickname(), _name, _topic), 0);
		return ;
	}
	std::list<User *>::iterator its = find(this->_users.begin(), this->_users.end(), usr->getNickname());
	if (its == _users.end())
	{
		StaticFunctions::SendToFd(usr->getFd(), ERR_USERNOTINCHANNEL(usr->getNickname(), this->getName()), 0);
		return	;
	}
	std::vector<std::string> newTopic = Parser::SplitCmd(cmd, " ");
	if (this->_channelMod.find('t') != std::string::npos)
	{
		if (isUserOp(usr) == false)
			return	;
	}
	sendToEveryuser(":" + usr->getNickname() + " TOPIC " + this->_name + " " + cmd);
	this->_topic = cmd;
}

void		Channel::kickOneUser(User *op, std::string name, std::string reason)
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

void		Channel::kickUser(User *op, std::vector<std::string> args)
{
	std::vector<std::string> nameSplit = Parser::SplitCmd(args[1], ",");
	for (std::size_t j = 0; j < nameSplit.size(); j++)
	{
		if (args.size() <= 2)
		{
			kickOneUser(op, nameSplit[j], op->getNickname());
		}
			//(*chan)->kickUser(op, nameSplit[j], (*usrIt)->getNickname());
		else
		{
			std::vector<std::string>::iterator begin = args.begin();
			std::advance(begin, 2);
			//(*chan)->kickUser(*usrIt, nameSplit[j], args[2]);
			kickOneUser(op, nameSplit[j], args[2]);
		}
	}
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

void		Channel::addOperator(User *op, std::string &name)
{
	if (isUserOp(op) == false)
		return	;
	if (name.size() == 0)
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_NEEDMOREPARAMS(op->getNickname(), static_cast<std::string>("MODE")), 0);
		return	;
	}
	std::list<User *>::iterator its = find(this->_users.begin(), this->_users.end(), name);
	if (its == _users.end())
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_NOTONCHANNEL(op->getNickname(), _name), 0);
		return	;
	}
	(*its)->addFlag(this->_id, 'o');
}

void		Channel::rmOperator(User *op, std::string &name)
{
	if (isUserOp(op) == false)
		return	;
	if (name.size() == 0)
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_NEEDMOREPARAMS(op->getNickname(), static_cast<std::string>("MODE")), 0);
		return	;
	}
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
	//std::cout << this->getName() << std::endl;
	if (std::find(_users.begin(), _users.end(), target) != _users.end())
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_USERONCHANNEL(target->getNickname(), _name), 0);
		return ;
	}
	if (std::find(_users.begin(), _users.end(), op) == _users.end())
	{
		StaticFunctions::SendToFd(op->getFd(), ERR_NOTONCHANNEL(op->getNickname(), _name), 0);
		return ;
	}
	if (this->isFlagPresent('i') == true)
	{
		if (this->isUserOp(op) == true)
		{
			StaticFunctions::SendToFd(op->getFd(), RPL_INVITING(op->getNickname(), target->getNickname(), _name), 0);
			if (std::find(_invitedUsers.begin(), _invitedUsers.end(), target) == _invitedUsers.end())
				_invitedUsers.push_back(target);
			return;
		}
		else	
		{
			StaticFunctions::SendToFd(op->getFd(), ERR_CHANOPRIVSNEEDED(op->getNickname(), _name), 0);
			return ;
		}
	}
	else
	{
		StaticFunctions::SendToFd(op->getFd(), RPL_INVITING(op->getNickname(), target->getNickname(), _name), 0);
		if (std::find(_invitedUsers.begin(), _invitedUsers.end(), target) == _invitedUsers.end())
			_invitedUsers.push_back(target);
		return;
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
