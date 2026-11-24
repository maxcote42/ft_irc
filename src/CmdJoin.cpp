#include "Command.hpp"
#include <set>

static void	sendJoinMessages(Server &srv, Client &client, Channel *channel, const std::string &name)
{
	std::string names;
	const std::set<int>	&members = channel->getMembers();
	std::string	prefix = clientPrefix(client) + "JOIN " + name;
	for (std::set<int>::const_iterator memberIt = members.begin(); memberIt != members.end(); memberIt++)
	{
		Client	*member = srv.findClient(*memberIt);
		if (!member)
			continue;
		srv.sendTo(*member, prefix);
		if (!names.empty())
			names += " ";
		if (channel->isOperator(*memberIt))
			names += "@";
		names += member->getNick();
	}
	if (channel->getTopic().empty())
		srv.sendTo(client, numeric(client, "331", name));
	else
		srv.sendTo(client, numeric(client, "332", name, channel->getTopic()));
	srv.sendTo(client, numeric(client, "353", "= " + name, names));
	srv.sendTo(client, numeric(client, "366", name));
}

static void	sendJoinError(Server &srv, Client &client, const std::string &name, memberReturn result)
{
	switch (result)
	{
		case ALREADY_MEMBER:
			break;
		
		case CHANNEL_FULL:
			srv.sendTo(client, numeric(client, "471", name));
			break;

		case MEMBER_INVITE_ONLY:
			srv.sendTo(client, numeric(client, "473", name));
			break;

		case MEMBER_BAD_KEY:
			srv.sendTo(client, numeric(client, "475", name));
			break;

		default:
			break;
	}
}

static void	joinOne(Server &srv, Client &client, const std::string &name, std::string key)
{
	if (name.size() < 2 || name[0] != '#')
	{
		srv.sendTo(client, numeric(client, "403", name));
		return ;
	}
	int	clientFd = client.getFd();
	Channel *channel = srv.findChannel(name);
	if (channel == NULL)
	{
		channel = srv.addChannel(name);
		channel->addMember(clientFd, key);
		channel->addOperator(clientFd);
	}
	else
	{
		memberReturn ret = channel->addMember(clientFd, key);
		if (ret != MEMBER_ADDED)
		{
			sendJoinError(srv, client, name, ret);
			return ;
		}
	}
	sendJoinMessages(srv, client, channel, name);
}

void cmdJoin(Server &srv, Client &client, const Message &msg)
{
	if (!requireRegistered(srv, client))
		return;
	if (msg.params.empty())
	{
		srv.sendTo(client, numeric(client, "461", "JOIN"));
		return;
	}
	std::vector<std::string> channels = split(msg.params[0], ',');
	std::vector<std::string> keys;
	if (msg.params.size() > 1)
		keys = split(msg.params[1], ',');
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i].empty())
			continue;
		std::string key;
		if (i < keys.size())
			key = keys[i];
		joinOne(srv, client, channels[i], key);
	}
}
