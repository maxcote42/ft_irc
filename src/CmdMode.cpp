#include "Command.hpp"
#include <cctype>
#include <sstream>

void	sendMessToMembers(Server &srv, Client &client, Channel *chan, const std::string &c, bool sign, const std::string &arg)
{
	std::string line = clientPrefix(client) + "MODE " + chan->getName();
	if (sign)
		line += " +" + c;
	else
		line += " -" + c;
	if (!arg.empty())
		line += " " + arg;
	sendToMembers(srv, chan, line);
}

static void	manage_modek(Server &srv, Client &client, Channel *chan, const std::string &arg, bool sign)
{
	std::string channel = chan->getName();
	if (sign)
	{
		if (arg.empty())
		{
			srv.sendTo(client, numeric(client, "461", "MODE"));
			return ;		
		}
		if (chan->getModek())
		{
			srv.sendTo(client, numeric(client, "467", channel));
			return ;
		}
		chan->setModek(true, arg);
		sendMessToMembers(srv, client, chan, "k", true, arg);
	}
	else
	{
		if (!chan->getModek())
			return;
		chan->setModek(false, "");
		sendMessToMembers(srv, client, chan, "k", false, "*");
	}
}

static void	manage_modeo(Server &srv, Client &client, Channel *chan, const std::string &arg, bool sign)
{
	Client *target = findClientOrError(srv, client, arg);
	if (target == NULL)
		return ;
	int	fdtarget = target->getFd();
	if (!chan->isMember(fdtarget))
	{
		srv.sendTo(client, numeric(client, "441", arg + " " + chan->getName()));
		return ;
	}
	if (sign && !chan->isOperator(fdtarget))
	{
		chan->addOperator(fdtarget);
		sendMessToMembers(srv, client, chan, "o", true, arg);
	}
	else if (!sign && chan->isOperator(fdtarget))
	{
		chan->removeOperator(fdtarget);
		sendMessToMembers(srv, client, chan, "o", false, arg);	
	}
}

static void	manage_model(Server &srv, Client &client, Channel *chan, const std::string &arg, bool sign)
{
	if (!sign)
	{
		if (chan->getModel() == 0)
			return;
		chan->setModel(0);
		sendMessToMembers(srv, client, chan, "l", sign);
		return ;
	}
	int count = 0;

	for (size_t i = 0; i< arg.size() && count <= 65535; i++)
	{
		if (!std::isdigit(static_cast<unsigned char>(arg[i])))
		{
			count = 0;
			break ;
		}
		count = count * 10 + (arg[i] - '0');
	}
	if (arg.empty() || count <= 0 || count > 65535)
	{
		srv.sendTo(client, numeric(client, "461", "MODE"));
		return;
	}
	chan->setModel(count);
	std::ostringstream	oss;
	oss << count;
	std::string nb = oss.str();
	sendMessToMembers(srv, client, chan, "l", true, nb);
}

static void	execMode(Server &srv, Client &client, Channel *chan, const std::vector<std::string> &arg)
{
	std::string params = arg[1];
	bool	sign = true;
	size_t j = 2;
	for (size_t i = 0; i < params.size() ; i++)
	{
		switch (params[i])
		{
			case '+':
				sign = true;
				break;
			
			case '-':
				sign = false;
				break;

			case 'i':
				if (sign == chan->getModei())
					break ;
				chan->setModei(sign);
				sendMessToMembers(srv, client, chan, "i", sign);
				break;

			case 't':
				if (sign == chan->getModet())
					break ;
				chan->setModet(sign);
				sendMessToMembers(srv, client, chan, "t", sign);
				break;

			case 'k':
				if (j >= arg.size())
				{
					if (sign)
						srv.sendTo(client, numeric(client, "461", "MODE"));
					else
						manage_modek(srv, client, chan, "", sign);
				}
				else
					manage_modek(srv, client, chan, arg[j++], sign);
				break;

			case 'o':
				if (j >= arg.size())
					srv.sendTo(client, numeric(client, "461", "MODE"));
				else
					manage_modeo(srv, client, chan, arg[j++], sign);
				break;

			case 'l':
				if (sign && j >= arg.size())
					srv.sendTo(client, numeric(client, "461", "MODE"));
				else if (sign)
					manage_model(srv, client, chan, arg[j++], true);
				else
					manage_model(srv, client, chan, "", false);
				break;

			case 'b':
				srv.sendTo(client, numeric(client, "368", chan->getName()));
				break;

			default:
				srv.sendTo(client, numeric(client, "472", std::string(1, params[i])));
				break;
		}
	}
}

static void	displayMode(Server &srv, Client &client, Channel *chan)
{
	std::string	modes = "+";
	std::string	args;
	if (chan->getModei())
		modes += "i";
	if (chan->getModet())
		modes += "t";
	if (chan->getModek())
	{
		modes += "k";
		args += " " + chan->getPassw();
	}
	if (chan->getModel() > 0)
	{
		modes += "l";
		std::ostringstream	oss;
		oss << chan->getModel();
		args += " " + oss.str();
	}
	if (chan->isOperator(client.getFd()))
		modes += args;
	srv.sendTo(client, numeric(client, "324", chan->getName() + " " + modes));
}

void	cmdMode(Server &srv, Client &client, const Message &msg)
{
	if (!requireRegistered(srv, client))
		return;
	if (msg.params.size() == 0)
	{
		srv.sendTo(client, numeric(client, "461", "MODE"));
		return;
	}
	const std::string	&channel = msg.params[0];
	if (channel.empty() || channel[0] != '#')
		return;
	Channel	*chan = findMemberChannel(srv, client, channel);
	if (chan == NULL)
		return;
	if (msg.params.size() == 1)
	{
		displayMode(srv, client, chan);
		return ;
	}
	if (msg.params.size() == 2
		&& (msg.params[1] == "b" || msg.params[1] == "+b"))
	{
		srv.sendTo(client, numeric(client, "368", chan->getName()));
		return ;
	}
	if (msg.params.size() == 2
		&& (msg.params[1] == "e" || msg.params[1] == "+e"
			|| msg.params[1] == "I" || msg.params[1] == "+I"))
		return ;
	if (!requireOperator(srv, client, chan))
		return ;
	execMode(srv, client, chan, msg.params);
}
