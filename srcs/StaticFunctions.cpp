#include "StaticFunctions.hpp"
#include "User.hpp"
#include <cstring>

std::list<User *>::iterator StaticFunctions::findByFd(std::list<User *> &usr, int fd)
{
	std::list<User *>::iterator it;
	std::list<User *>::iterator ite = usr.end();
	for (it = usr.begin(); it != ite; it++)
	{
		if ((*it)->getUserFd() == fd)
			return	it;
	}
	return	ite;
}

int StaticFunctions::checkMode(char *buff, std::string mode, char flag)
{
	if (flag == 0)
	{
		int i = 0;
		while (buff[i] != '\0' && buff[i] == ' ')
			i++;
		std::cout << i << " \n";
		if (buff[i] == '\0')
			return (1);
		if (strncmp(buff, mode.c_str(), i) != 0)
			return (1);
	}
	else 
	{
		return (0);
	}
	return (0);	
}

void StaticFunctions::SendToFd(int fd, std::string str, std::string str2, int flag)
{
	std::string toSend = str + str2 + "\r\n";
	send(fd, toSend.c_str(), toSend.length(), flag);
}
