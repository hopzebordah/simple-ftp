//********************************************************************
//
// Alexander Peters
// Computer Networks
// Programming Project #3: Simple FTP Server
// October 27, 2019
// Instructor: Dr. Ajay K. Katangur
//
//********************************************************************

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

// man action functions
void send_directory_contents(int client_fd, char *send_buffer, char *recv_buffer);
void recv_file_from_user(int client_fd, char *send_buffer, char *recv_buffer);
void send_file_to_user(int client_fd, char *send_buffer, char *recv_buffer);

// lib combination functions
size_t read_and_send_file_to_socket(FILE *fp, size_t filesize, int socket_fd, char *send_buffer);
size_t recv_and_write_file_from_socket(FILE *fp, size_t filesize, int socket_fd, char *recv_buffer);

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

        // printf("[+] received connection from %s\n", inet_ntoa(client_addr.sin_addr));

        if (current_threads < MAX_CLIENT_THREADS) {
            pthread_t thread_id;
            int *client_fd_ptr = &client_fd;
            pthread_create(&thread_id, NULL, client_handler, (void *)(client_fd_ptr));
            printf("[%d] %d\n", current_threads + 1, client_fd);
        } else {
            printf("[-] Server is at capacity, refusing connection...\n");
            shutdown(client_fd, SHUT_RDWR);
            close(client_fd);
        }
    }

    close(socket_fd);
    freeaddrinfo(addr_ptr);

    return 0;
}

//********************************************************************
//
// Pthread Client Handler Function
//
// This function defines the server behavior for each client thread that is created.  
// 
// Value parameters
// ---------------
// client_socket_fd     void *      void pointer for data passed to thread
// 
// Local variables
// ---------------
// client_fd            int         file descriptor, casted from thread argument
// bytes_sent           size_t      bytes sent
// send_buffer          char *      send buffer
// bytes_received       size_t      bytes received
// recv_buffer          char *      receive buffer
//
//********************************************************************
void *client_handler(void *client_socket_fd) {
    current_threads++;

    int *client_fd_ptr = (int *)client_socket_fd;
    int client_fd = *client_fd_ptr;

    size_t bytes_sent = 0;
    char send_buffer[BUFFER_SIZE];

    size_t bytes_received = 0;
    char recv_buffer[BUFFER_SIZE];

    while (!strings_match(recv_buffer, EXIT_COMMAND)) { // while recv buffer is not goodbye command

        send_string_constant(client_fd, send_buffer, PROMPT); // send the prompt
        bytes_received = recv_data(client_fd, recv_buffer); // receive data from client

        if (strings_match(recv_buffer, LIST_FILES)) {
            send_string_constant(client_fd, send_buffer, ACK);
            send_directory_contents(client_fd, send_buffer, recv_buffer);
        } else if (strings_match(recv_buffer, UPLOAD_FILE)) {
            send_string_constant(client_fd, send_buffer, ACK); // send ack
            recv_file_from_user(client_fd, send_buffer, recv_buffer); // receive the file
        } else if (strings_match(recv_buffer, DOWNLOAD_FILE)) {
            send_string_constant(client_fd, send_buffer, ACK);
            send_file_to_user(client_fd, send_buffer, recv_buffer);
        }
    }

    printf("[+] %d disconnected\n", client_fd);

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);

    current_threads--;
    pthread_exit(NULL);
}

//********************************************************************
//
// Send Directory Contents
//
// This function handshakes with the client, 
// sends data related to the local server files/ directory, and then sends that data. 
// 
// Value parameters
// ---------------
// client_fd        int         client file descriptor 
// send_buffer      char *      send buffer 
// recv_buffer      char *      receive buffer
// 
// Local variables
// ---------------
// bytes_sent           size_t      bytes sent
// directorty_length    size_t      number of files in directory
// filename             char *      filename array
//
//********************************************************************
void send_directory_contents(int client_fd, char *send_buffer, char *recv_buffer) {
    size_t bytes_sent; 

    size_t directory_length = get_number_of_files_in_directory();

    // send number of files to expect
    bytes_sent = send_size_value(client_fd, send_buffer, directory_length);
    recv_string_constant(client_fd, send_buffer, recv_buffer, ACK);

    char filename[FILENAME_SIZE_MAX];

    // send each filename in a loop
    for (int i=1; i<=directory_length; i++) {
        get_filename_by_number(filename, i);
        sprintf(filename, "%s", filename);
        send_string(client_fd, send_buffer, filename);
        recv_string_constant(client_fd, send_buffer, recv_buffer, ACK);
    }

    recv_string_constant(client_fd, send_buffer, recv_buffer, CLIENT_DONE);
}

//********************************************************************
//
// Receive File From User and Write to File From Socket Function
//
// This function handshakes with the client and receives file data from the client socket and
// writes it to the matching file in increments.  
// 
// Value parameters
// ---------------
// client_fd        int         client socket 
// send_buffer      char *      send buffer 
// recv_buffer      char *      receive buffer
// 
// Local variables
// ---------------
// incoming_filesize        size_t      filesize of incoming file
// filename                 char *      filename of incoming file
// total_bytes_received     size_t      number of bytes received from server
//
//********************************************************************
void recv_file_from_user(int client_fd, char *send_buffer, char *recv_buffer) {
    
    size_t incoming_filesize = recv_size_value(client_fd, recv_buffer);
    send_string_constant(client_fd, send_buffer, ACK); // send ack as receipt

    char filename[FILENAME_SIZE_MAX];
    clear_filename_array(filename);
    recv_string(client_fd, recv_buffer, filename);
    send_string_constant(client_fd, send_buffer, ACK);

    FILE *fp = open_file(filename, "w");
    size_t total_bytes_received = recv_and_write_file_from_socket(fp, incoming_filesize, client_fd, recv_buffer);
    fclose(fp);

    send_string_constant(client_fd, send_buffer, ACK);

    recv_string_constant(client_fd, send_buffer, recv_buffer, CLIENT_DONE);
}

//********************************************************************
//
// Read and Send File to User Socket Function
//
// This function handshakes with the client and sends file data then 
// sends it to the client. 
// 
// Value parameters
// ---------------
// client_fd        int         client socket 
// send_buffer      char *      send buffer 
// recv_buffer      char *      receive buffer
// 
// Local variables
// ---------------
// requested_file_number    int         requested file number
// filename                 char *      filename of chosen file
// fp                       FILE *      pointer to file to sent to client
// filesize                 size_t      filesize of chosen file
// total_bytes_sent         size_t      number of bytes sent to client
//
//********************************************************************
void send_file_to_user(int client_fd, char *send_buffer, char *recv_buffer) {

    int requested_file_number = recv_int(client_fd, recv_buffer);
    send_string_constant(client_fd, send_buffer, ACK);

    char filename[FILENAME_SIZE_MAX];

    // TODO: handle invalid file numbers by returning NULL

    FILE *fp = open_file_by_number(filename, "r", requested_file_number);
    size_t filesize = get_filesize(fp);

    send_size_value(client_fd, send_buffer, filesize);
    recv_string_constant(client_fd, send_buffer, recv_buffer, ACK);

    send_string(client_fd, send_buffer, filename);
    recv_string_constant(client_fd, send_buffer, recv_buffer, ACK);

    size_t total_bytes_sent = read_and_send_file_to_socket(fp, filesize, client_fd, send_buffer);
    recv_string_constant(client_fd, send_buffer, recv_buffer, ACK);

    recv_string_constant(client_fd, send_buffer, recv_buffer, CLIENT_DONE);
}