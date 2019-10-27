#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "file_functions.h"

void print_files_directory() {
    DIR *directory = opendir("files");
    struct dirent *file;

    if (directory == NULL) {
        perror("error on opendir");
        exit(EXIT_FAILURE);
    }

    int i = 1;
    while ((file = readdir(directory))) {
        if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0) {
            printf("%d. %s\n", i++, file->d_name);
        }
    }

    closedir(directory);
}

size_t get_filename_by_number(char *str, int file_number) {
    DIR *directory = opendir("files");
    struct dirent *file;

    if (directory == NULL) {
        perror("error on opendir");
        exit(EXIT_FAILURE);
    }

    int i = 1;
    while ((file = readdir(directory))) {
        if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0) {
            if (i == file_number) {
                size_t chars_written = sprintf(str, "%s%s", "files/", file->d_name);
                closedir(directory);
                return chars_written;
            }
            i++;
        }
    }

    closedir(directory);
    return 0;
}

FILE * open_file(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("open file by number");
        exit(EXIT_FAILURE);
    }
    return fp;
}

size_t get_filesize(FILE *fp) {
    fseek(fp, 0, SEEK_END); // seek to end of file
    size_t size = ftell(fp); // get current file pointer
    fseek(fp, 0, SEEK_SET);
    return size;
}

size_t write_byte_array_to_file(char *filename, char *data, size_t data_size) {
    return 0;
}