#include <iostream>

#ifndef DEFINE_HPP
#define DEFINE_HPP

#define RPL_WELCOME(networkname, nick) ("Welcome to the " + networkname + " Network " + nick)
#define RPL_NOTOPIC "No topic is set"


#define ERR_NOSUCHNICK "No such nick/channel"
#define ERR_NOSUCHSERVER "No such server"
#define ERR_NOSUCHCHANNEL "No such channel"
#define ERR_CANNOTSENDTOCHAN "Cannot send to channel"
#define ERR_NORECIPIENT "No recipient given"
#define ERR_NOTEXTTOSEND "No text to send"
#define ERR_UNKNOWNCOMMAND "Unknown command"
#define ERR_NONICKNAMEGIVEN "No nickname given"
#define ERR_NICKNAMEINUSE "Nickname is already in use"
#define ERR_USERNOTINCHANNEL "They aren't on that channel"
#define ERR_NOTONCHANNEL "You're not on that channel"
#define ERR_USERONCHANNEL "is already on channel"
#define ERR_NOTREGISTERED "You have not registered"
#define ERR_NEEDMOREPARAMS "Not enough parameters"
#define ERR_ALREADYREGISTERED "You may not reregister"
#define ERR_PASSWDMISMATCH "Password incorrect"
#define ERR_CHANNELISFULL "Cannot join channel (+l)"
#define ERR_UNKNOWNMODE "is unknown mode char to me"
#define ERR_INVITEONLYCHAN "Cannot join channel (+i)"
#define ERR_BADCHANNELKEY "Cannot join channel (+k)"
#define ERR_CHANOPRIVSNEEDED "You're not channel operator"
#define ERR_CANTKILLSERVER "You cant kill a server!"
#define ERR_UMODEUNKNOWNFLAG "Unknown MODE flag"
#define ERR_USERSDONTMATCH "Cant change mode for other users"
#define ERR_INVALIDKEY "Key is not well-formed"

#endif