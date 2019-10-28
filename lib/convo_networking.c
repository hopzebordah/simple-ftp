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
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "convo_networking.h"

//********************************************************************
//
// Reset Buffer Function
//
// This function fills a string with the filename of a file indicated by a number.
//
// Return value
// ---------------
// The length of the filename. 
// 
// Value parameters
// ---------------
// buffer           char *      pointer to buffer
//
//********************************************************************
void reset_buffer(char *buffer) {
    bzero(buffer, BUFFER_SIZE);
}

//********************************************************************
//
// Receive Data Function
//
// This function wraps the recv() function for ease of use
//
// Return value
// ---------------
// Number of bytes received. 
// 
// Value parameters
// ---------------
// fd               int         socket file descriptor
// recv_buffer      char *      pointer to buffer to use
// 
// Local variables
// ---------------
// bytes_received    size_t      number of bytes received
//
//********************************************************************
size_t recv_data(int fd, char *recv_buffer) {
    reset_buffer(recv_buffer);
    size_t bytes_received = recv(fd, recv_buffer, BUFFER_SIZE, 0);
    return bytes_received;
}

//********************************************************************
//
// Receive String Function
//
// This function wraps the recv_data function for ease of use with strings
//
// Return value
// ---------------
// Number of bytes received. 
// 
// Value parameters
// ---------------
// fd               int         socket file descriptor
// recv_buffer      char *      pointer to buffer to use
// str              char *      string to fill with received data
// 
// Local variables
// ---------------
// bytes_received    size_t     number of bytes received
// bytes_saved       size_t     length of saved string
//
//********************************************************************
size_t recv_string(int fd, char *recv_buffer, char *str) {
    reset_buffer(recv_buffer);
    size_t bytes_received = recv_data(fd, recv_buffer);
    size_t bytes_saved = sprintf(str, "%s", recv_buffer);
    return bytes_saved;
}

//********************************************************************
//
// Receive Size Function
//
// This function wraps the recv_data function for ease of use with size_t values
//
// Return value
// ---------------
// Size received. 
// 
// Value parameters
// ---------------
// fd               int         socket file descriptor
// recv_buffer      char *      pointer to buffer to use
// 
// Local variables
// ---------------
// bytes_received       size_t      number of bytes received
// incoming_size_value  size_t      size value to return
// bytes_saved          size_t      number of bytes saved from sscanf 
//
//********************************************************************
size_t recv_size_value(int fd, char *recv_buffer) {
    reset_buffer(recv_buffer);
    size_t bytes_received = recv_data(fd, recv_buffer);

    size_t incoming_size_value;
    size_t bytes_saved =  sscanf(recv_buffer, "%zu", &incoming_size_value);

    return incoming_size_value;
}

//********************************************************************
//
// Receive int Function
//
// This function wraps the recv_data function for ease of use with int values
//
// Return value
// ---------------
// int received. 
// 
// Value parameters
// ---------------
// fd               int         socket file descriptor
// recv_buffer      char *      pointer to buffer to use
// 
// Local variables
// ---------------
// bytes_received       size_t      number of bytes received
// incoming_int_value   int         size value to return
// bytes_saved          size_t      number of bytes saved from sscanf 
//
//********************************************************************
int recv_int(int fd, char *recv_buffer) {
    reset_buffer(recv_buffer);
    size_t bytes_received = recv_data(fd, recv_buffer);

    int incoming_int_value;
    size_t bytes_saved = sscanf(recv_buffer, "%d", &incoming_int_value);

    return incoming_int_value;
}

//********************************************************************
//
// Send Data Function
//
// This function wraps the send() function for ease of use
//
// Return value
// ---------------
// Bytes sent. 
// 
// Value parameters
// ---------------
// fd               int         socket file descriptor
// send_buffer      char *      pointer to buffer to use
// send_buffer_size size_t      size of buffer to use
// 
// Local variables
// ---------------
// bytes_sent           size_t      number of bytes sent 
//
//********************************************************************
size_t send_data(int fd, char *send_buffer, size_t send_buffer_size) {
    size_t bytes_sent = send(fd, send_buffer, send_buffer_size, 0);
    reset_buffer(send_buffer);
    return bytes_sent;
}

//********************************************************************
//
// Send String Function
//
// This function wraps the send() function for ease of use with strings
//
// Return value
// ---------------
// Bytes sent. 
// 
// Value parameters
// ---------------
// fd               int         socket file descriptor
// send_buffer      char *      pointer to buffer to use
// data             char *      pointer to string that needs to be sent
// 
// Local variables
// ---------------
// bytes_stored     size_t      length of string sent 
//
//********************************************************************
size_t send_string(int fd, char *send_buffer, char *data) {
    reset_buffer(send_buffer);
    size_t bytes_stored = sprintf(send_buffer, "%s", data);
    return send_data(fd, send_buffer, bytes_stored);
}

//********************************************************************
//
// Send Size Function
//
// This function wraps the send() function for ease of use with size_t values
//
// Return value
// ---------------
// Bytes sent. 
// 
// Value parameters
// ---------------
// fd               int         socket file descriptor
// send_buffer      char *      pointer to buffer to use
// size             size_t      size value to send
// 
// Local variables
// ---------------
// bytes_stored     size_t      number of bytes stored with sprintf
//
//********************************************************************
size_t send_size_value(int fd, char *send_buffer, size_t size) {
    reset_buffer(send_buffer);
    size_t bytes_stored = sprintf(send_buffer, "%zu", size);
    return send_data(fd, send_buffer, bytes_stored);
}

//********************************************************************
//
// Send int Function
//
// This function wraps the send() function for ease of use with int values
//
// Return value
// ---------------
// Bytes sent. 
// 
// Value parameters
// ---------------
// fd               int         socket file descriptor
// send_buffer      char *      pointer to buffer to use
// value             int        int value to send
// 
// Local variables
// ---------------
// bytes_stored     size_t      number of bytes stored with sprintf
//
//********************************************************************
size_t send_int(int fd, char *send_buffer, int value) {
    reset_buffer(send_buffer);
    size_t bytes_stored = sprintf(send_buffer, "%d", value);
    return send_data(fd, send_buffer, bytes_stored);
}

//********************************************************************
//
// Send String Constant Function
//
// This function wraps the send() function for ease of use with constant
// This function is essentially identical to the send_string function, 
// but it is void and has debug functionality
// 
// Value parameters
// ---------------
// fd               int         socket file descriptor
// send_buffer      char *      pointer to buffer to use
// data             char *      pointer to string that needs to be sent
// 
// Local variables
// ---------------
// bytes_stored     size_t      length of string to be sent 
// bytes_sent       size_t      bytes sent
//
//********************************************************************
void send_string_constant(int client_fd, char *send_buffer, char *str_constant) {
    if (DEBUG) printf("SENDING %s...\n", str_constant);
    reset_buffer(send_buffer);
    size_t bytes_stored = sprintf(send_buffer, "%s", str_constant);
    size_t bytes_sent = send_data(client_fd, send_buffer, bytes_stored);
}

//********************************************************************
//
// Receive String Constant Function
//
// This function wraps the recv_data function for ease of use with constants
// This function expects a string and fails if the string received is not what it expects. 
// Has debug functionality. 
// 
// Value parameters
// ---------------
// fd               int         socket file descriptor
// send_buffer      char *      pointer to send buffer to use
// recv_buffer      char *      pointer to recv buffer to use
// data             char *      pointer to string that needs to be sent
// 
// Local variables
// ---------------
// bytes_received   size_t      bytes received with recv()
//
//********************************************************************
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

