#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <vector>

#include "config.h"

class Client {
public:
    Client(const std::string& ip, int port) :  pc_ip(ip), port(port), sock(-1) {} 
    ~Client() {
        if (sock >= 0) close(sock);
    }
    bool connect_server();
    bool send_all(const void* data, size_t len);        // send any kind of data
    void send_command(char command);
    void send_audio(const std::vector<float>& audio);   // send audio
    void disconnect();

private:
    std::string pc_ip;
    int port;
    int sock;
};

#endif