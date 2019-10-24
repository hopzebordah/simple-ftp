#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>

#define RECV_BUFFER_SIZE 1000
#define INPUT_MAX_SIZE 1000
#define CLOSING_CONNECTION "Closing the connection\n"
#define LIST_CLIENT_FILES "ls client"

// CREATE A THREAD POOL INSTEAD OF CREATING AND KILLING THREADS. 
// ACQUIRE, NOTIFY, CLEAN THREADS?

void usage();

size_t get_dynamic_input(char *result, int starting_size);

int main(int argc, char *argv[]) {

    // char* address = "127.0.0.1";
    // char* port = "12000";

    if (argc > 3 || argc < 3) {
        usage();
        exit(EXIT_SUCCESS);
    }

    char *address = argv[1];
    char *port = argv[2];

    struct addrinfo hints, *addr_ptr;
    char recv_buffer[RECV_BUFFER_SIZE];

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

    char input_buffer[INPUT_MAX_SIZE];
    char *input_buffer_ptr = input_buffer;
    size_t input_max_size = INPUT_MAX_SIZE; // need access to pointer for getline() call

    while (1) {
        int bytes_received = recv(socket_fd, recv_buffer, sizeof(recv_buffer), 0); // receive prompt from server
        printf("%s", recv_buffer); // print prompt

        if (strcmp(recv_buffer, CLOSING_CONNECTION) == 0) break; // if prompt is close message, kill connection

        // here need to check if message from server is a file of some sort and loop until we receive everything

        memset(recv_buffer, 0, sizeof(recv_buffer)); // reset receiving buffer to 0s

        memset(input_buffer, 0, INPUT_MAX_SIZE); // reset input buffer to 0s before receiving input

        size_t input_size = getline(&input_buffer_ptr, &input_max_size, stdin);

        // TODO: check if string input is 'ls client' and if so display files in client directory

        if (strncmp(input_size, LIST_CLIENT_FILES, strlen(LIST_CLIENT_FILES))) {
            get_files_in_directory_v();
        }

        // TODO: check if string input is 'u <file>' and if so send file to server

        send(socket_fd, input_buffer, --input_size, 0); // send user input string to server, pre decrement input_size to remove null terminator
    }

    close(socket_fd);
    freeaddrinfo(addr_ptr);

    return 0;
}

void usage() {
    printf("Usage: ./ftp_client <ip_addr> <port>\n");
}

void get_files_in_directory_v() {
    struct dirent *de;
    DIR *dir = open("./files");
    if (dir == NULL) {
        perror("[-] could not open directory for reading");
        exit(EXIT_FAILURE);
    }
    int i = 1;
    while ((de = readdir(dir)) != NULL) {
        printf("%d. %s\n", i++, ent->d_name);
    }
    closedir(dir);
}
