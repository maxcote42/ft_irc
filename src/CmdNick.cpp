#include "Command.hpp"
#include <cctype>
#include <map>
#include <set>

static bool	isNickChar(char c, bool first)
{
	if (std::isalpha(static_cast<unsigned char>(c)))
		return true;
	if (c == '[' || c == ']' || c == '\\' || c == '^' || c == '_'
		|| c == '`' || c == '{' || c == '|' || c == '}')
		return true;
	if (!first && (std::isdigit(static_cast<unsigned char>(c)) || c == '-'))
		return true;
	return false;
}

static bool	isValidNick(const std::string &nick)
{
	if (nick.empty())
		return false;
	for (size_t i = 0; i < nick.size(); i++)
	{
		if (!isNickChar(nick[i], i == 0))
			return false;
	}
	return true;
}

static void broadcastNick(Server &srv, Client &client, const std::string &oldNick, const std::string &newNick)
{
	std::string line = ":" + oldNick + "!" + client.getUser() + "@host NICK :" + newNick;

	std::set<int> recipients;

	const std::map<std::string, Channel *> &channels = srv.getChannels();

	for (std::map<std::string, Channel *>::const_iterator channelIt = channels.begin(); channelIt != channels.end(); channelIt++)
	{
		if (channelIt->second->isMember(client.getFd()))
		{
			const std::set<int> &members = channelIt->second->getMembers();

			for (std::set<int>::const_iterator memberIt = members.begin();
				memberIt != members.end(); memberIt++)
				recipients.insert(*memberIt);
		}
	}
	recipients.insert(client.getFd());
	for (std::set<int>::const_iterator recipientIt = recipients.begin();
		recipientIt != recipients.end(); recipientIt++)
	{
		Client *target = srv.findClient(*recipientIt);
		if (target)
			srv.sendTo(*target, line);
	}
}


void cmdNick(Server &srv, Client &client, const Message &msg)
{
	if (msg.params.empty())
	{
		srv.sendTo(client, numeric(client, "431", "", "No nickname given"));
		return;
	}
	const std::string &nick = msg.params[0];
	if (!isValidNick(nick))
	{
		srv.sendTo(client, numeric(client, "432", nick, "Erroneous nickname"));
		return;
	}
	Client *other = srv.findClientByNick(nick);
	if (other != NULL && other != &client)
	{
		srv.sendTo(client, numeric(client, "433", nick, "Nickname is already in use"));
		return;
	}
	std::string oldNick = client.getNick();

	if (client.getWelcomed())
	{
		broadcastNick(srv, client, oldNick, nick);
		client.setNick(nick);
	}
	else
	{
		client.setNick(nick);
		tryWelcome(srv, client);
	}
}
