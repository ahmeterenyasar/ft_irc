# ft_irc

*This project has been created as part of the 42 curriculum by ayasar, aakyuz and igurses.*

## Description

**ft_irc** is a custom Internet Relay Chat (IRC) server written in **C++ 98**. The primary goal of this project is to implement a stable, non-blocking server capable of handling multiple clients simultaneously using a single I/O multiplexing poll loop.

This project delves deep into network programming, TCP/IP protocols, and socket manipulation without relying on forking or multi-threading, strictly adhering to the 42 norm and performance constraints.

### Key Features

* **Non-Blocking I/O:** Uses `poll()` to manage all socket operations (read/write/listen) in a single thread.
* **Multi-Client Support:** Handles multiple simultaneous connections without hanging.
* **Channel Operations:** Users can join channels, send messages to the channel, and private message other users.

## Implemented Commands

### Authentication & Registration
* **PASS** - Set connection password (must be sent before NICK/USER)
* **NICK** - Set or change user nickname
* **USER** - Set username, hostname, and realname
* **CAP** - Capability negotiation (for modern IRC clients)

### Channel Operations
* **JOIN** - Join a channel or create a new one
* **PART** - Leave a channel
* **TOPIC** - View or change channel topic
* **INVITE** - Invite a user to an invite-only channel
* **KICK** - Remove a user from a channel (operator only)
* **MODE** - Change channel or user modes

### Communication
* **PRIVMSG** - Send a private message to a user or channel
* **NOTICE** - Send a notice (similar to PRIVMSG but without auto-reply)

### Information
* **WHO** - Get information about users
* **LIST** - List all channels and their topics

### Connection
* **PING** - Test connection (server responds with PONG)
* **QUIT** - Disconnect from the server

### Channel Modes

The following channel modes are supported:

* **+i** / **-i** - Set/unset invite-only channel
* **+t** / **-t** - Set/unset topic restriction (only operators can change)
* **+k <key>** / **-k** - Set/remove channel password
* **+o <nick>** / **-o <nick>** - Give/remove operator privileges
* **+l <limit>** / **-l** - Set/remove user limit

**Example usage:**
```bash
/MODE #channel +i          # Make channel invite-only
/MODE #channel +k password # Set channel password
/MODE #channel +o username # Give operator status
/MODE #channel +l 10       # Set user limit to 10
```



## Instructions

### 1. Compilation

The project uses a `Makefile` for compilation. To build the server, run:

```bash
make

```

### 2. Execution

To start the server, you must provide a listening port and a connection password:

```bash
./ircserv <port> <password>

```

**Example:**

```bash
./ircserv 6667 password123

```

### 3. Connecting with HexChat (Reference Client)

We use **HexChat** as our reference client for evaluation.

1. Open HexChat.
2. Go to **Network List** (Ctrl+S).
3. Add a new network (e.g., "ft_irc_local").
4. Edit the network:
* **Server:** `localhost/6667` (or `127.0.0.1/6667`)
* **Password:** Enter the password you used when starting `./ircserv` (e.g., `password123`).
* **Uncheck:** "Use SSL for all servers on this network".


5. Connect and use `/join #channelname` to test.

### 4. Testing with Netcat (Subject Requirement)

To test raw data processing and command fragmentation (Ctrl+D handling), use `nc`:

```bash
nc -C 127.0.0.1 6667

```

## Bonus Features

### IRC Bot

In addition to the core IRC server functionality, we have implemented a bonus IRC bot that can:

* **Automatically respond to user commands** in channels or private messages
* **Provide helpful information** about server commands and usage
* **Execute custom commands** with the `/bot` prefix

#### Bot Usage

1. **Compile the bot:**
```bash
make bonus

./bot <port> <password> 
```



## Resources

### AI Usage

AI assistance was used for the following purposes:
* **Code documentation**: Generating comments and function descriptions.
* **Debugging**: Identifying potential issues in network socket handling.
* **Learning**: Understanding IRC protocol specifications and poll() usage.

No AI-generated code was directly copied into the project without understanding and modification.

### References

* [RFC 1459](https://datatracker.ietf.org/doc/html/rfc1459) - Internet Relay Chat Protocol
* [RFC 2812](https://datatracker.ietf.org/doc/html/rfc2812) - Internet Relay Chat: Client Protocol
* [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) - Essential guide for socket programming and `poll()`.

