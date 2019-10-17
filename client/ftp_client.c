#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define RECV_BUFFER_SIZE 1000
#define SEND_BUFFER_SIZE 50
#define CLOSING_CONNECTION "Closing the connection\n"

void Usage();

int main(int argc, char *argv[]) {

    // char* address = "127.0.0.1";
    // char* port = "12000";

    if (argc > 3 || argc < 3) {
        Usage();
        exit(EXIT_SUCCESS);
    }

    char *address = argv[1];
    char *port = argv[2];

    struct addrinfo hints, *addr_ptr;
    char recv_buffer[RECV_BUFFER_SIZE];
    char send_buffer[SEND_BUFFER_SIZE];

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

    while (1) {
        int bytes_received = recv(socket_fd, recv_buffer, sizeof(recv_buffer), 0); // receive prompt from server
        printf("%s", recv_buffer); // print prompt
        if (strcmp(recv_buffer, CLOSING_CONNECTION) == 0) break; // if prompt is close message, kill connection
        memset(recv_buffer, 0, sizeof(recv_buffer)); // reset receiving buffer to 0s

        fgets(send_buffer, SEND_BUFFER_SIZE, stdin); // get user input string
        send(socket_fd, send_buffer, SEND_BUFFER_SIZE, 0); // send user input string to server
    }

    close(socket_fd);
    freeaddrinfo(addr_ptr);

    return 0;
}

void Usage() {
    printf("Usage: ./ftp_client <ip_addr> <port>");
}