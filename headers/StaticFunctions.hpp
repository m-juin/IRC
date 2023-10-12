#include <list>
#include <iostream>
#include <netinet/in.h>
#include "Channel.hpp"

#ifndef STATICFUNCTION_HPP
#define STATICFUNCTION_HPP

class User;

class StaticFunctions
{
	public:
		static void SendToFd(int fd, std::string str, int flag);
		static std::list<User *>::iterator findByFd(std::list<User *> &usr, int fd);
		static std::vector<User *>::iterator findByFd(std::vector<User *> &usr, int fd);
		static std::list<Channel *>::iterator findChannelById(std::list<Channel *> &channels, std::size_t id);
};

#endif
