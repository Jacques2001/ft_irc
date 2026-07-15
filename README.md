*This project has been created as part of the 42 curriculum by **jchiu | dinza-cr | wooyang**.*

## Description

**ft_irc** (Internet Relay Chat) is a project that consists of transferring data as a client through a server. The connection is established through localhost, as the subject does not require server-to-server communication.

Connections must be handled smoothly and remain non-blocking. The server must follow a strict IRC protocol, meaning it must parse and respond using the exact same syntax as a real IRC server for commands to function properly.

A client that connect to the server must enter its password with PASS <password>

Clients that connect to the server must be able to:
* NICK & USER - Set their nickname and username
* PRIVMSG - Send private messages
* JOIN - Join different channels
* PRIVMSG #<channel> - Send messages to channels

A channel operator shall be able to :
* KICK - Eject a client from the channel
* INVITE - Invite a client to a channel
* TOPIC - Change or view the channel topic
* MODE - Change the channel’s mode:
    i: Set/remove Invite-only channel
    t: Set/remove the restrictions of the TOPIC command to channel operators
    k: Set/remove the channel key (password)
    o: Give/take channel operator privilege
    l: Set/remove the user limit to channel

## Instructions

**Compilation**
To compile the project, run the following command at the root of the repository:
`make`

**Execution**
To run the program, start the server with the following syntax:
`./ircserv <port> <password>`

* `<port>`: The port number the server will listen on for incoming connections.
* `<password>`: The connection password required for clients to authenticate.

## Resources

* [Modern IRC Protocol](https://modern.ircdocs.horse/)
* [IRC Numerics Reference](https://dd.ircdocs.horse/refs/numerics/)
* [RFC 2812 - Internet Relay Chat: Client Protocol](https://www.rfc-editor.org/rfc/rfc2812)

**AI Usage**
AI was used as an assistive tool to help complete the following tasks during the project:
* **Documentation:** Understanding the basis of required C/C++ system functions, non-blocking I/O concepts, and how to properly test them.
* **Learning:** Breaking down the details of the IRC Protocol and understanding the RFC standards.