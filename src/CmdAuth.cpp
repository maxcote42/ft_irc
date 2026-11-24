#include "Command.hpp"

void tryWelcome(Server &srv, Client &client)
{
	if (client.isRegistered() && !client.getWelcomed())
	{
		std::string welcome = "Welcome to the IRC Network, " + client.getNick() + '!' + client.getUser() + "@host";
		srv.sendTo(client, numeric(client, "001", "", welcome));
		srv.sendTo(client, numeric(client, "002", "", "Your host is ircserv, running version 1.0"));
		srv.sendTo(client, numeric(client, "003", "", "This server was created today"));
		srv.sendTo(client, numeric(client, "004", "ircserv 1.0 o itkol "));
		client.setWelcomed(true);
	}
}

void	cmdPass(Server &srv, Client &client, const Message &msg)
{
	if (msg.params.empty())
	{
		srv.sendTo(client, numeric(client, "461", "PASS"));
		return;
	}
	if (client.isRegistered())
	{
		srv.sendTo(client, numeric(client, "462", "", "You may not reregister"));
		return;
	}
	if (msg.params[0] != srv.getPassword())
	{
		srv.sendTo(client, numeric(client, "464", "", "Password incorrect"));
		client.setToDisconnect(true);
		return;
	}
	client.setPassOk(true);
	tryWelcome(srv, client);
}

void cmdUser(Server &srv, Client &client, const Message &msg)
{
	if (msg.params.size() < 4)
	{
		srv.sendTo(client, numeric(client, "461", "USER"));
		return;
	}
	if (client.isRegistered())
	{
		srv.sendTo(client, numeric(client, "462", "", "You may not reregister"));
		return;
	}
	client.setUser(msg.params[0]);
	client.setRealname(msg.params[3]);
	tryWelcome(srv, client);
}
