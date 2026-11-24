#include "Command.hpp"
#include <set>

static void	changeTopic(Server &srv, Client &client, Channel *channel, const std::string &topic)
{
	if (channel->getModet() && !channel->isOperator(client.getFd()))
	{
		srv.sendTo(client, numeric(client, "482", channel->getName()));
		return;
	}
	channel->setTopic(topic);
	std::string line = clientPrefix(client) + "TOPIC " + channel->getName() + " :" + topic;
	sendToMembers(srv, channel, line);
}

void	cmdTopic(Server &srv, Client &client, const Message &msg)
{
	if (!requireRegistered(srv, client))
		return;
	if (msg.params.empty())
	{
		srv.sendTo(client, numeric(client, "461", "TOPIC"));
		return;
	}
	std::string channel = msg.params[0];
	Channel	*channelPtr = findMemberChannel(srv, client, channel);
	if (channelPtr == NULL)
		return;
	if (msg.params.size() == 1)
	{
		std::string topic = channelPtr->getTopic();
		if (topic.empty())
			srv.sendTo(client, numeric(client, "331", channel));
		else
			srv.sendTo(client, numeric(client, "332", channel, topic));
	}
	else
		changeTopic(srv, client, channelPtr, msg.params[1]);

}
