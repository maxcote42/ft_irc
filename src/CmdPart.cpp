#include "Command.hpp"
#include <set>

static void	sendPartMessages(Server &srv, Client &client, Channel *channel, const std::string &reason)
{
	std::string line = clientPrefix(client) + "PART " + channel->getName();
	if (!reason.empty())
		line += " :" + reason;
	sendToMembers(srv, channel, line);
}

static void	partOne(Server &srv, Client &client, const std::string &channelName, const std::string &reason)
{
	Channel	*channel = findMemberChannel(srv, client, channelName);
	if (channel == NULL)
		return;
	int	clientFd = client.getFd();
	sendPartMessages(srv, client, channel, reason);
	removeMemberAndDeleteChannel(srv, channel, channelName, clientFd);
}

void	cmdPart(Server &srv, Client &client, const Message &msg)
{
	if (!requireRegistered(srv, client))
		return;
	if (msg.params.empty())
	{
		srv.sendTo(client, numeric(client, "461", "PART"));
		return;
	}
	std::vector<std::string> channels = split(msg.params[0], ',');
	std::string reason;
	if (msg.params.size() > 1)
		reason = msg.params[1];
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i].empty())
			continue;
		partOne(srv, client, channels[i], reason);
	}
}
