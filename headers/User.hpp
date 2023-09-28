#include <list>
#include <iostream>
#include <unistd.h>
#include "Channel.hpp"

#ifndef USER_HPP
# define USER_HPP

typedef std::list<std::pair<std::size_t, std::string> > flagsPair;

class User {

    public:
        User();
        User(User const & user);
        User & operator=(User const & user);
        ~User();
		bool operator==(User const & user);
		

		User(std::size_t id, std::string username, std::string nickname, int fd);
		std::size_t	getIdUser() const;
		std::string getUsernameUser() const;
		std::string getUserNickname() const;
		int			getUserFd() const;
		
		void	setIdUser(std::size_t value);
		void	setUsernameUser(std::string value);
		void 	setUserNickname(std::string value);
		void	setUserFd(int value);

		bool	addFlag(std::size_t channelID, char flag);
		bool	rmFlag(std::size_t channelID, char flag);

		void	connectChannel(std::size_t channelID);
		void	disconnectChannel(std::size_t channelID);
		std::string  getFlags(std::size_t channelId);

    private:
		flagsPair::iterator	getFlagsIndex(std::size_t channelId);
		flagsPair 										_channelsFlags;
        std::size_t 									_id;
		std::string 									_username;
		std::string										_nickname;
		int												_fd;
};

bool operator==(User const * src, std::string const & name);

#endif
