#ifndef FILE_FUNCTIONS_H
#define FILE_FUNCTIONS_H

#define FILES_DIRECTORY "files/"
#define FILENAME_SIZE_MAX 250

void print_files_directory();

void clear_filename_array(char *filename);

size_t get_number_of_files_in_directory();

size_t get_filename_by_number(char *str, int file_number);

FILE * open_file(char *filename);

FILE * open_file_by_number(char *filename, int number);

size_t get_filesize(FILE *fp);

#endif