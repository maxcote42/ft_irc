#include "Client.hpp"
#include <unistd.h>

static const size_t MAX_OUTPUT_BUFFER = 16 * 1024 * 1024;

Client::Client (int fd) : _fd(fd), _passOk(false), _welcomed(false), _toDisconnect(false), _quitSent(false)
{
}

Client::~Client()
{
	close(_fd);
}

void	Client::addBuf(const char *str, size_t n)
{
	_bufferin.append(str, n);
}

void Client::setNick(const std::string &nick)
{
	this->_nick = nick;
}

void Client::setUser(const std::string &user)
{
	this->_user = user;
}

void Client::setRealname(const std::string &realname)
{
	this->_realname = realname;
}

std::string Client::getNick() const
{
	return this->_nick;
}

std::string Client::getUser() const
{
	return this->_user;
}

int	Client::getFd() const
{
	return (_fd);
}

std::string &Client::getBuf()
{
	return (_bufferin);
}

const std::string& Client::getBufferOut() const
{
	return _bufferout;
}

bool Client::appendOut(const std::string &line)
{
	if (line.size() + 2 > MAX_OUTPUT_BUFFER
		|| _bufferout.size() > MAX_OUTPUT_BUFFER - line.size() - 2)
		return (false);
	_bufferout += line + "\r\n";
	return (true);
}

bool Client::isBufferOutEmpty() const
{
	if (_bufferout.empty())
		return true;
	return false;
}

void Client::cutBufferOut(size_t bytes)
{
	_bufferout.erase(0, bytes);
}

bool Client::isRegistered() const 
{
	if (_passOk == true && !_nick.empty() && !_user.empty())
		return (true);
	return (false);
}

bool Client::getPassOk() const
{
	return _passOk;
}

void Client::setPassOk(bool state)
{
	this->_passOk = state;
}

bool Client::getWelcomed() const
{
	return _welcomed;
}

void Client::setWelcomed(bool state)
{
	this->_welcomed = state;
}

bool Client::getToDisconnect() const
{
	return _toDisconnect;
}

void Client::setToDisconnect(bool state)
{
	this->_toDisconnect = state;
}

bool Client::getQuitSent() const
{
	return _quitSent;
}

void Client::setQuitSent(bool state)
{
	this->_quitSent = state;
}
