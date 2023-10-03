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

		bool	addFlag(std::size_t channelID, char flag);//TODO: change flag for take letter and operator + or -
		bool	rmFlag(std::size_t channelID, char flag);

		void	connectChannel(std::size_t channelID);
		void	disconnectChannel(Channel *chn);

		std::string  		getFlags(std::size_t channelId);
		std::size_t			getChanId(int idx);
		std::size_t			getNbChannel() const;

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
