#include "Command.hpp"
#include <cctype>
#include <ctime>
#include <set>
#include <sstream>

static void sendChannelMessage(Server &srv, Client &client, const std::string &channelName, const std::string &message)
{
	Channel *channel = srv.findChannel(channelName);

	if (!channel)
	{
		srv.sendTo(client, numeric(client, "403", channelName));
		return;
	}

	if (!channel->isMember(client.getFd()))
	{
		srv.sendTo(client, numeric(client, "404", channelName));
		return;
	}

	std::string line = clientPrefix(client) + "PRIVMSG " + channelName + " :" + message;
	sendToMembers(srv, channel, line, client.getFd());
}

static void botReply(Server &srv, Client &client, const std::string &message)
{
	std::string answer;
	std::string command = message;

	for (size_t i = 0; i < command.size(); i++)
		command[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(command[i])));

	if (command == "help")
		answer = "Commands: help, hello, ping, time, channels";
	else if (command == "hello")
		answer = "Hello " + client.getNick() + "!";
	else if (command == "ping")
		answer = "pong";
	else if (command == "time")
	{
		std::time_t now = std::time(NULL);
		char buffer[64];
		std::strftime(buffer, sizeof(buffer), "%H:%M:%S", std::localtime(&now));
		answer = buffer;
	}
	else if (command == "channels")
	{
		std::ostringstream output;
		output << srv.getChannels().size();
		answer = output.str() + " open channel(s)";
	}
	else
		answer = "Unknown command, try: help";
	srv.sendTo(client, ":Bot!bot@host PRIVMSG " + client.getNick() + " :" + answer);
}

static bool isBotName(const std::string &nick)
{
	if (nick.size() != 3)
		return false;
	return (std::tolower(static_cast<unsigned char>(nick[0])) == 'b'
		&& std::tolower(static_cast<unsigned char>(nick[1])) == 'o'
		&& std::tolower(static_cast<unsigned char>(nick[2])) == 't');
}

static void sendPrivateMessage(Server &srv, Client &client, const std::string &targetNick, const std::string &message)
{
	if (isBotName(targetNick) && srv.findClientByNick("Bot") == NULL)
	{
		botReply(srv, client, message);
		return;
	}
	Client *target = findClientOrError(srv, client, targetNick);
	if (!target)
		return;

	std::string line = clientPrefix(client) + "PRIVMSG " + targetNick + " :" + message;

	srv.sendTo(*target, line);
}

void cmdPrivmsg(Server &srv, Client &client, const Message &msg)
{
	if (msg.params.empty())
	{
		srv.sendTo(client, numeric(client, "411", "", "No recipient given (PRIVMSG)"));
		return;
	}

	if (msg.params.size() < 2 || msg.params[1].empty())
	{
		srv.sendTo(client, numeric(client, "412", "", "No text to send"));
		return;
	}

	std::vector<std::string> targets = split(msg.params[0], ',');
	const std::string &message = msg.params[1];

	for (size_t i = 0; i < targets.size(); i++)
	{
		if (targets[i].empty())
			continue;
		if (targets[i][0] == '#')
			sendChannelMessage(srv, client, targets[i], message);
		else
			sendPrivateMessage(srv, client, targets[i], message);
	}
}
