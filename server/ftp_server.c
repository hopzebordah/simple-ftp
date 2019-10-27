#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "convo_networking.h"
#include "file_functions.h"
#include "util.h"

#define PROMPT "ftp> "
#define EXIT_COMMAND "bye"
#define LIST_FILES "ls"
#define UPLOAD_FILE "u"
#define DOWNLOAD_FILE "d"
#define CLIENT_DONE "DONE"

#define BACKLOG 5
#define MAX_CLIENT_THREADS 5

void *client_handler(void *client_socket_fd);

void send_directory_contents(int client_fd, char *send_buffer, char *recv_buffer);

void recv_file_from_user(int client_fd, char *send_buffer, char *recv_buffer);

void send_file_to_user(int client_fd, char *send_buffer, char *recv_buffer);

// TODO: CREATE PTHREAD POOL

int current_threads = 0;

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

        printf("[+] attempting to create client thread...\n");
        if (current_threads < MAX_CLIENT_THREADS) {
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, client_handler, (void *)((long)client_fd));
        } else {
            printf("[-] server is at capacity, refusing connection...\n");
            shutdown(client_fd, SHUT_RDWR);
            close(client_fd);
        }
    }

    close(socket_fd);
    freeaddrinfo(addr_ptr);

    return 0;
}

void *client_handler(void *client_socket_fd) {
    current_threads++;

    int client_fd = (int)client_socket_fd;

    size_t bytes_sent = 0;
    char send_buffer[BUFFER_SIZE];

    size_t bytes_received = 0;
    char recv_buffer[BUFFER_SIZE];

    while (!strings_match(recv_buffer, EXIT_COMMAND)) { // while the first three chars sent by client are not bye

        send_string_constant(client_fd, send_buffer, PROMPT); // send the prompt
        bytes_received = recv_data(client_fd, recv_buffer); // receive data from client

        // TODO: if user sends ls server then display files in server ftp directory
        // TODO: if user sends d <file> then send specified file
        // TODO: if user sends u then handle it

        if (strings_match(recv_buffer, LIST_FILES)) {
            send_string_constant(client_fd, send_buffer, ACK);
            send_directory_contents(client_fd, send_buffer, recv_buffer);
        } else if (strings_match(recv_buffer, UPLOAD_FILE)) {
            send_string_constant(client_fd, send_buffer, ACK); // send ack
            recv_file_from_user(client_fd, send_buffer, recv_buffer); // receive the file
        }
    }

    printf("[+] Saying goodbye...\n");

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);

    current_threads--;
    pthread_exit(NULL);
}

void send_directory_contents(int client_fd, char *send_buffer, char *recv_buffer) {
    size_t bytes_sent; 

    size_t directory_length = get_number_of_files_in_directory();

    // send number of files to expect
    bytes_sent = send_size_value(client_fd, send_buffer, directory_length);
    recv_string_constant(client_fd, send_buffer, recv_buffer, ACK);

    char filename[260];

    // send each filename in a loop
    for (int i=1; i<=directory_length; i++) {
        get_filename_by_number(filename, i);
        sprintf(filename, "%s", filename);
        send_string(client_fd, send_buffer, filename);
    }

    recv_string_constant(client_fd, send_buffer, recv_buffer, ACK);

    recv_string_constant(client_fd, send_buffer, recv_buffer, CLIENT_DONE);
}

// MARK main action functions
void recv_file_from_user(int client_fd, char *send_buffer, char *recv_buffer) {
    
    size_t incoming_filesize = recv_size_value(client_fd, recv_buffer);
    send_string_constant(client_fd, send_buffer, ACK); // send ack as receipt

    char filename[FILENAME_SIZE_MAX];
    clear_filename_array(filename);
    recv_string(client_fd, recv_buffer, filename);
    send_string_constant(client_fd, send_buffer, ACK);

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("fopen for writing");
        exit(EXIT_FAILURE);
    }

    size_t total_bytes_received = 0, bytes_written, bytes_received;
    while(total_bytes_received < incoming_filesize) {
        bytes_received = recv_data(client_fd, recv_buffer);
        total_bytes_received += bytes_received;
        printf("%s\n", recv_buffer);
        // write recv_buffer and amt bytes received to file with filename
        fwrite(recv_buffer, 1, bytes_received, fp);
    }

    fclose(fp);

    send_string_constant(client_fd, send_buffer, ACK);

    recv_string_constant(client_fd, send_buffer, recv_buffer, CLIENT_DONE);
}