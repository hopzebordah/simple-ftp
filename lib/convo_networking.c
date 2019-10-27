#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "convo_networking.h"

void reset_buffer(char *buffer) {
    bzero(buffer, BUFFER_SIZE);
}

size_t recv_data(int fd, char *recv_buffer) {
    reset_buffer(recv_buffer);
    size_t bytes_received = recv(fd, recv_buffer, BUFFER_SIZE, 0);
    return bytes_received;
}

size_t recv_string(int fd, char *recv_buffer, char *str) {
    reset_buffer(recv_buffer);
    size_t bytes_received = recv_data(fd, recv_buffer);
    size_t bytes_saved = sprintf(str, "%s", recv_buffer);
    return bytes_saved;
}

size_t recv_size_value(int fd, char *recv_buffer) {
    reset_buffer(recv_buffer);
    size_t bytes_received = recv_data(fd, recv_buffer);

    size_t incoming_size_value;
    size_t bytes_saved =  sscanf(recv_buffer, "%zu", &incoming_size_value);

    return incoming_size_value;
}

int recv_int(int fd, char *recv_buffer) {
    reset_buffer(recv_buffer);
    size_t bytes_received = recv_data(fd, recv_buffer);

    int incoming_int_value;
    size_t bytes_saved = sscanf(recv_buffer, "%d", &incoming_int_value);

    return incoming_int_value;
}

size_t send_data(int fd, char *send_buffer, size_t send_buffer_size) {
    size_t bytes_sent = send(fd, send_buffer, send_buffer_size, 0);
    reset_buffer(send_buffer);
    return bytes_sent;
}

size_t send_string(int fd, char *send_buffer, char *data) {
    reset_buffer(send_buffer);
    size_t bytes_stored = sprintf(send_buffer, "%s", data);
    return send_data(fd, send_buffer, bytes_stored);
}

size_t send_size_value(int fd, char *send_buffer, size_t size) {
    reset_buffer(send_buffer);
    size_t bytes_stored = sprintf(send_buffer, "%zu", size);
    return send_data(fd, send_buffer, bytes_stored);
}

size_t send_int(int fd, char *send_buffer, int value) {
    reset_buffer(send_buffer);
    size_t bytes_stored = sprintf(send_buffer, "%d", value);
    return send_data(fd, send_buffer, bytes_stored);
}

void send_string_constant(int client_fd, char *send_buffer, char *str_constant) {
    if (DEBUG) printf("SENDING %s...\n", str_constant);
    reset_buffer(send_buffer);
    size_t bytes_stored = sprintf(send_buffer, "%s", str_constant);
    size_t bytes_sent = send_data(client_fd, send_buffer, bytes_stored);
}

void recv_string_constant(int fd, char *send_buffer, char *recv_buffer, char *str_constant) {
    if (DEBUG) printf("WAITING FOR %s...\n", str_constant);
    reset_buffer(recv_buffer);
    size_t bytes_received = recv_data(fd, recv_buffer);
    if (strncmp(recv_buffer, str_constant, strlen(str_constant)) == 0) {
        reset_buffer(recv_buffer);
        if (DEBUG) printf("GOT IT!\n");
    } else {
        printf("[!] ERROR: expected %s and received %s...\nTerminating...\n", str_constant, recv_buffer);
        send_string_constant(fd, send_buffer, CLOSING_CONNECTION);
        exit(EXIT_FAILURE);
    }
}

