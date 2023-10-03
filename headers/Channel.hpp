#include <list>
#include <iostream>
#include <algorithm>
#include <unistd.h>

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

class User;

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
		std::list<User *>	getUsers()		const;
		std::string			getChannelMod() const;

		void	setName(std::string &name);		
		void	setPassword(std::string &password);		
		void	setTopic(std::string &topic);		

		void	updateFlag(const std::string flag, User *op);
		void	addFlag(char flag, User *op);
		void	addUser(User *user);
		void	addOperator(User *op, std::string &name);
		
		void	rmFlag(char flag, User *op);
		void	kickUser(User *op, std::string &name);
		void	rmOperator(User *op, std::string &name);

		void	leaveUser(User *usr);
		void	changeTopic(User *usr, std::string newTopic);
		void	changeUserLimit(User *usr, std::size_t limit);

		bool	isUserOp(User *op);
		
	private:
		std::size_t		_id;
		std::string		_name;
		std::string		_topic;

		std::size_t		_usersLimit;
		std::string		_password;
		std::string		_channelMod;

		std::list<User *>	_users;
		
		Channel(void);
};
		
bool operator==(Channel const * src, std::string const & name);

#endif
