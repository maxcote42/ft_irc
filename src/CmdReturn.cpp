#include "Command.hpp"
#include <map>

static const std::map<std::string, std::string> &replyTexts()
{
	static std::map<std::string, std::string> texts;

	if (texts.empty())
	{

		texts["331"] = "No topic is set";
		texts["366"] = "End of /NAMES List";
		texts["315"] = "End of /WHO list";
		texts["318"] = "End of /WHOIS list";
		texts["323"] = "End of /LIST";
		texts["368"] = "End of Channel Ban List";
		texts["401"] = "No such nick/channel";
		texts["403"] = "No such channel";
		texts["404"] = "Cannot send to channel";
		texts["409"] = "No origin specified";
		texts["411"] = "No recipient given (PRIVMSG)";
		texts["412"] = "No text to send";
		texts["421"] = "Unknown command";
		texts["441"] = "They aren't on that channel";
		texts["442"] = "You're not on that channel";
		texts["443"] = "is already on channel";
		texts["451"] = "You have not registered";
		texts["461"] = "Not enough parameters";
		texts["467"] = "Channel key already set";
		texts["471"] = "Cannot join channel (+l)";
		texts["472"] = "is unknown mode char to me";
		texts["473"] = "Cannot join channel (+i)";
		texts["475"] = "Cannot join channel (+k)";
		texts["482"] = "You're not channel operator";
	}
	return (texts);
}

std::string	numeric(const Client &client, const std::string &code, const std::string &params, const std::string &text)
{
	std::string nick = client.getNick();
	if (nick.empty())
		nick = "*";

	std::string	line = ":ircserv " + code + " " + nick;
	if (!params.empty())
		line += " " + params;

	std::string	message = text;
	if (message.empty())
	{
		std::map<std::string, std::string>::const_iterator it = replyTexts().find(code);
		if (it != replyTexts().end())
			message = it->second;
	}
	if (!message.empty())
		line += " :" + message;
	return (line);
}
