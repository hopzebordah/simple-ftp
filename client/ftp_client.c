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
#define BYE "Bye"
#define CLOSING_CONNECTION "Closing the connection\n"
#define LIST_CLIENT_FILES "ls client"
#define LIST_SERVER_FILES "ls server"
#define UPLOAD_FILE "u"
#define DOWNLOAD_FILE "d"

// CREATE A THREAD POOL INSTEAD OF CREATING AND KILLING THREADS. 
// ACQUIRE, NOTIFY, CLEAN THREADS?

void usage();

size_t get_dynamic_input(char *result, int starting_size);
void get_files_in_directory_v();
FILE *get_file_in_directory_by_number(int number);

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

    size_t input_size;
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
        input_size = getline(&input_buffer_ptr, &input_max_size, stdin);
        input_size--; // strip newline, no null terminator

        if (strncmp(input_buffer, LIST_CLIENT_FILES, strlen(LIST_CLIENT_FILES)) == 0) {
            printf("YOU WANT TO LIST FILES ON CLIENT!!!\n");
            get_files_in_directory_v();
        } else if (strncmp(input_buffer, LIST_SERVER_FILES, strlen(LIST_SERVER_FILES)) == 0) {
            printf("YOU WANT TO LIST FILES ON SERVER!!!\n");
        } else if (strncmp(input_buffer, UPLOAD_FILE, strlen(UPLOAD_FILE)) == 0) {
            printf("YOU WANT TO UPLOAD A FILE!!!\n");
            char *file_number_string = input_buffer + 2;
            int file_number = atoi(file_number_string);
            printf("%d\n", file_number);
            FILE *fp = get_file_in_directory_by_number(file_number);
            // TODO: send file in while loop 
            fclose(fp);
        } else if (strncmp(input_buffer, DOWNLOAD_FILE, strlen(DOWNLOAD_FILE)) == 0) {
            printf("YOU WANT TO DOWNLOAD A FILE!!!\n");
            char *file_number_string = input_buffer + 2;
            int file_number = atoi(file_number_string);
            printf("%d\n", file_number);
        } else if (strncmp(input_buffer, BYE, strlen(BYE)) != 0) {
            printf("GOODBYE COMMAND\n");
            //send(socket_fd, input_buffer, --input_size, 0);
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
    DIR *dir = opendir("./files");
    if (dir == NULL) {
        perror("[-] could not open directory for reading");
        exit(EXIT_FAILURE);
    }
    int i = 1;
    while ((de = readdir(dir)) != NULL) {
        printf("%d. %s\n", i++, de->d_name);
    }
    closedir(dir);
}

FILE *get_file_in_directory_by_number(int number) {
    struct dirent *de;
    DIR *dir = opendir("./files");
    if (dir == NULL) {
        perror("[-] could not open directory for reading");
        exit(EXIT_FAILURE);
    }
    int i = 1;
    while ((de = readdir(dir)) != NULL) {
        if (i == number) {
            FILE *fp;
            fp = fopen(de->d_name, "r");
            return fp;
        }
    }
    closedir(dir);
    return NULL;
}
