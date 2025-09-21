#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#define PORT 8080
#define BUFFER_SIZE 1024

class Client {
public:
    std::string username;
    std::string ip;
    int port;

    Client() = default;
    Client(const std::string &u, const std::string &ipAddr, int p)
        : username(u), ip(ipAddr), port(p) {}
};

// Allowed users (username -> password)
std::map<std::string, std::string> allowedUsers = {
    {"marcio", "1234"},
    {"paula", "abcd"},
    {"vini", "pass"}
};

// Connected clients (fd -> username)
std::map<int, std::string> onlineClients;

void sendToClient(int client_fd, const std::string &msg) {
    send(client_fd, msg.c_str(), msg.size(), 0);
}

void broadcast(const std::string &msg, int exclude_fd = -1) {
    for (auto &[fd, user] : onlineClients) {
        if (fd != exclude_fd) {
            sendToClient(fd, msg);
        }
    }
}

std::string getOnlineUsers() {
    std::ostringstream oss;
    oss << "Usuários online:\n";
    for (auto &[fd, user] : onlineClients) {
        oss << " - " << user << "\n";
    }
    return oss.str();
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address{};
    socklen_t addrlen = sizeof(address);

    fd_set master_set, read_fds;
    int fd_max;

    char buffer[BUFFER_SIZE];

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);
    fd_max = server_fd;

    std::cout << "Chat server running on port " << PORT << "...\n";

    while (true) {
        read_fds = master_set;
        if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        for (int fd = 0; fd <= fd_max; fd++) {
            if (FD_ISSET(fd, &read_fds)) {
                
                if (fd == server_fd) {
                    // New connection
                    new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
                    if (new_socket < 0) {
                        perror("accept");
                        continue;
                    }
                    FD_SET(new_socket, &master_set);
                    if (new_socket > fd_max) fd_max = new_socket;

                    sendToClient(new_socket, "[Server] Bem-vindo! Faça o login no formato: <USUÁRIO> <SENHA>\n");

                } else {
                    // Handle client input
                    memset(buffer, 0, BUFFER_SIZE);
                    int bytes = recv(fd, buffer, BUFFER_SIZE - 1, 0);
                    if (bytes <= 0) {
                        // Client disconnected
                        if (onlineClients.count(fd)) {
                            std::cout << onlineClients[fd] << " desconectado.\n";
                            broadcast("[Server] " + onlineClients[fd] + " deixou o chat.\n", fd);
                            onlineClients.erase(fd);
                        }
                        close(fd);
                        FD_CLR(fd, &master_set);
                    } else {
                        std::string msg(buffer);
                        msg.erase(msg.find_last_not_of("\r\n") + 1);
                        
                        // If fd is not on the list, its a pending authentication client
                        if (!onlineClients.count(fd)) {
                            // Authenticate
                            //Use stream to parse data
                            std::istringstream iss(msg);
                            std::string username, password;
                            iss >> username >> password;

                            //Check user and password
                            if (allowedUsers.count(username) && allowedUsers[username] == password) {
                                onlineClients[fd] = username;
                                std::cout << username << " logou.\n";
                                sendToClient(fd, "[Server] Login com sucesso!\n");
                                broadcast("[Server] " + username + " entrou no chat.\n", fd);
                            } else {
                                sendToClient(fd, "[Server] Login falhou. Desconectando.\n");
                                close(fd);
                                FD_CLR(fd, &master_set);
                            }
                        } else {
                            //Already authenticated: handle commands
                            /*
                            * Get online users command: "LIST"
                            * Send Message command: "SEND <user> <message>"
                            */
                            //Use stream to parse command
                            std::istringstream iss(msg);
                            std::string command;
                            iss >> command;

                            if (command == "LIST") {
                                sendToClient(fd, getOnlineUsers());
                            } else if (command == "SEND") {
                                std::string targetUser;
                                iss >> targetUser;
                                std::string text;
                                std::getline(iss, text);
                                //Removes leading space from msg
                                if (!text.empty() && text[0] == ' ') text.erase(0,1);

                                //Search for user on online list
                                bool found = false;
                                for (auto &[cfd, user] : onlineClients) {
                                    if (user == targetUser) {
                                        sendToClient(cfd, "[Chat de " + onlineClients[fd] + "]: " + text + "\n");
                                        sendToClient(fd, "[Msg enviada para " + targetUser + "]: " + text + "\n");
                                        found = true;
                                        break;
                                    }
                                }
                                if (!found) {
                                    sendToClient(fd, "[Server] Usuário não encontrado ou não está online.\n");
                                }
                            } else {
                                sendToClient(fd, "[Server] Comando não existe. Use LIST ou SEND.\n");
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}
