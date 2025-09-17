#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // for close(), sleep()
#include <arpa/inet.h>   // for sockaddr_in, inet_pton()

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // 1. Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    // 3. Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%d\n", SERVER_IP, PORT);

    // 4. Periodically send messages
    int counter = 1;
    while (1) {
        char message[BUFFER_SIZE];
        snprintf(message, sizeof(message), "Message #%d from client\n", counter++);

        // send message
        if (write(sock, message, strlen(message)) == -1) {
            perror("write failed");
            break;
        }

        printf("Sent: %s", message);

        // try to read server response (non-blocking if server doesn't reply)
        int bytes_read = read(sock, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Server replied: %s\n", buffer);
        }

        sleep(2); // wait 2 seconds before sending next message
    }

    // 5. Close socket
    close(sock);

    return 0;
}
