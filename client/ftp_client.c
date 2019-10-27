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
size_t read_and_send_file_to_socket(FILE *fp, size_t filesize, int socket_fd, char *send_buffer);

char recv_buffer[BUFFER_SIZE];

char send_buffer[BUFFER_SIZE];
char *send_buffer_ptr = send_buffer;
size_t send_max_size = BUFFER_SIZE; // need access to pointer for getline() call

char filename[FILENAME_SIZE_MAX + 10];

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

    connect(socket_fd, addr_ptr->ai_addr, addr_ptr->ai_addrlen);

    while (1) {
        recv_string_constant(socket_fd, send_buffer, recv_buffer, PROMPT); // assert server is sending prompt
        printf("%s", PROMPT); // print dat

        reset_buffer(send_buffer);
        size_t input_size = getline(&send_buffer_ptr, &send_max_size, stdin);
        input_size--; // strip newline, no null terminator

        if (strings_match(send_buffer, LIST_CLIENT_FILES)) {
            print_files_directory();
        } else if (strings_match(send_buffer, LIST_SERVER_FILES)) {
            receive_directory_contents(socket_fd);
        } else if (strings_match(send_buffer, UPLOAD_FILE)) {
            int file_number = atoi(send_buffer + 2);
            upload_file(socket_fd, file_number);
        } else if (strings_match(send_buffer, DOWNLOAD_FILE)) {
            int file_number = atoi(send_buffer + 2); // file number starts at the second index
            download_file(socket_fd, file_number);
        } else if (strings_match(send_buffer, BYE)) {
            printf("GOODBYE COMMAND\n");
            send_string_constant(socket_fd, send_buffer, BYE);
            break;
        }

        send_string_constant(socket_fd, send_buffer, CLIENT_DONE);
    }

    close(socket_fd);
    freeaddrinfo(addr_ptr);

    return 0;
}

void usage() {
    printf("Usage: ./ftp_client <ip_addr> <port>\n");
}

void receive_directory_contents(int socket_fd) {
    size_t bytes_sent, bytes_received;

    send_string_constant(socket_fd, send_buffer, LIST);
    recv_string_constant(socket_fd, send_buffer, recv_buffer, ACK);

    size_t num_files = recv_size_value(socket_fd, recv_buffer);
    send_string_constant(socket_fd, send_buffer, ACK);

    char filename[FILENAME_SIZE_MAX];
    for (int i=1; i<=num_files; i++) {
        clear_filename_array(filename);
        bytes_received = recv_string(socket_fd, recv_buffer, filename);
        printf("%d. ", i);
        printf("%s\n", filename);
    }

    send_string_constant(socket_fd, send_buffer, ACK);
}

void upload_file(int socket_fd, int file_number) {

    FILE *fp = open_file_by_number(filename, file_number);

    size_t filesize = get_filesize(fp);

    size_t bytes_sent, bytes_received;

    // send u to tell server we wanna upload something
    bytes_sent = send_string(socket_fd, send_buffer, "u");
    recv_string_constant(socket_fd, send_buffer, recv_buffer, ACK);

    // tell server incoming filesize
    bytes_sent = send_size_value(socket_fd, send_buffer, filesize);
    recv_string_constant(socket_fd, send_buffer, recv_buffer, ACK);

    // tell server incoming filename
    bytes_sent = send_string(socket_fd, send_buffer, filename);
    recv_string_constant(socket_fd, send_buffer, recv_buffer, ACK);

    // send() bytes until filesize == bytes_sent
    size_t total_bytes_sent = read_and_send_file_to_socket(fp, filesize, socket_fd, send_buffer);
    recv_string_constant(socket_fd, send_buffer, recv_buffer, ACK);

    fclose(fp);
}

void download_file(int socket_fd, int file_number) {
    printf("YOU WANT TO DOWNLOAD A FILE!!!\n");
    // TODO: tell server which file we want, send number
    // TODO: recv() filesize
    // TODO: loop recv() until received bytes == filesize
}

size_t read_and_send_file_to_socket(FILE *fp, size_t filesize, int socket_fd, char *send_buffer) {
    size_t total_bytes_sent = 0, bytes_read, bytes_sent;
    while (total_bytes_sent < filesize) {
        bytes_read = fread(send_buffer, 1, 1000, fp);
        bytes_sent = send_data(socket_fd, send_buffer, bytes_read);
        total_bytes_sent += bytes_sent;
    }
    return total_bytes_sent;
}