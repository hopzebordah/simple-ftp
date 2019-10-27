#include <stdio.h>
#include <string.h>

#include "util.h"

int strings_match(char *string1, char *string2) {
    if (strncmp(string1, string2, strlen(string2)) == 0)
        return 1;
    return 0;
}