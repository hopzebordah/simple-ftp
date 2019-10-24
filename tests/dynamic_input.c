#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define START_SIZE 3

size_t get_dynamic_input(char *result, int starting_size);

int main(void) {

    size_t size;
    char *input = (char *)malloc(START_SIZE * sizeof(char));

    while (1) {
        input = (char *)realloc(input, START_SIZE * sizeof(char));;

        memset(input, 0, START_SIZE);

        printf("echo>");

        size = get_dynamic_input(input, START_SIZE);

        printf("size of input is: %lu, input is: %s\n", size, input);
    }

    free(input);

    return 0;
}

// TODO: FIX THIS FUNCTION IT DOES SOME FUNKY STUFF
size_t get_dynamic_input(char *result, int starting_size) {
    size_t size = starting_size, index = 0;

    char next_char;
    while (1) { // while char is not newline of EOF
        next_char = getc(stdin); // get next char

        if (next_char == '\n' || next_char == EOF) {
            size++;
            result = (char *)realloc(result, size * sizeof(char));
            result[size] = '\0';
            break;
        }

        if (index == size) {
            size++;
            result = (char *)realloc(result, size * sizeof(char));
        }

        result[index] = next_char;
        index++;
    }

    return size;
}