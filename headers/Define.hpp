#include <iostream>

#ifndef DEFINE_HPP
#define DEFINE_HPP


#define RPL_WELCOME(networkname, nick) (":127.0.0.1 001 " + nick +" Welcome to the " + networkname + " Network " + nick)
#define RPL_NOTOPIC "No topic is set"
#define RPL_TOPIC(nick, channel, topic) (":127.0.0.1 332 " + nick + " " + channel + " :" + topic)
#define RPL_INVITING(nick, channel) (nick + channel)
#define RPL_NAMREPLY(channel, nickTo, nickFrom) (":127.0.0.1 353 " + nickFrom + " = " + channel + " : " + nickTo)
#define RPL_ENDOFNAMES(nick, channel) (":127.0.0.1 366 " + nick + " " + channel + " :End of /NAMES list")

#define ERR_NOSUCHNICK(nick) (nick + " :No such nick/channel")
#define ERR_NOSUCHSERVER(serv) (serv + " :No such server")
#define ERR_NOSUCHCHANNEL(chan) (chan + " :No such channel")
#define ERR_CANNOTSENDTOCHAN(chan) (chan + " :Cannot send to channel")
#define ERR_NORECIPIENT "No recipient given"
#define ERR_NOTEXTTOSEND "No text to send"
#define ERR_UNKNOWNCOMMAND(cmd) (cmd + " :Unknown command")
#define ERR_NONICKNAMEGIVEN "No nickname given"
#define ERR_NICKNAMEINUSE(nick) (nick + " :Nickname is already in use")
#define ERR_USERNOTINCHANNEL(nick, channel) (nick + " " + channel + " :They aren't on that channel")
#define ERR_NOTONCHANNEL(channel) (channel + " :You're not on that channel")
#define ERR_USERONCHANNEL(nick, channel) (nick + " " + channel + " :is already on channel")
#define ERR_NOTREGISTERED "You have not registered"
#define ERR_NEEDMOREPARAMS(cmd) (cmd + " Not enough parameters")
#define ERR_ALREADYREGISTERED "You may not reregister"
#define ERR_PASSWDMISMATCH "Password incorrect"
#define ERR_CHANNELISFULL(channel) (channel + " :Cannot join channel (+l)")
#define ERR_UNKNOWNMODE(modechar) (modechar + "is unknown mode char to me")
#define ERR_INVITEONLYCHAN(channel) (channel + " :Cannot join channel (+i)")
#define ERR_BADCHANNELKEY(channel) (channel + " :Cannot join channel (+k)")
#define ERR_CHANOPRIVSNEEDED(channel) (channel + " :You're not channel operator")
#define ERR_CANTKILLSERVER "You cant kill a server!"
#define ERR_UMODEUNKNOWNFLAG "Unknown MODE flag"
#define ERR_USERSDONTMATCH "Cant change mode for other users"
#define ERR_INVALIDKEY(channel) (channel + " :Key is not well-formed")
#define ERR_ERRONEUSNICKNAME(nick) (":127.0.0.1 432 * " + nick + " :Erroneus nickname")
#define ERR_NICKCOLLISION "Nickname collision KILL"

#endif