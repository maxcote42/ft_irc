#ifndef SERVER_HPP
# define SERVER_HPP

#include <signal.h>
#include <poll.h>
#include <string>
#include <vector>
#include <map>

#include "Client.hpp"
#include "Channel.hpp"

struct Message
{
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
};

class	Server
{
private:
	int			_port;
	std::string	_password;
	int			_serverfd;

	std::map<int, Client *>	_clients;
	std::vector<pollfd>		_pollfds;
	std::map<std::string, Channel *>	_channels;

	Server(const Server &);
	Server	&operator=(const Server &);

	static volatile sig_atomic_t _stop;

public:
	Server(int port, const std::string &password);
	~Server();

	void sendTo(Client &c, const std::string &line);
	const std::string	&getPassword() const;
	static bool	parsePort(const char *str, int &port);
	bool	init();
	void	run();

	void	removeClient(int fd);
	bool	readClient(int fd);
	void	acceptClient();
	bool	flushClient(int fd);
	Client	*findClient(int fd) const;

	Channel	*addChannel(const std::string &name);
	void	deleteChannel(const std::string &name);
	Channel	*findChannel(const std::string &name) const;

	Client *findClientByNick(const std::string &nick) const;

	static void sigHandler(int);
	std::map<std::string, Channel *> &getChannels();

	const std::map<std::string, Channel *> &getChannels() const;
};

# endif
