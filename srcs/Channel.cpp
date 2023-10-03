#include "Channel.hpp"
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
	//user->addFlag(this->_id, 'o');
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

void		Channel::setTopic(std::string &topic)
{
	this->_topic = topic;
}

void		Channel::addUser(User *user)
{
	user->connectChannel(this->_id);
	this->_users.push_back(user);
}

bool	Channel::isUserOp(User *op)
{
	if (op->getFlags(this->_id).find('o') == std::string::npos)
	{
		std::cerr << "Insufficient permissions for this action" << std::endl;
		return	false;
	}
	return	true;
}

static bool	isValidFlag(const char c)
{
	const std::string validFlag = "pitlk";
	if (validFlag.find(c) != validFlag.npos)
		return true;
	return false;
}

void	Channel::updateFlag(const std::string flag, User *op)
{
	if (flag.size() < 2 || flag.size() > 2)
	{
		std::cerr << "Invalid flag: " << flag << std::endl;
		return ;
	}
	if (isValidFlag(flag[1]) == false)
	{
		std::cerr << "User flag error: Invalid flag: \'" << flag[1] << "\'" << std::endl;
		return ;
	}
	if (flag[0] == '+')
		addFlag(flag[1], op);
	else if (flag[0] == '-')
		rmFlag(flag[1], op);
	else
		std::cerr << "User flag error: Invalid flag sign: \'" << flag[0] << "\'" << std::endl;

}

void	Channel::rmFlag(char flag, User *op)
{
	if (isUserOp(op) == false)
		return	;
	std::size_t idx = this->_channelMod.find(flag);
	if (idx != std::string::npos)
		this->_channelMod.erase(idx);
	else
		std::cerr << "Permissions not set to this channel" << std::endl;
}

void	Channel::addFlag(char flag, User *op)
{
	if (isUserOp(op) == false)
		return	;
	if (this->_channelMod.find(flag) == std::string::npos)
		this->_channelMod += flag;
	else
		std::cerr << "Permissions already set to this channel" << std::endl;
}

void	Channel::changeUserLimit(User *op, std::size_t limit)
{
	if (isUserOp(op) == false)
		return	;
	if (this->_channelMod.find('l') == std::string::npos)
	{
		this->_usersLimit = limit;	
	}
	else
	{
		std::cerr << "User limit not set on this channel" << std::endl;
	}

}

void	Channel::changeTopic(User *usr, std::string newTopic)
{
	if (this->_channelMod.find('t') != std::string::npos)
	{
		if (usr->getFlags(this->_id).find('o') == std::string::npos)
		{
			std::cerr << "Insufficient permissions to manage channel" << std::endl;
			return	;
		}
	}
	this->_topic = newTopic;
}

void		Channel::kickUser(User *op, std::string &name)
{
	std::list<User *>::iterator its = find(this->_users.begin(), this->_users.end(), name);
	if (op->getFlags(this->_id).find('o') == std::string::npos)
	{
		std::cerr << "Insufficient permissions to kick" << std::endl;
		return	;
	}
	(*its)->disconnectChannel(this);
}

void		Channel::leaveUser(User *usr)
{
	std::list<User *>::iterator its = find(this->_users.begin(), this->_users.end(), usr);
	this->_users.erase(its);
}

void		Channel::addOperator(User *op, std::string &name)
{
	if (isUserOp(op) == false)
		return	;
	std::list<User *>::iterator its = find(this->_users.begin(), this->_users.end(), name);
	if (its == _users.end())
	{
		std::cerr << "No users found to op" << std::endl;
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
		std::cerr << "No users found to unop" << std::endl;
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
