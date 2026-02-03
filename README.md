# ft_irc

*This project has been created as part of the 42 curriculum by ayasar, aakyuz and igurses.*

## Description

**ft_irc** is a custom Internet Relay Chat (IRC) server written in **C++ 98**. The primary goal of this project is to implement a stable, non-blocking server capable of handling multiple clients simultaneously using a single I/O multiplexing poll loop.

This project delves deep into network programming, TCP/IP protocols, and socket manipulation without relying on forking or multi-threading, strictly adhering to the 42 norm and performance constraints.

### Key Features

* **Non-Blocking I/O:** Uses `poll()` to manage all socket operations (read/write/listen) in a single thread.
* **Multi-Client Support:** Handles multiple simultaneous connections without hanging.
* **Channel Operations:** Users can join channels, send messages to the channel, and private message other users.
* **Operator Privileges:** Includes specific commands for channel operators:
* `KICK`: Eject a client from the channel.
* `INVITE`: Invite a client to a channel.
* `TOPIC`: Change or view the channel topic.
* `MODE`: Change channel modes.


* **Channel Modes:** Implemented specific modes as required:
* `i`: Invite-only channel.
* `t`: Restrict TOPIC command to operators.
* `k`: Set/remove channel key (password).
* `o`: Give/take operator privilege.
* `l`: Set/remove user limit.



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

## Resources

### References

* [RFC 1459](https://datatracker.ietf.org/doc/html/rfc1459) - Internet Relay Chat Protocol
* [RFC 2812](https://datatracker.ietf.org/doc/html/rfc2812) - Internet Relay Chat: Client Protocol
* [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) - Essential guide for socket programming and `poll()`.

