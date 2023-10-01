#include <list>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include "Parser.hpp"


#ifndef SERVER_HPP
# define SERVER_HPP

class Channel;
class User;

class Server {

    public:
        Server();
        Server(Server const & server);
        Server & operator=(Server const & server);
        ~Server();

		Server(std::size_t port, std::string password);
		
		Channel *			getChannel(std::string name);
		struct sockaddr_in	getAdresse();
		int					getServerFd();

		void	launch();
		void	closeAllSocket();
		fd_set	addNewSocket();

		bool isUserCorrectlyConnected(int i);
		bool isUserAuthenticated(int i);

		void joinChannel(Parser *cmd, int i);
		void checkPass(Parser *cmd, int i);

		void setNickname(int i, Parser *cmd);
		void setUsername(int i, Parser *cmd);

		void pvtMessage(int i, Parser *cmd);

	private:
        std::list<Channel*>		_channels;
		std::list<User *>		_users;
		std::size_t				_port;
		std::string				_password;
		struct sockaddr_in		_address;
		int						_serverFd;
		std::vector<int>		_socket;
		std::size_t				_channelNumber;
};



#endif