#include "StaticFunctions.hpp"
#include "User.hpp"
#include <cstring>

std::list<User *>::iterator StaticFunctions::findByFd(std::list<User *> &usr, int fd)
{
	std::list<User *>::iterator it;
	std::list<User *>::iterator ite = usr.end();
	for (it = usr.begin(); it != ite; it++)
	{
		if ((*it)->getFd() == fd)
			return	it;
	}
	return	ite;
}

std::vector<User *>::iterator StaticFunctions::findByFd(std::vector<User *> &usr, int fd)
{
	std::vector<User *>::iterator it;
	std::vector<User *>::iterator ite = usr.end();
	for (it = usr.begin(); it != ite; it++)
	{
		if ((*it)->getFd() == fd)
			return	it;
	}
	return	ite;
}

std::list<Channel *>::iterator StaticFunctions::findChannelById(std::list<Channel *> &channels, std::size_t id)
{
	std::list<Channel *>::iterator it;
	std::list<Channel *>::iterator ite = channels.end();
	for (it = channels.begin(); it != ite; it++)
	{
		if ((*it)->getId() == id)
			return	it;
	}
	return	ite;
}

void StaticFunctions::SendToFd(int fd, std::string str, int flag)
{
	std::string toSend = str + "\r\n";
	send(fd, toSend.c_str(), toSend.length(), flag);
}

