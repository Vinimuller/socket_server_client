#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // for close()
#include <arpa/inet.h>   // for sockaddr_in, inet_ntoa()
#include <netinet/in.h>  // for INADDR_ANY
#include <sys/select.h>  // for select()

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS  FD_SETSIZE  // limit handled by select()

int main() {
    int server_fd, client_fd, max_fd, activity;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Set of socket descriptors
    fd_set read_fds, master_set;

    // 1. Create listening socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Listen
    if (listen(server_fd, 5) == -1) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Initialize fd sets
    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set); // add listening socket
    max_fd = server_fd;

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        read_fds = master_set; // copy master set to pass to select()

        // 5. Wait for activity on sockets
        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            break;
        }

        // 6. Check each socket
        for (int fd = 0; fd <= max_fd; fd++) {
            if (FD_ISSET(fd, &read_fds)) {
                if (fd == server_fd) {
                    // New incoming connection
                    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                    if (client_fd == -1) {
                        perror("accept failed");
                        continue;
                    }
                    printf("New connection: %s:%d\n",
                           inet_ntoa(client_addr.sin_addr),
                           ntohs(client_addr.sin_port));

                    FD_SET(client_fd, &master_set);
                    if (client_fd > max_fd) max_fd = client_fd;
                } else {
                    // Data from a client
                    int bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
                    if (bytes_read <= 0) {
                        // Client disconnected
                        printf("Client on fd %d disconnected\n", fd);
                        close(fd);
                        FD_CLR(fd, &master_set);
                    } else {
                        buffer[bytes_read] = '\0';
                        printf("Received from fd %d: %s\n", fd, buffer);

                        const char *response = "Hello from select() server!\n";
                        write(fd, response, strlen(response));
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
