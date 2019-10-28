//********************************************************************
//
// Alexander Peters
// Computer Networks
// Programming Project #3: Simple FTP Server
// October 27, 2019
// Instructor: Dr. Ajay K. Katangur
//
//********************************************************************

#ifndef CONVO_NETWORKING_H
#define CONVO_NETWORKING_H

#define DEBUG 0

#define BUFFER_SIZE 1000

#define ACK "ACK"

#define CLOSING_CONNECTION "Closing the connection\n"

void reset_buffer(char *buffer);

size_t recv_data(int fd, char *recv_buffer);

size_t recv_string(int fd, char *recv_buffer, char *str);

size_t recv_size_value(int fd, char *recv_buffer);

int recv_int(int fd, char *recv_buffer);

size_t send_data(int fd, char *send_buffer, size_t send_buffer_size);

size_t send_string(int fd, char *send_buffer, char *data);

size_t send_size_value(int fd, char *send_buffer, size_t size);

size_t send_int(int fd, char *send_buffer, int value);

void send_string_constant(int client_fd, char *send_buffer, char *str_constant);

void recv_string_constant(int fd, char *send_buffer, char *recv_buffer, char *str_constant);

#endif