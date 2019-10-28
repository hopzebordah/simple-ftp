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
#include <string.h>

#include "util.h"

//********************************************************************
//
// String Comparison Function
//
// This function compares two strings.
// If the strings match, the function returns 1, 0 otherwise.
//
// Return value
// ---------------
// 1 or 0 if strings match / do not match. 
// 
// Value parameters
// ---------------
// string1      char *      first string to compare
// string2      char *      second string to compare
// 
// Reference parameters
// ---------------
//
// Local variables
// ---------------
// 
//********************************************************************
int strings_match(char *string1, char *string2) {
    if (strncmp(string1, string2, strlen(string2)) == 0)
        return 1;
    return 0;
}