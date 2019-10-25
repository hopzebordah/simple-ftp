#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

void print_files_directory();
FILE * open_file_by_number(int);

int main(void) {

    print_files_directory();
    
    size_t input_max_size = 250; // need access to pointer for getline() call

    char input_buffer[input_max_size];
    char *input_buffer_ptr = input_buffer;

    printf("enter a number>");

    memset(input_buffer, 0, input_max_size); // reset input buffer to 0s before receiving input
    size_t input_size = getline(&input_buffer_ptr, &input_max_size, stdin);
    input_size--; // strip newline, no null terminator

    int selected_file_number = atoi(input_buffer);

    FILE *file = open_file_by_number(selected_file_number);
    
    return 0;
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

// TODO: account for file_number being too big n shit
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