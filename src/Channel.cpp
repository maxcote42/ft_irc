#include "../includes/Channel.hpp"

Channel::Channel(const std::string & name) : _name(name), _modei(false), _modet(false), _modek(false), _passwordk(""), _model(0)
{
}
Channel::~Channel()
{
}

memberReturn	Channel::addMember(int fd, const std::string &key)
{
	if (isMember(fd))
		return ALREADY_MEMBER;
	if (_model > 0 && (getNumberMember() >= _model))
		return CHANNEL_FULL;
	if (_modei && !isInvited(fd))
		return MEMBER_INVITE_ONLY;
	if (_modek && key != _passwordk)
		return MEMBER_BAD_KEY;
	_members.insert(fd);
	_memberOrder.push_back(fd);
	_invited.erase(fd);
	return MEMBER_ADDED;
}

memberReturn	Channel::removeMember(int fd)
{
	_invited.erase(fd);
	if (!isMember(fd))
		return MEMBER_NOTFOUND ;
	_members.erase(fd);
	_operators.erase(fd);
	for (std::vector<int>::iterator memberIt = _memberOrder.begin();
		memberIt != _memberOrder.end(); memberIt++)
	{
		if (*memberIt == fd)
		{
			_memberOrder.erase(memberIt);
			break;
		}
	}
	return 	MEMBER_REMOVED ;
}

operatorReturn	Channel::addOperator(int fd)
{
	if (!isMember(fd))
		return OPERATOR_NOTMEMBER;
	if (isOperator(fd))
		return ALREADY_OPERATOR;
	_operators.insert(fd);
	return OPERATOR_ADDED;
}

int	Channel::promoteFirstMember(bool removedOperator)
{
	if (!removedOperator || !_operators.empty() || _memberOrder.empty())
		return -1;
	_operators.insert(_memberOrder.front());
	return _memberOrder.front();
}

operatorReturn	Channel::removeOperator(int fd)
{
	if (!isOperator(fd))
		return OPERATOR_NOTFOUND;
	_operators.erase(fd);
	return OPERATOR_REMOVED;
}

invitedReturn	Channel::addInvited(int fd)
{
	if (isMember(fd))
		return INVITED_ALREADYMEMBER;
	if (isInvited(fd))
		return ALREADY_INVITED;
	_invited.insert(fd);
	return INVITED_ADDED;
}

invitedReturn	Channel::removeInvited(int fd)
{
	if (!isInvited(fd))
		return INVITED_NOTFOUND;
	_invited.erase(fd);
	return INVITED_REMOVED;
}

bool	Channel::isEmpty() const
{
	return (_members.empty());
}

int	Channel::getNumberMember() const
{
	return (_members.size());
}

std::string	Channel::getName() const
{
	return (_name);
}

std::string	Channel::getTopic() const
{
	return (_topic);
}

bool	Channel::getModei() const
{
	return (_modei);
}

bool	Channel::getModet() const
{
	return (_modet);
}

bool	Channel::getModek() const
{
	return (_modek);
}

int	Channel::getModel() const
{
	return (_model);
}

std::string	Channel::getPassw() const
{
	return (_passwordk);
}

const	std::set<int>	&Channel::getMembers() const
{
	return (_members);
}

void	Channel::setTopic(const std::string & name)
{
	_topic = name;
}

void	Channel::setModei(bool what)
{
	_modei = what;
}
void	Channel::setModet(bool what)
{
	_modet = what;
}

void	Channel::setModek(bool what, const std::string & password)
{
	_modek = what;
	if (what)
		_passwordk = password;
	else
		_passwordk.clear();
}

void	Channel::setModel(int max)
{
	_model = max;
}

bool	Channel::isMember(int fd) const
{
	return (_members.count(fd));
}

bool	Channel::isOperator(int fd) const
{
	return (_operators.count(fd));
}

bool	Channel::isInvited(int fd) const
{
	return (_invited.count(fd));
}
