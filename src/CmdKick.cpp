#include "Command.hpp"
#include <set>

static void	sendKickMessages(Server &srv, Client &client, Channel *channel, const std::string &targetNick, const std::string &reason)
{
	std::string line = clientPrefix(client) + "KICK " + channel->getName() + " " + targetNick;
	if (!reason.empty())
		line += " :" + reason;
	sendToMembers(srv, channel, line);
}

static void	kickOne(Server &srv, Client &client, const std::string &channelName, const std::string &targetNick, const std::string &reason)
{
	Channel	*channel = findMemberChannel(srv, client, channelName);
	if (channel == NULL || !requireOperator(srv, client, channel))
		return;
	Client *target = findClientOrError(srv, client, targetNick);
	if (target == NULL)
		return ;
	int	targetFd = target->getFd();
	if (!channel->isMember(targetFd))
	{
		srv.sendTo(client, numeric(client, "441", targetNick + " " + channelName));
		return ;
	}
	sendKickMessages(srv, client, channel, targetNick, reason);
	removeMemberAndDeleteChannel(srv, channel, channelName, targetFd);
}

void	cmdKick(Server &srv, Client &client, const Message &msg)
{
	if (!requireRegistered(srv, client))
		return;
	if (msg.params.size() < 2)
	{
		srv.sendTo(client, numeric(client, "461", "KICK"));
		return;
	}
	std::vector<std::string> channels = split(msg.params[0], ',');
	std::vector<std::string> targets = split(msg.params[1], ',');
	if (channels.size() != 1 && targets.size() != channels.size())
	{
		srv.sendTo(client, numeric(client, "461", "KICK"));
		return;
	}
	std::string	reason;
	if (msg.params.size() > 2)
		reason = msg.params[2];

	if (channels.size() == 1)
	{
		for (size_t i = 0; i < targets.size(); i++)
		{
			if (targets[i].empty())
				continue;
			kickOne(srv, client, channels[0], targets[i], reason);
		}
		return;
	}
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (!channels[i].empty())
			kickOne(srv, client, channels[i], targets[i], reason);
	}
}
