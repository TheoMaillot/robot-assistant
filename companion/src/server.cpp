#include "../include/server.hpp"

void Server::open_server() {
    Server::server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (Server::server_fd < 0) {
        std::cerr << "socket() failed: " << strerror(errno) << std::endl;
        std::exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(Server::server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(Server::server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "bind() failed: " << strerror(errno) << std::endl;
        close(Server::server_fd);
        std::exit(EXIT_FAILURE);
    }

    if (listen(Server::server_fd, 1) < 0) {
        std::cerr << "listen() failed: " << strerror(errno) << std::endl;
        close(Server::server_fd);
        std::exit(EXIT_FAILURE);
    }

    std::cout << "Waiting for a connection on the port " << PORT << "..." << std::endl;

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    Server::client_fd = accept(Server::server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    if (Server::client_fd < 0) {
        std::cerr << "accept() failed: " << strerror(errno) << std::endl;
        close(Server::server_fd);
        std::exit(EXIT_FAILURE);
    }

    std::cout << "Client connected." << std::endl;
}

ssize_t Server::recv_all(int fd, void* buf, size_t len) {
    char* ptr = static_cast<char*>(buf);
    size_t received = 0;
    while (received < len) {
        ssize_t n = recv(fd, ptr + received, len - received, 0);
        if (n <= 0) return n;
        received += static_cast<size_t>(n);
    }
    return static_cast<ssize_t>(received);
}

Message Server::receive_message() {
    char type = 0;
    ssize_t n = recv_all(client_fd, &type, 1);
    if (n <= 0) return {MsgType::Disconnected};

    if (type == 'C') {
        char cmd = 0;
        if (recv_all(client_fd, &cmd, 1) <= 0) return {MsgType::Disconnected};
        return {MsgType::Command, cmd};
    }
    else if (type == 'A') {
        uint32_t n_samples_net = 0;
        if (recv_all(client_fd, &n_samples_net, sizeof(n_samples_net)) <= 0) return {MsgType::Disconnected};
        uint32_t n_samples = ntohl(n_samples_net);

        std::vector<float> audio(n_samples);
        if (recv_all(client_fd, audio.data(), n_samples * sizeof(float)) <= 0) return {MsgType::Disconnected};
        return {MsgType::Audio, 0, std::move(audio)};
    }
    return {MsgType::Error};
}

void Server::close_server() {
    close(Server::server_fd);
    close(Server::client_fd);
}