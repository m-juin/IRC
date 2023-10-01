#include <list>
#include <iostream>
#include <netinet/in.h>

#ifndef STATICFUNCTION_HPP
#define STATICFUNCTION_HPP

class User;

class StaticFunctions
{
	public:
		static void SendToFd(int fd, std::string str, std::string str2, int flag);
		static std::list<User *>::iterator findByFd(std::list<User *> &usr, int fd);
};

#endif
