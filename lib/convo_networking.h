#ifndef CONVO_NETWORKING_H
#define CONVO_NETWORKING_H

#define BUFFER_SIZE 1000

#define ACK "ACK"

#define CLOSING_CONNECTION "Closing the connection\n"

void reset_buffer(char *buffer);

size_t recv_data(int client_fd, char *recv_buffer);

size_t send_data(int client_fd, char *send_buffer, size_t send_buffer_size);

size_t send_string(int fd, char *send_buffer, char *data);

size_t send_size_value(int fd, char *send_buffer, size_t size);

void send_string_constant(int client_fd, char *send_buffer, char *str_constant);

void recv_string_constant(int fd, char *send_buffer, char *recv_buffer, char *str_constant);

#endif