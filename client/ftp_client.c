#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER_SIZE 1000

int main(int argc, char *argv[]) {

    char* address = "127.0.0.1";
    char* port = "12000";
    struct addrinfo hints, *addr_ptr;
    char buffer[BUFFER_SIZE];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((getaddrinfo(address, port, &hints, &addr_ptr) < 0)) {
        perror("[!] getaddrinfo");
        exit(EXIT_FAILURE);
    }

    int socket_fd = socket(addr_ptr->ai_family, addr_ptr->ai_socktype, 0);
    if (socket_fd < 0) {
        perror("[!] error: socket");
        exit(EXIT_FAILURE);
    }

    connect(socket_fd, addr_ptr->ai_addr, addr_ptr->ai_addrlen);

    int bytes_received;
    while ((bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0)) != 0) {
        printf("bytes received: %d\n", bytes_received);
        printf("%s\n", buffer);
    }

    close(socket_fd);
    freeaddrinfo(addr_ptr);

    return 0;
}