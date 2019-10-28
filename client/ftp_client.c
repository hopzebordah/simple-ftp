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
#include <dirent.h>

#include "convo_networking.h"
#include "file_functions.h"
#include "util.h"

#define PROMPT "ftp> "
#define BYE "bye"
#define LIST "ls"
#define LIST_CLIENT_FILES "ls client"
#define LIST_SERVER_FILES "ls server"
#define UPLOAD_FILE "u"
#define DOWNLOAD_FILE "d"
#define CLIENT_DONE "DONE"

// CREATE A THREAD POOL INSTEAD OF CREATING AND KILLING THREADS. 
// ACQUIRE, NOTIFY, CLEAN THREADS?

void usage();

// action function
void receive_directory_contents(int socket_fd);
void upload_file(int socket_fd, int file_number);
void download_file(int socket_fd, int file_number);

// library combination functions
void recv_and_print_directory_strings(int socket_fd, int num_files, char *send_buffer, char *recv_buffer);
size_t read_and_send_file_to_socket(FILE *fp, size_t filesize, int socket_fd, char *send_buffer);
size_t recv_and_write_file_from_socket(FILE *fp, size_t filesize, int socket_fd, char *recv_buffer);

// global variables
char recv_buffer[BUFFER_SIZE];

char send_buffer[BUFFER_SIZE];
char *send_buffer_ptr = send_buffer;
size_t send_max_size = BUFFER_SIZE; // need access to pointer for getline() call

char filename[FILENAME_SIZE_MAX];

int main(int argc, char *argv[]) {

    if (argc > 3 || argc < 3) {
        usage();
        exit(EXIT_SUCCESS);
    }

    char *address = argv[1];
    char *port = argv[2];

    struct addrinfo hints, *addr_ptr;

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

    if (connect(socket_fd, addr_ptr->ai_addr, addr_ptr->ai_addrlen)) {
        perror("[!] error: connect");
        exit(EXIT_FAILURE);
    }

    printf("[+] Connection successfully established with ftp server\n\n");

    while (1) {
        recv_string_constant(socket_fd, send_buffer, recv_buffer, PROMPT); // assert server is sending prompt
        printf("%s", PROMPT); // print dat

        reset_buffer(send_buffer);
        size_t input_size = getline(&send_buffer_ptr, &send_max_size, stdin);
        input_size--; // strip newline, no null terminator

        if (strings_match(send_buffer, LIST_CLIENT_FILES)) {
            print_files_directory();
            printf("If you wish to upload a file enter \"u\" follwed by the file number\n\n");
        } else if (strings_match(send_buffer, LIST_SERVER_FILES)) {
            receive_directory_contents(socket_fd);
            printf("If you wish to download a file enter \"d\" followed by file number\n\n");
        } else if (strings_match(send_buffer, UPLOAD_FILE)) {
            int file_number = atoi(send_buffer + 2);
            upload_file(socket_fd, file_number);
        } else if (strings_match(send_buffer, DOWNLOAD_FILE)) {
            int file_number = atoi(send_buffer + 2); // file number starts at the second index
            download_file(socket_fd, file_number);
        } else if (strings_match(send_buffer, BYE)) {
            printf("Closing connection...\n");
            send_string_constant(socket_fd, send_buffer, BYE);
            break;
        }

        send_string_constant(socket_fd, send_buffer, CLIENT_DONE);
    }

    close(socket_fd);
    freeaddrinfo(addr_ptr);

    return 0;
}

//********************************************************************
//
// Usage Function
//
// This function prints a usage string.
//
//********************************************************************
void usage() {
    printf("Usage: ./ftp_client <ip_addr> <port>\n");
}

//********************************************************************
//
// Receive Directory Contents Function
//
// This function handshakes with the server and receives 
// data related to the server's files/ directory, and calls a
// function that receives those strings and prints them. 
// 
// Value parameters
// ---------------
// socket_fd        int         socket file descriptor
// 
// Local variables
// ---------------
// bytes_sent       size_t      bytes sent
// bytes_received   size_t      bytes received
// num_files        size_t      number of files
//
//********************************************************************
void receive_directory_contents(int socket_fd) {
    size_t bytes_sent, bytes_received;

    send_string_constant(socket_fd, send_buffer, LIST);
    recv_string_constant(socket_fd, send_buffer, recv_buffer, ACK);

    size_t num_files = recv_size_value(socket_fd, recv_buffer);
    send_string_constant(socket_fd, send_buffer, ACK);

    recv_and_print_directory_strings(socket_fd, num_files, send_buffer, recv_buffer);
}

//********************************************************************
//
// Upload File Function
//
// This function handshakes with the server and sends data
// related to a local file that is to be uploaded, then uploads that data.  
// 
// Value parameters
// ---------------
// socket_fd        int         socket file descriptor
// file_number      int         file number to be opened
// 
// Local variables
// ---------------
// fp               FILE *      pointer to file to be sent
// bytes_sent       size_t      bytes sent
// bytes_received   size_t      bytes received
// total_bytes_sent size_t      total bytes sent, should match filesize
//
//********************************************************************
void upload_file(int socket_fd, int file_number) {
    FILE *fp = open_file_by_number(filename, "r", file_number);

    size_t filesize = get_filesize(fp);

    size_t bytes_sent, bytes_received;

    bytes_sent = send_string(socket_fd, send_buffer, "u");
    recv_string_constant(socket_fd, send_buffer, recv_buffer, ACK);

    bytes_sent = send_size_value(socket_fd, send_buffer, filesize);
    recv_string_constant(socket_fd, send_buffer, recv_buffer, ACK);

    bytes_sent = send_string(socket_fd, send_buffer, filename);
    recv_string_constant(socket_fd, send_buffer, recv_buffer, ACK);

    size_t total_bytes_sent = read_and_send_file_to_socket(fp, filesize, socket_fd, send_buffer);
    recv_string_constant(socket_fd, send_buffer, recv_buffer, ACK);

    fclose(fp);

    printf("%s File \"%s\" uploaded successfully. %zu bytes sent.\n\n", PROMPT, filename, total_bytes_sent);
}

