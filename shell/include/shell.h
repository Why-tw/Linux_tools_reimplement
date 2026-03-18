#ifndef SHELL_H
#define SHELL_H

#define MAX_INPUT 1024
#define MAX_TOKEN 4096
#define MAX_CMD 64
#define MAX_ARG 64

typedef struct {
    char *argv[MAX_ARG];

    char *input;
    char *output;
    char *append;
} Command;

#endif
