#include "User.hpp"

/*----------------------------------------------------------------------------------------------*/
/*																								*/
/*										Constructor	& Destructor								*/
/*																								*/
/*----------------------------------------------------------------------------------------------*/

User::User(std::size_t id, std::string username, std::string nickname, int fd)
{
	_id = id;
	_username = username;
	_fd = fd;
	_nickname = nickname;
}

User::User()
{
	return ;
}

User::User(User const & user)
{
	_id = user._id;
	_username = user._username;
	_nickname = user._nickname;
	_fd = user._fd;
}

User::~User()
{
	close(_fd);
	_fd = -1;
	return ;
}


/*----------------------------------------------------------------------------------------------*/
/*																								*/
/*											Getter & Setter										*/
/*																								*/
/*----------------------------------------------------------------------------------------------*/

std::size_t  User::getId() const
{
	return (_id);
}

std::string User::getUsername() const
{
	return (_username);
}

std::string	User::getNickname() const
{
	return (_nickname);
}

int	User::getFd() const
{
	return (_fd);
}

void	User::setId(std::size_t value)
{
	_id = value;
}

void	User::setUsername(std::string value)
{
	_username = value;
}

void 	User::setNickname(std::string value)
{
	_nickname = value;
}

void	User::setFd(int value)
{
	_fd = value;
}

/*----------------------------------------------------------------------------------------------*/
/*																								*/
/*												Flags											*/
/*																								*/
/*----------------------------------------------------------------------------------------------*/

flagsPair::iterator User::getFlagsIndex(std::size_t channelID)
{
	flagsPair::iterator it = this->_channelsFlags.begin();
	flagsPair::iterator ite = this->_channelsFlags.end();
	for (; it != ite; it++)
	{
		if (it->first == channelID)
			return it;
	}
	return ite;
}

bool User::isConnected(std::size_t channelID)
{
	flagsPair::iterator it;
	flagsPair::iterator ite = this->_channelsFlags.end();
	for (it = this->_channelsFlags.begin(); it != ite; it++)
	{
		if (it->first == channelID)
			return true;
	}
	return false;
}

static bool	isValidFlag(const char c)
{
	const std::string validFlag = "o";
	if (validFlag.find(c) != validFlag.npos)
		return true;
	return false;
}

void	User::updateFlag(std::size_t channelID, const std::string flag)
{
	if (flag.size() < 2 || flag.size() > 2)
	{
		StaticFunctions::SendToFd(_fd, ERR_NEEDMOREPARAMS(_nickname, flag), 0);
		return ;
	}
	if (isValidFlag(flag[1]) == false)
	{
		StaticFunctions::SendToFd(_fd, ERR_UMODEUNKNOWNFLAG(_nickname), 0);
		return ;
	}
	if (flag[0] == '+')
		addFlag(channelID, flag[1]);
	else if (flag[0] == '-')
		rmFlag(channelID, flag[1]);
	else
		StaticFunctions::SendToFd(_fd, ERR_UMODEUNKNOWNFLAG(_nickname), 0);

}

bool User::addFlag(std::size_t channelID, char flag)
{
	flagsPair::iterator it = getFlagsIndex(channelID);
	if (it == _channelsFlags.end())
	{
		return true; 
	}
	std::size_t search = it->second.find(flag, 0);
	if (search != it->second.npos)
	{
		return false; 
	}
	it->second += flag;
	return true;
}

bool User::rmFlag(std::size_t channelID, char flag)
{
	flagsPair::iterator it = getFlagsIndex(channelID);
	if (it == _channelsFlags.end())
	{
		return false; 
	}
	std::size_t search = it->second.find(flag, 0);
	if (search == it->second.npos)
	{
		return false; 
	}
	it->second.erase(search, 1);
	return true;
}

std::string User::getFlags(std::size_t channelId)
{
	return getFlagsIndex(channelId)->second;
}

std::size_t User::getChanId()
{
	return	_channelsFlags.begin()->first;
}

std::size_t User::getNbChannel() const
{
	return	_channelsFlags.size();
}

/*----------------------------------------------------------------------------------------------*/
/*																								*/
/*										Connection functions									*/
/*																								*/
/*----------------------------------------------------------------------------------------------*/

void User::connectChannel(std::size_t channelID)
{
	std::pair<std::size_t, std::string> newFlags = std::make_pair(channelID, "");
	this->_channelsFlags.push_back(newFlags);
}

bool User::disconnectChannel(Channel *chn)
{
	flagsPair::iterator it = getFlagsIndex(chn->getId());
	if ( it == this->_channelsFlags.end() )
	{
		StaticFunctions::SendToFd(_fd, ERR_NOTONCHANNEL(_nickname, chn->getName()), 0);
		return false;
	}
	chn->leaveUser(this);
	this->_channelsFlags.erase(it);
	return true;
}

/*----------------------------------------------------------------------------------------------*/
/*																								*/
/*											Operator											*/
/*																								*/
/*----------------------------------------------------------------------------------------------*/

bool User::operator==(User const &src)
{
	return this->_username == src._username;
}

bool operator==(User const * src, std::string const &name)
{
	return src->getNickname() == name;
}

User & User::operator=(User const & user)
{
	if (this != &user)
	{
		_id = user._id;
		_username = user._username;
		_nickname = user._nickname;
		_fd = user._fd;
	}
	return (*this);
}
