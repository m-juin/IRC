#include <list>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <vector>
#include "Define.hpp"


#ifndef CHANNEL_HPP
# define CHANNEL_HPP

class User;
struct Parser;

class Channel {

	public:
		Channel(std::size_t id, std::string name, User *user);
		Channel(Channel const & channel);
		Channel & operator=(Channel const & channel);
		~Channel();
		bool operator==(Channel const & src);

		std::string 		getName()		const;		
		std::string 		getPassword()	const;		
		std::string 		getTopic()		const;
		std::size_t 		getId() 		const;
		std::size_t 		getUserLimit()	const;
		std::list<User *>	getUsers()		const;
		std::string			getChannelMod() const;

		void	setName(std::string &name);
		void	setPassword(std::string &password);
		void	setUserLimit(std::size_t usr);
		void	setTopic(std::string &topic);

		void	updateFlag(std::string cmd, User *op);

		void	addFlag(char flag);
		void	addUser(User *user, int opState);
		void	addOperator(User *op, std::string &name);
		void	connectToChannel(User *user, int opState);
		
		void	rmFlag(char flag, User *op);
		void	kickUser(User *op, std::vector<std::string> args);
		void	rmOperator(User *op, std::string &name);
		void	inviteUser(User *op, User *target);
		void	leaveUser(User *usr);
		void	changeTopic(User *usr, std::string newTopic);
		void	changeUserLimit(User *op, std::size_t limit);

		void	sendToEveryuser(std::string toSend);
		void	sendToEveryuserNotHimself(std::string toSend, User *him);

		bool	isUserOp(User *op);
		bool	isFlagPresent(char flag);
		void	kickOneUser(User *op, std::string name, std::string reason);
		
	private:
		std::size_t		_id;
		std::string		_name;
		std::string		_topic;

		std::size_t		_usersLimit;
		std::string		_password;
		std::string		_channelMod;

		std::list<User *>	_users;
		std::list<User *>	_invitedUsers;
		
		Channel(void);
};
		
bool operator==(Channel const * src, std::string const & name);

#endif
