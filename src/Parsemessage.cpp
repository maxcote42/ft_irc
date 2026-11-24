#include "Command.hpp"
#include <cctype>
#include <map>
#include <sstream>

bool parseMessage(const std::string &line, Message &msg)
{
	msg.prefix.clear();
	msg.command.clear();
	msg.params.clear();

	std::string s = line;
	while (!s.empty() && (s[s.size() - 1] == '\n' || s[s.size() - 1] == '\r'))
		s.erase(s.size() - 1);

	std::istringstream iss(s);
	std::string word;

	if (!(iss >> word))
		return false;
	if (word[0] == ':')
	{
		msg.prefix = word.substr(1);
		if (!(iss >> word))
			return false;
	}
	for (size_t i = 0; i < word.size(); i++)
		word[i] = std::toupper(static_cast<unsigned char>(word[i]));
	msg.command = word;

	while (iss >> word)
	{
		if (word[0] == ':')
		{
			std::string rest;
			std::getline(iss, rest);
			msg.params.push_back(word.substr(1) + rest);
			break;
		}
		msg.params.push_back(word);
	}
	return true;
}

typedef void (*CmdHandler)(Server &, Client &, const Message &);

void cmdQuit(Server &srv, Client &client, const Message &msg)
{
	std::string reason = "Client Quit";

	if (!msg.params.empty())
		reason = msg.params[0];

	std::string quitMsg = clientPrefix(client) + "QUIT :" + reason;
	
	std::map<std::string, Channel *> &channels = srv.getChannels();

	for (std::map<std::string, Channel *>::iterator it = channels.begin();
		it != channels.end(); it++)
	{
		if (it->second->isMember(client.getFd()))
		{
			sendToMembers(srv, it->second, quitMsg, client.getFd());
		}
	}
	client.setQuitSent(true);
	srv.sendTo(client, "ERROR :Closing Link: " + reason);
	client.setToDisconnect(true);
}


void cmdPing(Server &srv, Client &client, const Message &msg)
{
	if (msg.params.empty())
	{
		srv.sendTo(client, numeric(client, "409", "", "No origin specified"));
		return;
	}
	srv.sendTo(client, "PONG ircserv :" + msg.params[0]);
}

void cmdPong(Server &srv, Client &client, const Message &msg)
{
	(void)srv;
	(void)client;
	(void)msg;
}

static void cmdCap(Server &srv, Client &client, const Message &msg)
{
	(void)srv;
	(void)client;
	(void)msg;
}

static void cmdNames(Server &srv, Client &client, const Message &msg)
{
	if (msg.params.empty())
		return;
	Channel *channel = srv.findChannel(msg.params[0]);
	if (!channel)
	{
		srv.sendTo(client, numeric(client, "366", msg.params[0]));
		return;
	}
	std::string names;
	const std::set<int> &members = channel->getMembers();
	for (std::set<int>::const_iterator it = members.begin(); it != members.end(); it++)
	{
		Client *member = srv.findClient(*it);
		if (!member)
			continue;
		if (!names.empty())
			names += " ";
		if (channel->isOperator(*it))
			names += "@";
		names += member->getNick();
	}
	srv.sendTo(client, numeric(client, "353", "= " + channel->getName(), names));
	srv.sendTo(client, numeric(client, "366", channel->getName()));
}

static void cmdWho(Server &srv, Client &client, const Message &msg)
{
	if (msg.params.empty())
		return;
	Channel *channel = srv.findChannel(msg.params[0]);
	if (channel)
	{
		const std::set<int> &members = channel->getMembers();
		for (std::set<int>::const_iterator it = members.begin(); it != members.end(); it++)
		{
			Client *member = srv.findClient(*it);
			if (!member)
				continue;
			std::string status = channel->isOperator(*it) ? "H@" : "H";
			std::string params = channel->getName() + " " + member->getUser() + " host ircserv " + member->getNick() + " " + status;
			srv.sendTo(client, numeric(client, "352", params + " :0 " + member->getNick()));
		}
	}
	srv.sendTo(client, numeric(client, "315", msg.params[0]));
}

