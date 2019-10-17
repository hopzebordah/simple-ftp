#include <stdio.h>
#include <stdlib.h>

#define START_SIZE 3

int main(void) {

    char *input = (char *)malloc(START_SIZE * sizeof(char));
    size_t size = START_SIZE, index = 0;

    printf("enter a string> ");

    char next_char = 0;
    while (1) { // while char is not newline of EOF
        next_char = getc(stdin); // get next char

        if (next_char == '\n') {
            size++;
            input = (char *)realloc(input, size * sizeof(char));
            input[size] = EOF;
            break;
        }

        if (index == size) {
            size++;
            input = (char *)realloc(input, size * sizeof(char));
        }

        input[index] = next_char;
        index++;
    }

    printf("size of input is: %lu, input is: %s\n", size, input);

    free(input);
    return 0;
}