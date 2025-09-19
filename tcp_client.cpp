#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>      // close(), sleep()
#include <arpa/inet.h>   // sockaddr_in, inet_pton()

constexpr char SERVER_IP[] = "127.0.0.1";
constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

int main() {
    int sock;
    struct sockaddr_in server_addr{};
    char buffer[BUFFER_SIZE];

    // 1. Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket failed");
        return 1;
    }

    // 2. Setup server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("invalid address");
        return 1;
    }

    // 3. Connect
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connection failed");
        close(sock);
        return 1;
    }

    std::cout << "Connected to server " << SERVER_IP << ":" << PORT << "\n";

    // 4. Periodic send loop
    int counter = 1;
    while (true) {
        std::string message = "Message #" + std::to_string(counter++) + " from client\n";

        if (write(sock, message.c_str(), message.size()) == -1) {
            perror("write failed");
            break;
        }
        std::cout << "Sent: " << message;

        int bytes_read = read(sock, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::cout << "Server replied: " << buffer;
        }

        sleep(2); // wait 2 seconds
    }

    close(sock);
    return 0;
}

