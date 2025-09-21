#include <iostream>
#include <thread>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void receiveMessages(int sock) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes <= 0) {
            std::cout << "Disconnected from server.\n";
            close(sock);
            exit(0);
        }
        std::cout << buffer;
    }
}

int main() {
    int sock;
    struct sockaddr_in serv_addr{};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    std::cout << "Connected to chat server!\n";

    // Start receiver thread
    std::thread receiver(receiveMessages, sock);
    receiver.detach();

    // Main input loop
    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "quit") {
            std::cout << "Closing connection.\n";
            close(sock);
            break;
        }
        send(sock, input.c_str(), input.size(), 0);
    }

    return 0;
}
