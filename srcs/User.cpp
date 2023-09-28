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

std::size_t  User::getIdUser() const
{
	return (_id);
}

std::string User::getUsernameUser() const
{
	return (_username);
}

std::string	User::getUserNickname() const
{
	return (_nickname);
}

int	User::getUserFd() const
{
	return (_fd);
}

void	User::setIdUser(std::size_t value)
{
	_id = value;
}

void	User::setUsernameUser(std::string value)
{
	_username = value;
}

void 	User::setUserNickname(std::string value)
{
	_nickname = value;
}

void	User::setUserFd(int value)
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
	flagsPair::iterator it;
	flagsPair::iterator ite = this->_channelsFlags.end();
	for (it = this->_channelsFlags.begin(); it != ite; it++)
	{
		if (it->first == channelID)
			return it;
	}
	return ite;
}

bool User::addFlag(std::size_t channelID, char flag)
{
	flagsPair::iterator it = getFlagsIndex(channelID);
	if (it == _channelsFlags.end())
	{
		std::cerr << "Error: No perm for channel " << channelID << std::endl;
		return false; 
	}
	std::size_t search = it->second.find(flag, 0);
	if (search != it->second.npos)
	{
		std::cerr << "Error: User already have flag '" << flag << "'!" << std::endl;
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
		std::cerr << "Error: No perm for channel " << channelID << std::endl;
		return false; 
	}
	std::size_t search = it->second.find(flag, 0);
	if (search == it->second.npos)
	{
		std::cerr << "Error: User not have flag '" << flag << "'!" << std::endl;
		return false; 
	}
	it->second.erase(search, 1);
	return true;
}

std::string User::getFlags(std::size_t channelId)
{
	return getFlagsIndex(channelId)->second;
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

void User::disconnectChannel(std::size_t channelID)
{
	flagsPair::iterator it = getFlagsIndex(channelID);
	if ( it == this->_channelsFlags.end() )
	{
		std::cerr << "User " << this->_username << "aren't connected to channel " << channelID << "!\n";
		return ;
	}
	this->_channelsFlags.erase(it);
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
	return src->getUsernameUser() == name;
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
