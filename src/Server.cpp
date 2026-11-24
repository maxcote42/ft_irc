#include "../includes/Server.hpp"
#include "Command.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cctype>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <new>

volatile sig_atomic_t Server::_stop = 0;

static const size_t MAX_IRC_LINE = 512;

static std::string channelKey(const std::string &name)
{
	std::string key = name;
	for (size_t i = 0; i < key.size(); i++)
		key[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(key[i])));
	return key;
}

static std::string logValue(const std::string &value)
{
	if (value.empty())
		return "*";
	return value;
}

static void logClientEvent(const Client &client, const std::string &event)
{
	std::cout << "[INFO] fd=" << client.getFd()
		<< " nick=" << logValue(client.getNick())
		<< " user=" << logValue(client.getUser())
		<< " event=" << event << std::endl;
}

static void logClientCommand(const Client &client, const Message &msg)
{
	std::cout << "[INFO] fd=" << client.getFd()
		<< " nick=" << logValue(client.getNick())
		<< " user=" << logValue(client.getUser())
		<< " command=" << msg.command;
	if (msg.command == "NICK" && !msg.params.empty())
		std::cout << " value=" << msg.params[0];
	else if ((msg.command == "PRIVMSG" || msg.command == "NOTICE")
		&& !msg.params.empty())
		std::cout << " target=" << msg.params[0];
	else if (!msg.params.empty() && msg.command != "PASS"
		&& msg.command != "USER")
		std::cout << " target=" << msg.params[0];
	std::cout << std::endl;
}

void Server::sigHandler(int)
{
	_stop = 1;
}

Server::Server(int port, const std::string &password) : _port(port), _password(password), _serverfd(-1) {}

Server::~Server()
{
	if (_serverfd != -1)
		close(_serverfd);
	for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++)
		delete (it->second);
	_channels.clear();
	for (std::map<int, Client *>::iterator	it = _clients.begin(); it != _clients.end(); it++)
		delete (it->second);
	_clients.clear();
}

bool	Server::parsePort(const char *str, int &port)
{
	if (!str || !*str)
		return (false);
	int count = 0;
	for (size_t i = 0; str[i]; i++)
	{
		if (!isdigit(str[i]))
			return (false);
		count = count * 10 + (str[i] - '0');
		if (count > 65535)
			return (false);
	}
	if (count < 1024)
		return (false);
	port = count;
	return (true);
}

bool	Server::init()
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, &Server::sigHandler);
	signal(SIGTERM, &Server::sigHandler);
	_serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverfd == -1) 
	{
		std::cerr << "Error: socket()\n";
		return (false);
	}
	if (fcntl(_serverfd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "Error: fcntl()\n";
		return (false);
	}
	int opt = 1;
	if (setsockopt(_serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "Error: setsockopt()\n";
		return (false);
	}
	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(_port);
	if (bind(_serverfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == -1)
	{
		std::cerr << "Error: bind()\n";
		return (false);
	}
	if (listen(_serverfd, SOMAXCONN) == -1)
	{
		std::cerr << "Error: listen()\n";
		return (false);
	}
	pollfd initsock;
	initsock.fd = _serverfd;
	initsock.events = POLLIN;
	initsock.revents = 0;
	_pollfds.push_back(initsock);

	return (true);
}

void	Server::acceptClient()
{
	sockaddr_in	clientAddr;
	socklen_t	len = sizeof(clientAddr);
	int clientfd = accept(_serverfd, reinterpret_cast<sockaddr *> (&clientAddr), &len);
	if (clientfd == -1)
	{
		std::cerr << "Error: accept()\n";
		return ;
	}
	if (fcntl(clientfd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "Error: fcntl()\n";
		close(clientfd);
		return ;
	}
	Client *client = NULL;
	try
	{
		client = new Client(clientfd);
		_clients[clientfd] = client;
		logClientEvent(*client, "connected");
		pollfd clientsock;
		clientsock.fd = clientfd;
		clientsock.events = POLLIN;
		clientsock.revents = 0;
		_pollfds.push_back(clientsock);
	}
	catch (const std::exception &error)
	{
		std::cerr << "Error: client setup failed: " << error.what() << std::endl;
		bool closed = false;
		std::map<int, Client *>::iterator it = _clients.find(clientfd);
		if (it != _clients.end())
		{
			delete it->second;
			_clients.erase(it);
			closed = true;
		}
		else if (client != NULL)
		{
			delete client;
			closed = true;
		}
		if (!closed)
			close(clientfd);
	}
}

void	Server::removeClient(int fd) {

	Client *client = findClient(fd);
	if (client != NULL && !client->getQuitSent())
	{
		std::string quitMsg = clientPrefix(*client) + "QUIT :Client Quit";
		for (std::map<std::string, Channel *>::iterator channelIt = _channels.begin();
			channelIt != _channels.end(); channelIt++)
		{
			if (channelIt->second->isMember(fd))
				sendToMembers(*this, channelIt->second, quitMsg, fd);
		}
		client->setQuitSent(true);
	}
	std::vector<std::string>	toDelete;
	for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); it++)
	{
		bool removedOperator = it->second->isOperator(fd);
		it->second->removeMember(fd);
		if (it->second->isEmpty())
			toDelete.push_back(it->first);
		else if (removedOperator)
			notifyOperatorPromotion(*this, it->second, removedOperator);
	}
	for (std::vector<std::string>::iterator itd = toDelete.begin(); itd != toDelete.end(); itd++)
		deleteChannel(*itd);

	std::map<int, Client *>::iterator pit = _clients.find(fd);
	if (pit != _clients.end())
	{
		logClientEvent(*pit->second, "disconnected");
		delete(pit->second);
		_clients.erase(fd);
	}
	for (std::vector<pollfd>::iterator pollIt = _pollfds.begin(); pollIt != _pollfds.end(); pollIt++)
	{
		if (pollIt->fd == fd)
		{
			_pollfds.erase(pollIt);
			break ;
		}
	}
}

bool	Server::readClient(int fd)
{
	std::map<int, Client *>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return (false);
	char	buf[512];
	ssize_t n = recv(fd, buf, sizeof(buf), 0);
	if (n <= 0)
	{
		removeClient(fd);
		return (true);
	}
	it->second->addBuf(buf, n);

	std::string &clientBuf = it->second->getBuf();
	if (clientBuf.find('\n') == std::string::npos
		&& clientBuf.size() > MAX_IRC_LINE)
	{
		removeClient(fd);
		return (true);
	}
	size_t pos;

	while (!it->second->getToDisconnect() && (pos = clientBuf.find('\n')) != std::string::npos)
	{
		std::string line = clientBuf.substr(0, pos);

		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.size() > MAX_IRC_LINE)
		{
			removeClient(fd);
			return (true);
		}

		clientBuf.erase(0, pos + 1);

		if (line.find_first_not_of(" \t") == std::string::npos)
			continue;

		Message msg;
		if (parseMessage(line, msg) && parseCommand(*this, *it->second, msg))
			logClientCommand(*it->second, msg);
	}
	return (false);
}

