#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create a socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Set socket options
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind the socket to a port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080); // Use port 8080 for this example
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));

    // Start listening for connections
    listen(server_fd, 3);

    std::cout << "Server is listening on port 8080..." << std::endl;

    return 0;
}
