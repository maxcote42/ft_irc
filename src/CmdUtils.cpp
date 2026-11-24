#include "Command.hpp"
#include <sstream>

std::string	clientPrefix(const Client &client)
{
	return ":" + client.getNick() + "!" + client.getUser() + "@host ";
}

void	sendToMembers(Server &srv, Channel *channel, const std::string &message, int excludedFd)
{
	const std::set<int> &members = channel->getMembers();

	for (std::set<int>::const_iterator it = members.begin(); it != members.end(); it++)
	{
		if (*it == excludedFd)
			continue;
		Client *member = srv.findClient(*it);
		if (member)
			srv.sendTo(*member, message);
	}
}

void	notifyOperatorPromotion(Server &srv, Channel *channel, bool removedOperator)
{
	int promotedFd = channel->promoteFirstMember(removedOperator);
	if (promotedFd == -1)
		return;
	Client *promoted = srv.findClient(promotedFd);
	if (promoted == NULL)
		return;
	std::string line = ":ircserv MODE " + channel->getName()
		+ " +o " + promoted->getNick();
	sendToMembers(srv, channel, line);
}

void	removeMemberAndDeleteChannel(Server &srv, Channel *channel, const std::string &name, int clientFd)
{
	bool removedOperator = channel->isOperator(clientFd);
	channel->removeMember(clientFd);
	if (channel->isEmpty())
		srv.deleteChannel(name);
	else if (removedOperator)
		notifyOperatorPromotion(srv, channel, removedOperator);
}

Channel	*findMemberChannel(Server &srv, Client &client, const std::string &name)
{
	Channel *channel = srv.findChannel(name);

	if (channel == NULL)
	{
		srv.sendTo(client, numeric(client, "403", name));
		return NULL;
	}
	if (!channel->isMember(client.getFd()))
	{
		srv.sendTo(client, numeric(client, "442", name));
		return NULL;
	}
	return channel;
}

Client	*findClientOrError(Server &srv, Client &client, const std::string &nick)
{
	Client *target = srv.findClientByNick(nick);

	if (target == NULL)
	{
		srv.sendTo(client, numeric(client, "401", nick));
		return NULL;
	}
	return target;
}

bool	requireOperator(Server &srv, Client &client, Channel *channel)
{
	if (channel->isOperator(client.getFd()))
		return true;
	srv.sendTo(client, numeric(client, "482", channel->getName()));
	return false;
}

bool	requireRegistered(Server &srv, Client &client)
{
	if (client.isRegistered())
		return true;
	srv.sendTo(client, numeric(client, "451"));
	return false;
}

std::vector<std::string> split(const std::string &s, char sep)
{
	std::vector<std::string>	result;
	std::istringstream			iss(s);
	std::string					item;
	
	while(std::getline(iss, item, sep))
		result.push_back(item);
	return (result);
}
