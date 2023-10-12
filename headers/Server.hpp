#include <list>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <map>
#include "Parser.hpp"
#include "Define.hpp"

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

		bool isUserCorrectlyConnected(int i, bool sendMessage);
		bool isUserAuthenticated(int i, bool printState);

		void joinChannel(std::pair<Command, std::string>cmd, int i);
		void checkPass(std::pair<Command, std::string>cmd, int i);
		void quitServer(std::pair<Command, std::string>cmd, int i);
		void changeModeChannel(std::pair<Command, std::string>cmd, int i);

		void connexionLost(int i);

		void setTopic(std::pair<Command, std::string>cmd, int i);
		void setNickname(std::pair<Command, std::string>cmd, int i);
		void setUsername(std::pair<Command, std::string>cmd, int i);

		void kickUser(std::pair<Command, std::string>cmd, int i);
		void messageChannel(std::pair<Command, std::string>cmd, int i, User *op);
		bool killServer(std::pair<Command, std::string>cmd, User *op, int i);
		void inviteUser(std::pair<Command, std::string>cmd, int i, User *op);
		void leaveChannel(std::pair<Command, std::string>cmd, int i);

		void closeConnexionUser(int i);

	private:
		std::map<int, std::string>	_buffers;
        std::list<Channel*>			_channels;
		std::list<User *>			_users;
		std::size_t					_port;
		std::string					_password;
		struct sockaddr_in			_address;
		int							_serverFd;
		std::vector<int>			_socket;
		std::size_t					_channelNumber;
};



#endif