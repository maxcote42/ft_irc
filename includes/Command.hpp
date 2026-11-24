#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <string>
#include <vector>

#include "Server.hpp"

bool	parseMessage(const std::string &line, Message &msg);
bool	parseCommand(Server &server, Client &client, Message &msg);
void	cmdJoin(Server &srv, Client &client, const Message &msg);
void	cmdKick(Server &Server, Client &client, const Message &msg);
void	cmdPass(Server &srv, Client &client, const Message &msg);
void	cmdNick(Server &srv, Client &client, const Message &msg);
void	cmdUser(Server &srv, Client &client, const Message &msg);
void	cmdPart(Server &srv, Client &client, const Message &msg);
void	cmdInvite(Server &srv, Client &client, const Message &msg);
void	cmdTopic(Server &srv, Client &client, const Message &msg);
void	cmdMode(Server &srv, Client &client, const Message &msg);
void	cmdPrivmsg(Server &srv, Client &client, const Message &msg);
void	tryWelcome(Server &srv, Client &client);
std::string	clientPrefix(const Client &client);
void	sendToMembers(Server &srv, Channel *chan, const std::string &message, int excludedFd = -1);
void	notifyOperatorPromotion(Server &srv, Channel *channel, bool removedOperator);
void	removeMemberAndDeleteChannel(Server &srv, Channel *chan, const std::string &name, int fd);
Channel	*findMemberChannel(Server &srv, Client &client, const std::string &name);
Client	*findClientOrError(Server &srv, Client &client, const std::string &nick);
bool	requireOperator(Server &srv, Client &client, Channel *chan);
bool	requireRegistered(Server &srv, Client &client);

std::string	numeric(const Client &client, const std::string &code, const std::string &params = "", const std::string &text = "");
std::vector<std::string> split(const std::string &s, char sep);
void	sendMessToMembers(Server &srv, Client &client, Channel *chan, const std::string &c, bool sign, const std::string &arg = "");


# endif
