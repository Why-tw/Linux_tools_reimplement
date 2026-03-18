#include "include/tokenizer.h"
#include <string.h>

int tokenize(char *input, char tokens[][64]) {
    int count = 0;
    char *tok = strtok(input, " \n");
    while (tok != NULL) {
        strcpy(tokens[count], tok);
        count++;
        tok = strtok(NULL, " \n");
    }
    return count;
}
