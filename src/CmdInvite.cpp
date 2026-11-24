#include "Command.hpp"

void cmdInvite(Server &srv, Client &client, const Message &msg)
{
	if (!requireRegistered(srv, client))
		return;
	if (msg.params.size() < 2)
	{
		srv.sendTo(client, numeric(client, "461", "INVITE"));
		return;
	}
	const std::string	&channel = msg.params[1];
	Channel	*channelPtr = findMemberChannel(srv, client, channel);
	if (channelPtr == NULL || !requireOperator(srv, client, channelPtr))
		return;
	const std::string	&targetNick = msg.params[0];
	Client *target = findClientOrError(srv, client, targetNick);
	if (target == NULL)
		return ;
	int	targetFd = target->getFd();
	if (channelPtr->isMember(targetFd))
	{
		srv.sendTo(client, numeric(client, "443", targetNick + " " + channel));
		return ;
	}
	channelPtr->addInvited(targetFd);
	srv.sendTo(*target, clientPrefix(client) + "INVITE " + targetNick  + " " + channelPtr->getName());
	srv.sendTo(client, numeric(client, "341", targetNick + " " + channel));
}
