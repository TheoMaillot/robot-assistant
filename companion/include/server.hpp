#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstdlib>
#include <optional>
#include <vector>

#include "config.h"

enum class MsgType { Command, Audio, Disconnected, Error };

struct Message {
    MsgType type;
    char command = 0;
    std::vector<float> audio;
};

class Server {
public:
    Server(int port) : port(port), server_fd(-1), client_fd(-1) {}
    ~Server() {
        if (server_fd >= 0) close(server_fd);
        if (client_fd >= 0) close(client_fd);
    }
    void open_server();

    ssize_t recv_all(int fd, void* buf, size_t len);    // receive any kind of message
    Message receive_message();                          // analyses the type and outputs the message

    void close_server();
    
private:
    int port;
    int client_fd;
    int server_fd;
};

#endif