void	Server::run()
{
	while (!_stop)
	{
		try
		{
			for (size_t k = 1; k < _pollfds.size(); k++)
			{
				std::map<int, Client *>::iterator it = _clients.find(_pollfds[k].fd);
				_pollfds[k].events = POLLIN;
				if (it != _clients.end() && !it->second->isBufferOutEmpty())
					_pollfds[k].events |= POLLOUT;
			}
			if (poll(&_pollfds[0], _pollfds.size(), -1) == -1)
			{
				if (errno == EINTR)
					continue;
				std::cerr << "Error: poll()" << std::endl;
				break;
			}
			size_t i = 0;
			while (i < _pollfds.size())
			{
				bool removed = false;
				if (_pollfds[i].fd == _serverfd)
				{
					if (_pollfds[i].revents & POLLIN)
						acceptClient();
				}
				else
				{
					if (_pollfds[i].revents & POLLOUT)
						removed = flushClient(_pollfds[i].fd);
					if (!removed && (_pollfds[i].revents & (POLLIN | POLLHUP | POLLERR)))
						removed = readClient(_pollfds[i].fd);
				}
				if (!removed)
					i++;
			}

			std::vector<int> toRemove;
			for (std::map<int, Client *>::iterator it = _clients.begin();
				it != _clients.end(); it++)
			{
				if (it->second->getToDisconnect()
					&& it->second->isBufferOutEmpty())
				{
					toRemove.push_back(it->first);
				}
			}
			for (size_t j = 0; j < toRemove.size(); j++)
				removeClient(toRemove[j]);
		}
		catch (const std::exception &error)
		{
			std::cerr << "Error: server exception: "
				<< error.what() << std::endl;
		}
	}
}

void Server::sendTo(Client &c, const std::string &line)
{
	if (!c.appendOut(line))
		c.setToDisconnect(true);
}

bool Server::flushClient(int fd)
{
	std::map<int, Client *>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return false;
	ssize_t n = send(fd, it->second->getBufferOut().c_str(), it->second->getBufferOut().size(), 0);
	if (n <= 0)
	{
		removeClient(fd);
		return true;
	}
	it->second->cutBufferOut(n);
	return false;
}

Channel	*Server::addChannel(const std::string &name)
{
	std::string key = channelKey(name);
	std::map<std::string, Channel *>::const_iterator	it = _channels.find(key);
	if (it != _channels.end())
		return (it->second);
	Channel *channel = new Channel(name);
	_channels[key] = channel;
	return (channel);
}

void	Server::deleteChannel(const std::string &name) {
	std::map<std::string, Channel *>::iterator it = _channels.find(channelKey(name));
	if (it == _channels.end())
		return ;
	delete (it->second);
	_channels.erase(it);
}

Channel	*Server::findChannel(const std::string &name) const {
	std::map<std::string, Channel *>::const_iterator it = _channels.find(channelKey(name));
	if (it == _channels.end())
		return (NULL);
	return (it->second);
}

Client	*Server::findClient(int fd) const {
	std::map<int, Client *>::const_iterator it =  _clients.find(fd);
	if (it == _clients.end())
		return (NULL);
	return (it->second);
}
const std::string &Server::getPassword() const
{
	return _password;
}

Client* Server::findClientByNick(const std::string &nick) const
{
	std::map<int, Client *>::const_iterator it = _clients.begin();

	while (it != _clients.end())
	{
		if (it->second)
		{
			std::string current = it->second->getNick();
			if (current.size() == nick.size())
			{
				bool equal = true;
				for (size_t i = 0; i < current.size(); i++)
				{
					if (std::tolower(static_cast<unsigned char>(current[i]))
						!= std::tolower(static_cast<unsigned char>(nick[i])))
					{
						equal = false;
						break;
					}
				}
				if (equal)
					return it->second;
			}
		}
		it++;
	}
	return NULL;
}

std::map<std::string, Channel *> &Server::getChannels()
{
	return _channels;
}

const std::map<std::string, Channel *> &Server::getChannels() const
{
	return _channels;
}
