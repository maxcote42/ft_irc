#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <set>
#include <vector>

enum	memberReturn
{
	MEMBER_ADDED,
	ALREADY_MEMBER,
	CHANNEL_FULL,
	MEMBER_INVITE_ONLY,
	MEMBER_BAD_KEY,
	MEMBER_REMOVED,
	MEMBER_NOTFOUND
};

enum	operatorReturn
{
	OPERATOR_ADDED,
	OPERATOR_REMOVED,
	OPERATOR_NOTMEMBER,
	ALREADY_OPERATOR,
	OPERATOR_NOTFOUND
};

enum	invitedReturn
{
	INVITED_ADDED,
	INVITED_REMOVED,
	INVITED_ALREADYMEMBER,
	INVITED_NOTFOUND,
	ALREADY_INVITED
};

class Channel
{
private:
	const std::string	_name;
	std::string			_topic;

	std::set<int>		_operators;
	std::set<int>		_members;
	std::vector<int>	_memberOrder;
	std::set<int>		_invited;
	bool				_modei;
	bool				_modet;
	bool				_modek;
	std::string			_passwordk;
	int					_model;

	Channel(const Channel &src);
	Channel	&operator=(const Channel &src);

public:
	Channel(const std::string & name);
	~Channel();
	memberReturn		addMember(int fd, const std::string &key);
	memberReturn		removeMember(int fd);
	operatorReturn		addOperator(int fd);
	operatorReturn		removeOperator(int fd);
	int					promoteFirstMember(bool removedOperator);
	invitedReturn		addInvited(int fd);
	invitedReturn		removeInvited(int fd);
	bool				isEmpty() const;
	int					getNumberMember() const;
	std::string			getName() const;
	std::string			getTopic() const;
	bool				getModei() const;
	bool				getModet() const ;
	bool				getModek() const;
	int					getModel() const;
	std::string			getPassw() const;
	const std::set<int>	&getMembers() const;
	void				setTopic(const std::string & name);
	void				setModei(bool what);
	void				setModet(bool what);
	void				setModek(bool what, const std::string & password);
	void				setModel(int max);
	bool				isMember(int fd) const;
	bool				isOperator(int fd) const;
	bool				isInvited(int fd) const;
};


# endif
