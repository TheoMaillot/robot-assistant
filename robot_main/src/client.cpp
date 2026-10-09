#include "../include/client.hpp"

bool Client::connect_server() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Error creating client socket" << std::endl;
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, pc_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid IP address" << std::endl;
        close(sock);
        return false;
    }

    if (::connect(sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        std::cerr << "Failed to connect to the PC server (" << pc_ip << ")" << std::endl;
        close(sock);
        return false;
    }

    return true;
}

// Send any kind of data
bool Client::send_all(const void* data, size_t len) {
    const char* ptr = static_cast<const char*>(data);
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(sock, ptr + sent, len - sent, 0);
        if (n <= 0) {
            std::cerr << "send() failed" << std::endl;
            return false;
        }
        sent += static_cast<size_t>(n);
    }
    return true;
}

// send a simple char
void Client::send_command(char command) {
    char msg[2] = {CMD_TAG, command};
    send_all(msg, 2);
}

// send audio
void Client::send_audio(const std::vector<float>& audio) {
    char type = AUDIO_TAG;
    send_all(&type, 1);
    uint32_t n = htonl(static_cast<uint32_t>(audio.size()));
    send_all(&n, sizeof(n));
    send_all(audio.data(), audio.size() * sizeof(float));
}

void Client::disconnect() {
    if (sock >= 0) {
        close(sock);
        sock = -1;
    }
}