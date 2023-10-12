#include <iostream>
#include <cstdlib>
#include "headers/Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

int main(int argc, char **argv)
{
    if (argc > 3 || argc <= 2 || argv[1][0] == '\0' || argv[2][0] == '\0')
    {
        std::cerr << "Need two arguments port and password" << std::endl;
        return (1);
    }
    std::string value = argv[1];
    char *err = NULL;
    int port = std::strtol(value.c_str(), &err, 10);
    if (err[0] != '\0' || value.length() > 5 || 65535 < port)
    {
        std::cerr << "Port argument error" << std::endl;
        return (1);
    }

	Server server(port, argv[2]);
	server.launch();
    return (0);
}