static void cmdList(Server &srv, Client &client, const Message &msg)
{
	(void)msg;
	const std::map<std::string, Channel *> &channels = srv.getChannels();
	for (std::map<std::string, Channel *>::const_iterator it = channels.begin(); it != channels.end(); it++)
	{
		std::ostringstream count;
		count << it->second->getNumberMember();
		srv.sendTo(client, numeric(client, "322", it->second->getName() + " " + count.str(), it->second->getTopic()));
	}
	srv.sendTo(client, numeric(client, "323"));
}

static void cmdWhois(Server &srv, Client &client, const Message &msg)
{
	if (msg.params.empty())
		return;
	Client *target = findClientOrError(srv, client, msg.params[0]);
	if (!target)
		return;
	srv.sendTo(client, numeric(client, "311", msg.params[0] + " " + target->getUser() + " host *"));
	srv.sendTo(client, numeric(client, "312", msg.params[0] + " ircserv", "ircserv"));
	srv.sendTo(client, numeric(client, "318", msg.params[0]));
}

static void sendNotice(Server &srv, Client &client, const std::string &targetName, const std::string &message)
{
	std::string line = clientPrefix(client) + "NOTICE " + targetName + " :" + message;
	if (!targetName.empty() && targetName[0] == '#')
	{
		Channel *channel = srv.findChannel(targetName);
		if (!channel || !channel->isMember(client.getFd()))
			return;
		sendToMembers(srv, channel, line, client.getFd());
	}
	else
	{
		Client *target = srv.findClientByNick(targetName);
		if (target)
			srv.sendTo(*target, line);
	}
}

static void cmdNotice(Server &srv, Client &client, const Message &msg)
{
	if (msg.params.size() < 2 || msg.params[1].empty())
		return;
	std::vector<std::string> targets = split(msg.params[0], ',');
	for (size_t i = 0; i < targets.size(); i++)
	{
		if (!targets[i].empty())
			sendNotice(srv, client, targets[i], msg.params[1]);
	}
}

bool parseCommand(Server &server, Client &client, Message &msg)
{
	static std::map<std::string, CmdHandler> cmds;

	if (cmds.empty())
	{
		cmds["PASS"]    = &cmdPass;
		cmds["NICK"]    = &cmdNick;
		cmds["USER"]    = &cmdUser;
		cmds["JOIN"]    = &cmdJoin;
		cmds["PART"]    = &cmdPart;
		cmds["PRIVMSG"] = &cmdPrivmsg;
		cmds["QUIT"]    = &cmdQuit;
		cmds["PING"]    = &cmdPing;
		cmds["KICK"]    = &cmdKick;
		cmds["INVITE"]  = &cmdInvite;
		cmds["TOPIC"]   = &cmdTopic;
		cmds["MODE"]    = &cmdMode;
		cmds["PONG"] = &cmdPong;
		cmds["CAP"] = &cmdCap;
		cmds["NAMES"] = &cmdNames;
		cmds["WHO"] = &cmdWho;
		cmds["LIST"] = &cmdList;
		cmds["WHOIS"] = &cmdWhois;
		cmds["NOTICE"] = &cmdNotice;
	}

	std::map<std::string, CmdHandler>::iterator it = cmds.find(msg.command);

	if (it == cmds.end())
	{
		server.sendTo(client, numeric(client, "421", msg.command));
		return true;
	}
	if (!client.isRegistered() && msg.command != "CAP" && msg.command != "PASS" && msg.command != "NICK"
		&& msg.command != "USER" && msg.command != "QUIT")
	{
		server.sendTo(client, numeric(client, "451"));
		return true;
	}

	it->second(server, client, msg);
	return true;
}
