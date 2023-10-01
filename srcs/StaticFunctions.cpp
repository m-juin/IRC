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

void StaticFunctions::SendToFd(int fd, std::string str, std::string str2, int flag)
{
	std::string toSend = str + str2 + "\r\n";
	send(fd, toSend.c_str(), toSend.length(), flag);
}

