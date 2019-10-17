#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BACKLOG 5
#define PROMPT "ftp> "
#define EXIT_COMMAND  "bye"
#define CLOSING_CONNECTION "Closing the connection\n"

int main(int argc, char *argv[]) {

    if (argc < 2 || argc > 2) {
        printf("[!] Usage: ./ftp_server <PORT>\n");
        exit(EXIT_SUCCESS);
    }

    char* port = argv[1];
    struct addrinfo hints, *addr_ptr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((getaddrinfo(NULL, port, &hints, &addr_ptr) < 0)) {
        perror("[!] getaddrinfo");
        exit(EXIT_FAILURE);
    }

    int socket_fd = socket(addr_ptr->ai_family, addr_ptr->ai_socktype, 0);
    if (socket_fd < 0) {
        perror("[!] error: socket");
        exit(EXIT_FAILURE);
    }

    if ((bind(socket_fd, addr_ptr->ai_addr, addr_ptr->ai_addrlen)) < 0) {
        perror("[!] error: bind");
        exit(EXIT_FAILURE);
    }

    if ((listen(socket_fd, BACKLOG)) < 0) {
        perror("[!] error: listen");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    while (1) {
        int client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_fd < 0) {
            perror("[!] error: accept");
            exit(EXIT_FAILURE);
        }

        printf("[+] received connection from %s\n", inet_ntoa(client_addr.sin_addr));

        // dynamically allocate space for the commands sent from client
        // maybe with malloc()???
        char input_buffer[1000];
        memset(input_buffer, 0, sizeof(input_buffer));

        while (strncmp(input_buffer, EXIT_COMMAND, 3) != 0) {
            send(client_fd, PROMPT, strlen(PROMPT) + 1, 0);
            memset(input_buffer, 0, sizeof(input_buffer));
            int bytes_received = recv(client_fd, input_buffer, sizeof(input_buffer), 0);
            printf("received %d bytes\n", bytes_received);
            printf("data is %s", input_buffer);
        }

        printf("[+] closing connection from %s\n", inet_ntoa(client_addr.sin_addr));

        send(client_fd, CLOSING_CONNECTION, strlen(CLOSING_CONNECTION) + 1, 0);
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
    }

    close(socket_fd);
    freeaddrinfo(addr_ptr);

    return 0;
}