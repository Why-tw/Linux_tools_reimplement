#include "include/parser.h"
#include <string.h>

int parse(int n, char tokens[][64], Command commands[MAX_CMD]) {
    int idx = 0;
    int argi = 0;

    for (int i = 0; i < n; i++) {
        if (!strcmp(tokens[i], ">")) {
            commands[idx].output = tokens[i+1];
            i++;
            continue;
        }
        if (!strcmp(tokens[i], "<")) {
            commands[idx].input = tokens[i+1];
            i++;
            continue;
        }
        if (!strcmp(tokens[i], ">>")) {
            commands[idx].append = tokens[i+1];
            i++;
            continue;
        }
        if (!strcmp(tokens[i], "|")) {
            commands[idx].argv[argi] = NULL;
            idx++;
            argi = 0;
            continue;
        }
        commands[idx].argv[argi++] = tokens[i];
    }

    commands[idx].argv[argi] = NULL;
    return idx + 1;
}
