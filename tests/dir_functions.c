#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

void print_files_directory();
void get_filename_by_number(int, char *);

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

    char selected_file_name[input_max_size];
    memset(selected_file_name, 0, input_max_size); // reset input buffer to 0s before receiving input
    get_filename_by_number(selected_file_number, selected_file_name);

    printf("selected filename: %s\n", selected_file_name);
    
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
void get_filename_by_number(int file_number, char *str) {

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
                strcpy(str, file->d_name);
                break;
            }
            i++;
        }
    }

    closedir(directory);
}