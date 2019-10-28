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
#include <dirent.h>

#include "file_functions.h"


//********************************************************************
//
// Directory Print Function
//
// This function loops over the contents of a directory. 
// It prints all files contained in that directory. 
// 
// Local variables
// ---------------
// directory    DIR *           directory pointer
// file         struct dirent   dirent structure 
//
//********************************************************************
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


//********************************************************************
//
// Filename Clear Function
//
// This function zeroes out a filename array to size FILENAME_SIZE_MAX. 
//
// Value parameters
// ---------------
// filename      char *      filename pointer to clear
// 
//********************************************************************
void clear_filename_array(char *filename) {
    memset(filename, 0, FILENAME_SIZE_MAX);
}


//********************************************************************
//
// File Number Function
//
// This function returns the number of files in a local "files" directory.
//
// Return value
// ---------------
// An int indicating the number of files in the directory.
//
// Local variables
// ---------------
// num_files    int             file number integer
// directory    DIR *           directory pointer
// file         struct dirent   dirent structure 
//
//********************************************************************
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

//********************************************************************
//
// Filename by Number Function
//
// This function fills a string with the filename of a file indicated by a number.
//
// Return value
// ---------------
// The length of the filename. 
// 
// Value parameters
// ---------------
// str              char *      pointer to filename string
// file_number      char *      file number to get filename from
// 
// Reference parameters
// ---------------
//
// Local variables
// ---------------
// 
//********************************************************************
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

//********************************************************************
//
// Open File Function
//
// This function wraps the fopen function in an error handling wrapper.
//
// Return value
// ---------------
// A pointer to the file that was opened. 
// 
// Value parameters
// ---------------
// filename         char *      pointer to filename string
// mode             char *      pointer to file opening mode
// 
// Local variables
// ---------------
// fp       FILE *      file pointer to be returned
//
//********************************************************************
FILE * open_file(char *filename, char *mode) {
    FILE *fp = fopen(filename, mode);
    if (fp == NULL) {
        perror("open file by number");
        exit(EXIT_FAILURE);
    }
    return fp;
}

//********************************************************************
//
// Filename by Number Function
//
// This function fills a string with the filename of a file indicated by a number.
//
// Return value
// ---------------
// The length of the filename. 
// 
// Value parameters
// ---------------
// str              char *      pointer to filename string
// file_number      char *      file number to get filename from
// 
// Local variables
// ---------------
// temp_filename    char *      temp filename array to prepend directory name
// filename_size    size_t      size of filename 
// fp               FILE *      file pointer 
//
//********************************************************************
FILE * open_file_by_number(char *filename, char *mode, int number) {
    clear_filename_array(filename);
    char temp_filename[FILENAME_SIZE_MAX];
    clear_filename_array(temp_filename);
    size_t filename_size = get_filename_by_number(temp_filename, number);
    sprintf(filename, "%s%s", FILES_DIRECTORY, temp_filename);
    FILE *fp = open_file(filename, mode);
    return fp;
}

//********************************************************************
//
// Filesize Function
//
// This function get the filesize of a file in bytes
//
// Return value
// ---------------
// The size of the file
// 
// Value parameters
// ---------------
// fp               FILE *      pointer to file
// 
// Local variables
// ---------------
// size    size_t   size of file
//
//********************************************************************
size_t get_filesize(FILE *fp) {
    fseek(fp, 0, SEEK_END); // seek to end of file
    size_t size = ftell(fp); // get current file pointer
    fseek(fp, 0, SEEK_SET);
    return size;
}