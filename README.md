# **IRC**
<img alt="IRC" src="https://img.shields.io/static/v1?label=IRC&message=115+/+125&color=gree&style=plastic"/>


# Project
IRC is one of the three project in the 42 program sixth circle. This is a group project of the 42 common core program. In this project the students an irc (Internet Relay Chat) server.

# Involved Language(s)
* <img alt="C_Logo" src="https://upload.wikimedia.org/wikipedia/commons/thumb/1/18/ISO_C%2B%2B_Logo.svg/459px-ISO_C%2B%2B_Logo.svg.png" style="height : 50px;"/>

# Constraints
According to the subject there only three constraint for this project:
* The project files must be compiled with  ```c++ -Wall -Werror -Wextra -std=c++98```
* Forking is not allowed
* Only 1 poll() (or equivalent) can be used.

# External Functions Allowed

This project don't allow any external library.
* socket
* close
* setsockopt, getsockname
* getprotobyname, gethostbyname,
* getaddrinfo, freeaddrinfo
* bind
* connect
* listen
* accept
* htons, htonl, ntohs, ntohl
* inet_addr, inet_ntoa
* send
* recv
* signal, sigaction
* lseek
* fstat
* fcntl
* poll or equivalent

# Project breakdown
Our group splitted the workload in 4 parts:
* The parsing handled by me.
* The client class mostly handled by me.
* The server class mostly handled by [TCazenav](https://github.com/Kelvi3)
* The Channel class mostly handled by [ParesseuxRose](https://github.com/ParesseuxRose)

# Group choices

Depending on the subject, our group had to make 2 choices before starting work:
* A ``poll`` equivalent : ``select()``
* A reference client : [Hexchat](https://hexchat.github.io/)

# Mandatory Features
Following a non-exhaustive list of the commands and features requested by the subject:

|  Name   |                                                  Descripion                                                  |       Status       |
|:-------:|:------------------------------------------------------------------------------------------------------------:|:------------------:|
| **No Block**  |                               All I/O operations must be non-blocking.                            | :white_check_mark: |
| **Multi Clients** |The server must be capable of handling multiple clients at the same time and never hang.        | :white_check_mark: |
|  **PASS**   | Enter the server password to login. | :white_check_mark: |
|  **NICK**  | Define or change user nickname   | :white_check_mark: |
|  **USER**  | Define user Username. | :white_check_mark: |
| **JOIN** | Join a channel        | :white_check_mark: |
| **PRVMSG** |Send a message to a user or a channel| :white_check_mark: |
| **Operators** |  The server channels must have operators and regular users           | :white_check_mark:|
||**Channel Operator Commands**
| **KICK** |    Eject a client from the channel     | :white_check_mark: |
| **INVITE** |   Invite a client to a channel     | :white_check_mark: |
| **TOPIC** |Change or view the channel topic     | :white_check_mark: |
| **MODE** |    Change the channel’s mode:        | :white_check_mark: |
|| **Required Modes**
|**i**|Set/remove Invite-only channe|:white_check_mark: |
|**t**|Set/remove the restrictions of the TOPIC command to channel operators|:white_check_mark: |
|**k**| Set/remove the channel key (password) |:white_check_mark: |
|**o**| Give/take channel operator privilege |:white_check_mark: |
|**l**| Set/remove the user limit to channel |:white_check_mark: |

# Bonus Features
Following a list of the bonus features requested by the subject:

|  Name   |                                                  Descripion                                                  |       status       |
|:-------:|:------------------------------------------------------------------------------------------------------------:|:------------------:|
| **Files**  |                              Handle file transfer                                                | :white_check_mark: |
| **Bot** |                                         Create a bot client for the server                                     | :x: |


# Input
The project executable have 2 required input:
1. **Port**: The port number on which the IRC server will be listening to for incoming IRC connections.
2. **Password**: The connection password. It will be needed by any IRC client that tries to connect to the server.

# Usage

``make`` to compile.

``clean`` to clean all compiled file without the library file.

``fclean`` to clean all compiled file with the library file.

``re`` to fclean then make.

# Exemple
```console
foo@bar:~$ make
foo@bar:~$ ./ircserv 8000 "123" #Start the server on the port 8000 with 123 as password
```

```console
foo@bar:~$ ./nc 127.0.0.1 8000 #Open a netcat connect on local port 8000
foo@bar:~$ PASS 123 #Enter the server password
foo@bar:~$ USER user #Enter a username
foo@bar:~$ NICK nick #Enter a nickname
foo@bar:~$ :127.0.0.1 001 nick Welcome to the >ALL Network nick #The server answer proving that you are connected 
```
