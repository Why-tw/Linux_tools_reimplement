#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include "include/shell.h"
#include "include/tokenizer.h"
#include "include/parser.h"
#include "include/history.h"

int isbuiltins(char *cmd, char *builtins_list[]) {
	for (int i = 0; i < 16; i ++) {
		if (!strcmp(cmd, builtins_list[i])) {
			return 1;
		}
	}	
	return 0;
}

int main () {
	init_history();
	char *builtins_list[16] = {
		"history"
	};
    while (1) {
        char cwd[1024];
        getcwd(cwd, 1024);
        printf("User%s> ", cwd);
        char input[MAX_INPUT];
        fgets(input, MAX_INPUT, stdin);
        if (!strcmp(input, "\n")) continue;
		add_history(input);
        char tokens[MAX_TOKEN][64];
        int ntoken = tokenize(input, tokens);
        Command commands[MAX_CMD] = {0};
        int ncmd = parse(ntoken, tokens, commands);
        int pipes[MAX_CMD][2];  
        for (int i = 0; i < ncmd-1; i++) pipe(pipes[i]);
        pid_t pids[MAX_CMD];
        for (int i = 0; i < ncmd; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                if (i > 0) dup2(pipes[i-1][0], STDIN_FILENO);
                if (i < ncmd-1) dup2(pipes[i][1], STDOUT_FILENO);

                if (commands[i].input) {
                    int fd = open(commands[i].input, O_RDONLY);
                    dup2(fd, STDIN_FILENO);
					close(fd);
                }
                if (commands[i].output) {
                    int fd = open(commands[i].output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    dup2(fd, STDOUT_FILENO);
					close(fd);
                }
                if (commands[i].append) {
                    int fd = open(commands[i].append, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    dup2(fd, STDOUT_FILENO);
					close(fd);
                }

                for (int j = 0; j < ncmd-1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                execvp(commands[i].argv[0], commands[i].argv);
                exit(1);
            }
            pids[i] = pid;
        }
        for (int i = 0; i < ncmd-1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        for (int i = 0; i < ncmd; i++) waitpid(pids[i], NULL, 0);
    }
    return 0;
}
