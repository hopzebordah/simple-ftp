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

void clear_filename_array(char *filename) {
    memset(filename, 0, FILENAME_SIZE_MAX);
}

size_t get_number_of_files_in_directory() {
    DIR *directory = opendir("files");
    struct dirent *file;

    if (directory == NULL) {
        perror("error on opendir");
        exit(EXIT_FAILURE);
    }

    size_t num_files = 0;
    while ((file = readdir(directory))) {
        if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0) {
            num_files++;
        }
    }

    closedir(directory);

    return num_files;
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
                size_t chars_written = sprintf(str, "%s", file->d_name);
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

FILE * open_file_by_number(char *filename, int number) {
    clear_filename_array(filename);
    char temp_filename[FILENAME_SIZE_MAX + 10];
    clear_filename_array(temp_filename);
    size_t filename_size = get_filename_by_number(temp_filename, number);
    sprintf(filename, "%s%s", FILES_DIRECTORY, temp_filename);
    printf("%s\n", filename);
    FILE *fp = open_file(filename);
    return fp;
}

size_t get_filesize(FILE *fp) {
    fseek(fp, 0, SEEK_END); // seek to end of file
    size_t size = ftell(fp); // get current file pointer
    fseek(fp, 0, SEEK_SET);
    return size;
}