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
#define BYE "bye"
#define CLOSING_CONNECTION "Closing the connection\n"
#define LIST_CLIENT_FILES "ls client"
#define LIST_SERVER_FILES "ls server"
#define UPLOAD_FILE "u"
#define DOWNLOAD_FILE "d"

// CREATE A THREAD POOL INSTEAD OF CREATING AND KILLING THREADS. 
// ACQUIRE, NOTIFY, CLEAN THREADS?

void usage();

size_t get_dynamic_input(char *result, int starting_size);
void print_files_directory();
FILE * open_file_by_number(int file_number);
size_t get_filesize(FILE *fp);
void upload_file(int socket_fd, int file_number);

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

    char recv_buffer[RECV_BUFFER_SIZE];

    char input_buffer[INPUT_MAX_SIZE];
    char *input_buffer_ptr = input_buffer;
    size_t input_max_size = INPUT_MAX_SIZE; // need access to pointer for getline() call

    while (1) {
        memset(recv_buffer, 0, sizeof(recv_buffer)); // reset receiving buffer to 0s
        int bytes_received = recv(socket_fd, recv_buffer, sizeof(recv_buffer), 0); // receive prompt from server
        printf("%s", recv_buffer); // print first message of each interaction, whether prompt or goodbye
        if (strcmp(recv_buffer, CLOSING_CONNECTION) == 0) break; // if prompt is close message, kill connection

        // here need to check if message from server is a file of some sort and loop until we receive everything

        memset(input_buffer, 0, INPUT_MAX_SIZE); // reset input buffer to 0s before receiving input
        size_t input_size = getline(&input_buffer_ptr, &input_max_size, stdin);
        input_size--; // strip newline, no null terminator

        if (strncmp(input_buffer, LIST_CLIENT_FILES, strlen(LIST_CLIENT_FILES)) == 0) {
            printf("YOU WANT TO LIST FILES ON CLIENT!!!\n");
            print_files_directory();
        } else if (strncmp(input_buffer, LIST_SERVER_FILES, strlen(LIST_SERVER_FILES)) == 0) {
            printf("YOU WANT TO LIST FILES ON SERVER!!!\n");
            // TODO: ask server for list of files, send() ls
            // TODO: recv() size of files string
            // TODO: loop recv() until entire string is recieved
        } else if (strncmp(input_buffer, UPLOAD_FILE, strlen(UPLOAD_FILE)) == 0) {
            printf("YOU WANT TO UPLOAD A FILE!!!\n");
            int file_number = atoi(input_buffer + 2);
            upload_file(socket_fd, file_number);
        } else if (strncmp(input_buffer, DOWNLOAD_FILE, strlen(DOWNLOAD_FILE)) == 0) {
            printf("YOU WANT TO DOWNLOAD A FILE!!!\n");
            // TODO: send d to tell server we wanna download stuff
            int file_number = atoi(input_buffer + 2); // file number starts at the second index
            // TODO: tell server which file we want, send number
            // TODO: recv() filesize
            // TODO: loop recv() until received bytes == filesize
        } else if (strncmp(input_buffer, BYE, strlen(BYE)) == 0) {
            printf("GOODBYE COMMAND\n");
            //send(socket_fd, input_buffer, input_size, 0);
        }

        send(socket_fd, input_buffer, input_size, 0); // send user input string to server, pre decrement input_size to remove null terminator
    }

    close(socket_fd);
    freeaddrinfo(addr_ptr);

    return 0;
}

void usage() {
    printf("Usage: ./ftp_client <ip_addr> <port>\n");
}

void print_files_directory() {
    DIR *directory = opendir("files");
    struct dirent *file;

    if (directory == NULL) {
        perror("error on opendir");
        exit(EXIT_FAILURE);
    }

    int i = 1;
    while (file = readdir(directory)) {
        if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0) {
            printf("%d. %s\n", i++, file->d_name);
        }
    }

    closedir(directory);
}

FILE * open_file_by_number(int file_number) {

    DIR *directory = opendir("files");
    struct dirent *file;

    if (directory == NULL) {
        perror("error on opendir");
        exit(EXIT_FAILURE);
    }

    int i = 1;
    while (file = readdir(directory)) {
        if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0) {
            if (i == file_number) {
                FILE *f = fopen(file->d_name, "r");
                closedir(directory);
                return f;
            }
            i++;
        }
    }

    closedir(directory);
    return NULL;
}

size_t get_filesize(FILE *fp) {
    fseek(fp, 0, SEEK_END); // seek to end of file
    size_t size = ftell(fp); // get current file pointer
    fseek(fp, 0, SEEK_SET);
    return size;
}

void upload_file(int socket_fd, int file_number) {
    FILE *fp = open_file_by_number(file_number);
    size_t filesize = get_filesize(fp);
    // TODO: send u to tell server we wanna upload something
    send(socket_fd, "u", 1, 0);
    // TODO: tell server incoming filesize
    // TODO: convert int to string with correct call to itoa
    send(socket_fd, itoa(filesize), sizeof(filesize), 0);
    // TODO: send() bytes until filesize == bytes_sent
    fclose(fp);
}