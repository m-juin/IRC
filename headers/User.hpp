#include <list>
#include <iostream>
#include <unistd.h>
#include "StaticFunctions.hpp"
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

		std::size_t	getId()			const;
		std::string getUsername()	const;
		std::string getNickname()	const;
		int			getFd() 		const;
		
		void	setId(std::size_t value);
		void	setUsername(std::string value);
		void 	setNickname(std::string value);
		void	setFd(int value);

		void	updateFlag(std::size_t channelID, const std::string flag);
		bool	addFlag(std::size_t channelID, char flag);
		bool	rmFlag(std::size_t channelID, char flag);

		void	connectChannel(std::size_t channelID);
		bool	disconnectChannel(Channel *chn);

		std::string  	getFlags(std::size_t channelId);
		std::size_t		getChanId();
		std::size_t		getNbChannel() const;
		bool			isConnected(std::size_t id);


    private:
		flagsPair::iterator		getFlagsIndex(std::size_t channelId);

		flagsPair 				_channelsFlags;
        std::size_t 			_id;
		std::string 			_username;
		std::string				_nickname;
		int						_fd;
};

bool operator==(User const * src, std::string const & name);

#endif