//********************************************************************
//
// Download File Function
//
// This function handshakes with the server and receives data
// related to a remote file that is to be downloaded, then receives that data.  
// 
// Value parameters
// ---------------
// socket_fd        int         socket file descriptor
// file_number      int         file number to be opened
// 
// Local variables
// ---------------
// requested_filesize   size_t      size of file to be downloaded
// total_bytes_sent     size_t      total bytes sent, should match filesize
// fp                   FILE *      pointer to file to be sent
//
//********************************************************************
void download_file(int socket_fd, int file_number) {
    send_string_constant(socket_fd, send_buffer, DOWNLOAD_FILE);
    recv_string_constant(socket_fd, send_buffer, recv_buffer, ACK);

    send_int(socket_fd, send_buffer, file_number);
    recv_string_constant(socket_fd, send_buffer, recv_buffer, ACK);

    size_t requested_filesize = recv_size_value(socket_fd, recv_buffer);
    send_string_constant(socket_fd, send_buffer, ACK);

    clear_filename_array(filename);
    recv_string(socket_fd, recv_buffer, filename);
    send_string_constant(socket_fd, send_buffer, ACK);

    FILE *fp = open_file(filename, "w");
    size_t total_bytes_received = recv_and_write_file_from_socket(fp, requested_filesize, socket_fd, recv_buffer);
    fclose(fp);

    send_string_constant(socket_fd, send_buffer, ACK);

    printf("%s File \"%s\" downloaded successfully. %zu bytes received.\n\n", PROMPT, filename, total_bytes_received);
}

//********************************************************************
//
// Receive and Print Directory Strings Function
//
// This function receives a set number of strings from the server and
// prints each string as it is received. 
// 
// Value parameters
// ---------------
// socket_fd        int         socket file descriptor
// num_files        int         number of file strings to expect
// send_buffer      char *      send buffer 
// recv_buffer      char *      receive buffer 
// 
// Local variables
// ---------------
// filename             char *      filename string for received filenames
// bytes_received       size_t      bytes received
//
//********************************************************************
void recv_and_print_directory_strings(int socket_fd, int num_files, char *send_buffer, char *recv_buffer) {
    char filename[FILENAME_SIZE_MAX];
    size_t bytes_received;
    for (int i=1; i<=num_files; i++) {
        clear_filename_array(filename);
        bytes_received = recv_string(socket_fd, recv_buffer, filename);
        send_string_constant(socket_fd, send_buffer, ACK);
        printf("%d. ", i);
        printf("%s\n", filename);
    }
}

//********************************************************************
//
// Read File and Send to Socket Function
//
// This function reads a file and sends it to the indicated socket
// in increments.  
// 
// Return values
// ---------------
// Total number of bytes sent
// 
// Value parameters
// ---------------
// fp               FILE *      pointer to file to read from 
// filesize         size_t      filesize of file to read from 
// socket_fd        int         socket file descriptor
// send_buffer      char *      send buffer 
// 
// Local variables
// ---------------
// total_bytes_sent     size_t      total bytes sent to server
// bytes_read           size_t      bytes read by fread function
// bytes_sent           size_t      incremental value of bytes sent to server
//
//********************************************************************
size_t read_and_send_file_to_socket(FILE *fp, size_t filesize, int socket_fd, char *send_buffer) {
    size_t total_bytes_sent = 0, bytes_read, bytes_sent;
    while (total_bytes_sent < filesize) {
        bytes_read = fread(send_buffer, 1, 1000, fp);
        bytes_sent = send_data(socket_fd, send_buffer, bytes_read);
        total_bytes_sent += bytes_sent;
    }
    return total_bytes_sent;
}

//********************************************************************
//
// Receive File and Write to File From Socket Function
//
// This function receives file data from the indicated socket and
// writes it to the indicated file in increments.  
// 
// Return values
// ---------------
// Total number of bytes received
// 
// Value parameters
// ---------------
// fp               FILE *      pointer to file to read from 
// filesize         size_t      filesize of file to read from 
// socket_fd        int         socket file descriptor
// recv_buffer      char *      receive buffer 
// 
// Local variables
// ---------------
// total_bytes_received     size_t      total bytes received from server
// bytes_written        size_t      bytes written by fwrite function
// bytes_received       size_t      incremental value of bytes received from server
//
//********************************************************************
size_t recv_and_write_file_from_socket(FILE *fp, size_t filesize, int socket_fd, char *recv_buffer) {
    size_t total_bytes_received = 0, bytes_written, bytes_received;
    while(total_bytes_received < filesize) {
        bytes_received = recv_data(socket_fd, recv_buffer);
        total_bytes_received += bytes_received;
        fwrite(recv_buffer, 1, bytes_received, fp);
    }
    return total_bytes_received;
}