#include "Server.hpp"
#include <iostream>

static bool	argsCheck(int ac, char **av, int &port)
{
	if (ac != 3)
	{
		std::cerr << "Error: executable must be run as follows: ./ircserv <port> <password>" << std::endl;
		return (false);
	}
	if (!Server::parsePort(av[1], port))
	{
		std::cerr << "Error: port must be a number between 1024 and 65535." << std::endl;
		return (false);
	}
	if (!av[2][0])
	{
		std::cerr << "Error: password must not be empty." << std::endl;
		return (false);
	}
	return (true);
}

int	main(int ac, char **av)
{
	int	port;

	if (!argsCheck(ac, av, port))
		return (1);
	Server	server(port, av[2]);
	if (!server.init())
		return (1);
	server.run();
	return (0);
}
