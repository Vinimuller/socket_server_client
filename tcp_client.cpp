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
            std::cout << "Disconectou do server.\n";
            close(sock);
            exit(0);
        }
        std::cout << buffer;
    }
}

int main() {
    int sock;
    struct sockaddr_in serv_addr{};

    // Criar socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Endereço Inválido/Endereço não suportado");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Conexão Falhou");
        return -1;
    }

    std::cout << "Conectado ao server de chat!\n";

    // Start receiver thread
    std::thread receiver(receiveMessages, sock);
    receiver.detach();

    // Main input loop
    std::string input;
    while (true) {
        std::getline(std::cin, input);
        if (input == "quit") {
            std::cout << "Encerrando conexão.\n";
            close(sock);
            break;
        }
        send(sock, input.c_str(), input.size(), 0);
    }

    return 0;
}
