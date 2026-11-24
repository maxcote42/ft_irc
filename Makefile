NAME		= ircserv

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -Iincludes

SRCS		= main.cpp \
			  src/Server.cpp \
			  src/Client.cpp \
			  src/Channel.cpp \
			  src/Parsemessage.cpp \
			  src/CmdJoin.cpp \
			  src/CmdKick.cpp \
			  src/CmdAuth.cpp \
			  src/CmdUtils.cpp \
			  src/CmdTopic.cpp \
			  src/CmdPart.cpp \
			  src/CmdInvite.cpp \
			  src/CmdMode.cpp \
			  src/CmdReturn.cpp \
			  src/CmdPrivmsg.cpp \
			  src/CmdNick.cpp

HEADERS		= includes/Server.hpp \
			  includes/Client.hpp \
			  includes/Channel.hpp \
			  includes/Command.hpp
OBJS_DIR	= objects
OBJS		= $(SRCS:%.cpp=$(OBJS_DIR)/%.o)

all:	$(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJS_DIR)/%.o: %.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR)

fclean:	clean
	rm -f $(NAME)

re:	fclean all

.PHONY: all clean fclean re