#ifndef FILE_FUNCTIONS_H
#define FILE_FUNCTIONS_H

void print_files_directory();

size_t get_filename_by_number(char *str, int file_number);

FILE * open_file(char *filename);

size_t get_filesize(FILE *fp);

size_t write_byte_array_to_file(char *filename, char *data, size_t data_size);

#endif