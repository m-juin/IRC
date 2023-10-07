#include <iostream>

#ifndef DEFINE_HPP
#define DEFINE_HPP


#define RPL_WELCOME(networkname, nick) (":127.0.0.1 001 " + nick +" Welcome to the " + networkname + " Network " + nick)
#define RPL_NOTOPIC "No topic is set"
#define RPL_TOPIC(nick, channel, topic) (":127.0.0.1 332 " + nick + " " + channel + " :" + topic)
#define RPL_INVITING(nick, channel) (nick + channel)
#define RPL_NAMREPLY(channel, nickTo, nickFrom) (":127.0.0.1 353 " + nickFrom + " = " + channel + " : " + nickTo)
#define RPL_ENDOFNAMES(nick, channel) (":127.0.0.1 366 " + nick + " " + channel + " :End of /NAMES list")

#define ERR_NOSUCHNICK(nick) (":127.0.0.1 401 " + nick + " :No such nick/channel")
#define ERR_NOSUCHSERVER(serv) (":127.0.0.1 402 " + serv + " :No such server")
#define ERR_NOSUCHCHANNEL(nick, chan) (":127.0.0.1 403 " + nick + " " + chan + " :No such channel")
#define ERR_CANNOTSENDTOCHAN(nick, chan) (":127.0.0.1 404 " + nick + " " + chan + " :Cannot send to channel")
#define ERR_NORECIPIENT(nick) (":127.0.0.1 411 " + nick + " :No recipient given")
#define ERR_NOTEXTTOSEND (nick) (":127.0.0.1 412 " + nick + " :No text to send")
#define ERR_UNKNOWNCOMMAND(nick, cmd) (":127.0.0.1 421 " + nick + " " + cmd + " :Unknown command")
#define ERR_NONICKNAMEGIVEN(nick) (":127.0.0.1 431 " + nick + " :No nickname given")
#define ERR_ERRONEUSNICKNAME(nick, newNick) (":127.0.0.1 432 " + nick + " " + newNick + " :Erroneus nickname")
#define ERR_NICKNAMEINUSE(nick, newNick) (":127.0.0.1 433 " + nick + " " + newNick + " :Nickname is already in use")
#define ERR_USERNOTINCHANNEL(nick, channel) (":127.0.0.1 441 * " + nick + " " + channel + " :They aren't on that channel")
#define ERR_NOTONCHANNEL(nick, channel) (":127.0.0.1 442 " + nick + " " + channel + " :You're not on that channel")
#define ERR_USERONCHANNEL(nick, channel) (":127.0.0.1 443 * " + nick + " " + channel + " :is already on channel")
#define ERR_NOTREGISTERED(nick) (":127.0.0.1 451 " + nick + " :You have not registered")
#define ERR_NEEDMOREPARAMS(cmd) (":127.0.0.1 461 * " + cmd + " :Not enough parameters")
#define ERR_ALREADYREGISTERED(nick) (":127.0.0.1 462 " + nick + " :You may not reregister")
#define ERR_PASSWDMISMATCH ":127.0.0.1 464 * :Password incorrect"
#define ERR_CHANNELISFULL(nick, channel) (":127.0.0.1 471 " + nick + " " + channel + " :Cannot join channel (+l)")
#define ERR_UNKNOWNMODE(nick, modechar) (":127.0.0.1 472 " + nick + " " + modechar + "is unknown mode char to me")
#define ERR_INVITEONLYCHAN(nick, channel) (":127.0.0.1 473 " + nick + " " + channel + " :Cannot join channel (+i)")
#define ERR_BADCHANNELKEY(nick, channel) (":127.0.0.1 475 " + nick + " " + channel + " :Cannot join channel (+k)")
#define ERR_CHANOPRIVSNEEDED(nick, channel) (":127.0.0.1 482 " + nick + " " + channel + " :You're not channel operator")
#define ERR_CANTKILLSERVER (nick) (":127.0.0.1 483 " + nick + " :You cant kill a server!")
#define ERR_UMODEUNKNOWNFLAG(nick) (":127.0.0.1 501 " + nick +  " :Unknown MODE flag")
#define ERR_USERSDONTMATCH ":127.0.0.1 502 * :Cant change mode for other users"
#define ERR_INVALIDKEY(channel) (channel + " :Key is not well-formed")

#endif