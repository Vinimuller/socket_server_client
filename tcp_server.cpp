#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>       // close()
#include <arpa/inet.h>    // sockaddr_in, inet_ntoa()
#include <netinet/in.h>   // INADDR_ANY
#include <sys/select.h>   // select()

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

struct client{
    struct sockaddr_in *socket;
    int fd;
    std::string status;
};

int main() {
    int server_fd, client_fd, max_fd;
    struct sockaddr_in server_addr{}, client_addr{};
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    std::string statusClientArr[10];

    fd_set master_set, read_fds;

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        return 1;
    }

    // 2. Allow reuse of address/port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt failed");
        close(server_fd);
        return 1;
    }

    // 3. Bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }

    // 4. Listen
    if (listen(server_fd, 5) == -1) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }

    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);
    max_fd = server_fd;

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        read_fds = master_set;

        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
        if (activity < 0) {
            perror("select error");
            break;
        }

        for (int fd = 0; fd <= max_fd; ++fd) {
            if (FD_ISSET(fd, &read_fds)) {
                if (fd == server_fd) {
                    // New connection
                    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                    if (client_fd == -1) {
                        perror("accept failed");
                        continue;
                    }
                    FD_SET(client_fd, &master_set);
                    if (client_fd > max_fd) max_fd = client_fd;

                    std::cout << "New connection from "
                              << inet_ntoa(client_addr.sin_addr)
                              << ":" << ntohs(client_addr.sin_port) << "\n";
                    
                    statusClientArr[client_fd] = "authPending\n";
                    
                } else {
                    // Existing client sent data
                    int bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
                    if (bytes_read <= 0) {
                        std::cout << "Client on fd " << fd << " disconnected\n";
                        close(fd);
                        FD_CLR(fd, &master_set);
                    } else {

                        std::cout << "Client " << fd << " is on status: " << statusClientArr[fd];

                        if(statusClientArr[fd] == "authPending"){
                            //Parse auth message
                            //Auth message: <user>:<password>

                            //response ok -> set status to online
                            //response nok and close connection
                        }else if(statusClientArr[fd] == "online"){
                            //Accept working commands
                            //command: get online List
                            //--msg = cmd:getOnline
                            //response online list: "user1;user2;user3;user4"

                            //command: send messsage
                            //--msg = cmd:sendMessage;to:<user>;msg:<message>
                        }

                        buffer[bytes_read] = '\0';
                        std::string msg(buffer);
                        std::cout << "Client " << fd << " says: " << msg;

                        std::string response = "Echo: " + msg;
                        write(fd, response.c_str(), response.size());
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
