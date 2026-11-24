#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>

#define YES 1
#define NO 0

class	Client
{
private:
	int _fd;
	std::string	_nick;
	std::string	_user;
	std::string	_realname;
	std::string	_bufferin;
	std::string	_bufferout;
	bool		_passOk;
	bool		_welcomed;
	bool _toDisconnect;
	bool _quitSent;

	Client(const Client &);
	Client  &operator=(const Client &);

public:
	Client(int fd);
	~Client();

	void		addBuf(const char *str, size_t n);
	std::string	&getBuf();

	void	setNick(const std::string &nick);
	void	setUser(const std::string &user);
	void	setRealname(const std::string &realname);

	std::string	getNick() const;
	std::string	getUser() const;
	int			getFd() const;
	bool		appendOut(const std::string &line);

	bool	isBufferOutEmpty() const;
	void	cutBufferOut(size_t bytes);
	const	std::string &getBufferOut() const;

	bool	isRegistered() const;
	bool	getPassOk() const;
	void	setPassOk(bool state);

	bool getWelcomed() const;
	void setWelcomed(bool state);

	bool getToDisconnect() const;
	void setToDisconnect(bool state);
	bool getQuitSent() const;
	void setQuitSent(bool state);
};

# endif
