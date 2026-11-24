*This project has been created as part of the 42 curriculum by sfeijoei, maxcote, mvignes.*

# ft_irc

## Description

**ft_irc** is a 42 project consisting of implementing an IRC server in C++98.

The goal is to develop an IRC server that allows multiple clients to connect, communicate through channels, and exchange private messages using the IRC protocol.

**Irssi** has been chosen as the reference IRC client for development. The server must support the protocol features required by the subject and work correctly with this client.

## Instructions

### Compilation

```bash
make
```

### Execution

Start the server with a port and a password:

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 secret
```

## IRC Command Reference

The commands below can be sent with `nc`, Irssi, or any other IRC client.
Every command must end with `CRLF` (`\r\n`).

### Syntax notation

- `<parameter>`: required parameter.
- `[parameter]`: optional parameter.
- `:text`: trailing parameter, which may contain spaces.
- `#channel`: IRC channel name.

The `:` prefix is important for text containing spaces. For example, in
`USER bart 0 * :Bart Simpson`, `Bart Simpson` is transmitted as one parameter.

### Subject scope

The core command flow required by the subject and reflected in the correction
sheet is:

- authentication with `PASS`;
- choosing a nickname with `NICK`;
- identifying the user with `USER`;
- joining channels with `JOIN`;
- exchanging private messages with `PRIVMSG`;
- managing channels with `KICK`, `INVITE`, `TOPIC`, and `MODE`.

The server also accepts `CAP`, `NOTICE`, `QUIT`, `PING`, `PONG`, `NAMES`, `WHO`,
`LIST`, and `WHOIS` for compatibility with clients such as Irssi. They are
not part of the core feature list used for the evaluation.

### Connection and identification

#### `PASS`

```text
PASS <password>
```

Sends the password required by the server.

```text
PASS secret
```

#### `NICK`

```text
NICK <nickname>
```

Sets the nickname visible to other users.

```text
NICK bart
```

#### `USER`

```text
USER <username> <mode> <unused> :<realname>
```

- `<username>`: user identifier.
- `<mode>`: legacy user mode field, usually `0`. It is part of the historical
  IRC syntax, but this server does not use it to configure the client.
- `<unused>`: legacy hostname/servername field, usually `*`. It is retained by
  the protocol for compatibility and is not used by this server.
- `<realname>`: user's real name or description.

```text
USER bart 0 * :Bart Simpson
```

The `0` and `*` values are therefore normally left unchanged. They are not
where a user sets the channel modes or server permissions: channel modes are
managed later with the `MODE` command. The server mainly uses the username and
real name from this command, but all four fields are kept in the command
because they are part of the IRC protocol format.

Registration is complete once `PASS`, `NICK`, and `USER` have been accepted.

### Channels and messages

#### `JOIN`

```text
JOIN <#channel>[,#channel] [key[,key]]
```

Joins one or more channels. A key is required for a channel protected with
mode `+k`.

```text
JOIN #general
JOIN #private channel-key
```

#### `PART`

```text
PART <#channel>[,#channel] [:reason]
```

Leaves one or more channels with an optional reason.

```text
PART #general :I have to go
```

#### `PRIVMSG`

```text
PRIVMSG <target>[,<target>] :<message>
```

Sends a message to a user or channel. Multiple targets can be separated by
commas.

```text
PRIVMSG bart :Hello!
PRIVMSG #general :Hello everyone
```

### Channel operations

The following commands require the user to be a channel member. Mode changes,
invites, and kicks require channel operator privileges.

#### `TOPIC`

```text
TOPIC <#channel> [:<topic>]
```

- Without a topic: displays the current topic.
- With a topic: sets the channel topic.

```text
TOPIC #general
TOPIC #general :General discussion
```

With mode `+t`, only an operator can change the topic.

#### `INVITE`

```text
INVITE <nickname> <#channel>
```

Invites a user to join a channel, especially when it is protected by mode
`+i`.

```text
INVITE bart #private
```

#### `KICK`

```text
KICK <#channel>[,#channel] <nickname>[,<nickname>] [:reason]
```

Kicks one or more users from one or more channels. Irssi also accepts several
comma-separated nicknames for a single channel:

```text
KICK #general bart :Reason for the kick
KICK #general bart,lisa :Stop flooding
```

#### `MODE`

```text
MODE <#channel> [<modes> [arguments...]]
```

Without modes, displays the current channel configuration. The supported
channel modes are:

| Mode | Meaning | Example |
| --- | --- | --- |
| `i` | Invite-only channel | `MODE #private +i` |
| `t` | Only operators can change the topic | `MODE #general +t` |
| `k` | Protects the channel with a key | `MODE #private +k secret` |
| `o` | Adds or removes operator status | `MODE #general +o bart` |
| `l` | Limits the number of members | `MODE #general +l 20` |
| `b` | Requests the ban-list response | `MODE #general b` |

A mode can be removed with `-`:

```text
MODE #private -i
MODE #private -k
MODE #general -o bart
MODE #general -l
```

## Resources

- [RFC 1459 — IRC Protocol](https://www.rfc-editor.org/rfc/rfc1459)
- [RFC 2812 — IRC Client Protocol](https://www.rfc-editor.org/rfc/rfc2812)
- [Irssi Documentation](https://irssi.org/documentation/)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

### AI Usage

AI was used near the end of the project to create helper functions, reduce
repeated code, and improve readability.

AI was used to identify errors in edge cases and run multiple tests quickly.

AI was used to find IRC documentation and make its explanations easier to
understand.

AI was also used to write and revise this README.
