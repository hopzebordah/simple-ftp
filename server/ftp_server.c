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

#define PROMPT "ftp> "
#define EXIT_COMMAND "bye"
#define CLOSING_CONNECTION "Closing the connection\n"
#define LIST_FILES "ls"
#define UPLOAD_FILE "u"
#define DOWNLOAD_FILE "d"
#define SERVER_ACK "ACK"
#define CLIENT_DONE "DONE"

#define BACKLOG 5
#define RECV_MAX_SIZE 1000
#define MAX_CLIENT_THREADS 5

#define SEND_MAX_SIZE 1000

void *client_handler(void *client_socket_fd);

void reset_send_buffer(char *send_buffer);
void reset_recv_buffer(char *recv_buffer);

void recv_string_constant(int fd, char *send_buffer, char *recv_buffer, char *str_constant);
size_t recv_data(int client_fd, char *recv_buffer);

void send_string_constant(int client_fd, char *send_buffer, char *str_constant);
size_t send_data(int client_fd, char *send_buffer, size_t send_buffer_size);

void recv_file_from_user(int client_fd, char *send_buffer, char *recv_buffer, size_t buffer_size);

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
    char send_buffer[SEND_MAX_SIZE];

    size_t bytes_received = 0;
    char recv_buffer[RECV_MAX_SIZE];

    while (strncmp(recv_buffer, EXIT_COMMAND, 3) != 0) { // while the first three chars sent by client are not bye

        send_string_constant(client_fd, send_buffer, PROMPT); // send the prompt
        bytes_received = recv_data(client_fd, recv_buffer); // receive data from client

        // TODO: if user sends ls server then display files in server ftp directory
        // TODO: if user sends d <file> then send specified file
        // TODO: if user sends u then handle it

        if (strncmp(recv_buffer, UPLOAD_FILE, sizeof(UPLOAD_FILE)) == 0) {
            send_string_constant(client_fd, send_buffer, SERVER_ACK); // send ack
            recv_file_from_user(client_fd, send_buffer, recv_buffer, RECV_MAX_SIZE); // receive the file
        }
    }

    printf("[+] Saying goodbye...\n");

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);

    current_threads--;
    pthread_exit(NULL);
}

// MARK utility functions
void reset_send_buffer(char *send_buffer) {
    memset(send_buffer, 0, SEND_MAX_SIZE);
}

void reset_recv_buffer(char *recv_buffer) {
    memset(recv_buffer, 0, RECV_MAX_SIZE);
}

// MARK: methods for recv()ing data
void recv_string_constant(int fd, char *send_buffer, char *recv_buffer, char *str_constant) {
    printf("WAITING FOR %s...\n", str_constant);
    reset_recv_buffer(recv_buffer);
    size_t bytes_received = recv_data(fd, recv_buffer);
    if (strncmp(recv_buffer, str_constant, strlen(str_constant)) == 0) {
        printf("RECEIVED %s!!!\n", str_constant);
        reset_recv_buffer(recv_buffer);
    } else {
        printf("[!] ERROR: expected %s and received %s...\nTerminating...\n", str_constant, recv_buffer);
        send_string_constant(fd, send_buffer, CLOSING_CONNECTION);
        exit(EXIT_FAILURE);
    }
}

size_t recv_data(int client_fd, char *recv_buffer) {
    reset_recv_buffer(recv_buffer);
    size_t bytes_received = recv(client_fd, recv_buffer, RECV_MAX_SIZE, 0);

    printf("received data : %s\n", recv_buffer);
    printf("received bytes : %lu\n", bytes_received);

    return bytes_received;
}

// MARK: methods for send()ing data
void send_string_constant(int client_fd, char *send_buffer, char *str_constant) {
    //printf("SENDING STRING %s...\n", str_constant);
    reset_send_buffer(send_buffer);
    size_t bytes_stored = sprintf(send_buffer, "%s", str_constant);
    size_t bytes_sent = send_data(client_fd, send_buffer, bytes_stored);
}

size_t send_data(int client_fd, char *send_buffer, size_t send_buffer_size) {
    size_t bytes_sent = send(client_fd, send_buffer, send_buffer_size, 0);
    reset_send_buffer(send_buffer);
    return bytes_sent;
}

// MARK main action functions
void recv_file_from_user(int client_fd, char *send_buffer, char *recv_buffer, size_t buffer_size) {

    size_t bytes_received, incoming_filesize;
    bytes_received = recv_data(client_fd, recv_buffer); // wait for filesize to be sent
    sscanf(recv_buffer, "%zu", &incoming_filesize);
    send_string_constant(client_fd, send_buffer, SERVER_ACK); // send ack as receipt

    char filename[260];
    memset(filename, 0, sizeof(filename));
    bytes_received = recv_data(client_fd, recv_buffer);
    sprintf(filename, "%s", recv_buffer);
    send_string_constant(client_fd, send_buffer, SERVER_ACK);

    size_t total_bytes_received = 0, bytes_written;
    while(total_bytes_received < incoming_filesize) {
        bytes_received = recv_data(client_fd, recv_buffer);
        total_bytes_received += bytes_received;
        printf("%s\n", recv_buffer);
        // write recv_buffer and amt bytes received to file with filename
    }

    send_string_constant(client_fd, send_buffer, SERVER_ACK);

    recv_string_constant(client_fd, send_buffer, recv_buffer, CLIENT_DONE);
}

size_t write_byte_array_to_file(char *filename, char *data, size_t data_size) {
    return 0;
}