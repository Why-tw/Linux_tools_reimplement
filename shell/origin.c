#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>

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

int parse(int n, char tokens[][64], Command commands[MAX_CMD]) {
	int idx = 0;
	int argi = 0;
	for (int i = 0; i < n; i ++) {
		if (!strcmp(tokens[i], ">")) {
			commands[idx].output = tokens[i+1];	
			i ++;
			continue;
		}
		if (!strcmp(tokens[i], "<")) {
			commands[idx].input = tokens[i+1];
			i ++;
			continue;
		}
		if (!strcmp(tokens[i], ">>")) {
			commands[idx].append = tokens[i+1];
			i ++;
			continue;
		}
		if (!strcmp(tokens[i], "|")) {
			commands[idx].argv[argi] = NULL;
			idx ++;
			argi = 0;
			continue;
		}
		commands[idx].argv[argi++] = tokens[i];
	}
	commands[idx].argv[argi] = NULL;
	return idx + 1;
}

int main () {
	while (1) {
		char cwd[1024];
		getcwd(cwd, 1024);
		printf("User%s> ", cwd);
		char input[MAX_INPUT];
		fgets(input, MAX_INPUT, stdin);
		if (!strcmp(input, "\n")) continue;
		char tokens[MAX_TOKEN][64];
		int ntoken = tokenize(input, tokens);
		// for (int i = 0; i < ntoken; i ++) printf("%s ", tokens[i]);
		Command commands[MAX_CMD];
		for (int i = 0; i < MAX_CMD; i ++) {
			commands[i].input = NULL;
			commands[i].output = NULL;
			commands[i].append = NULL;
			for (int j = 0; j < MAX_ARG; j ++) commands[i].argv[j] = NULL;			
		}

		int ncmd = parse(ntoken, tokens, commands);
		int pipes[MAX_CMD][2];	
		for (int i = 0; i < ncmd-1; i ++) pipe(pipes[i]);
		pid_t pids[MAX_CMD];
		for (int i = 0; i < ncmd; i ++) {
			pid_t pid = fork();
			if (pid == 0) {	
				if (i > 0) {
					dup2(pipes[i-1][0], STDIN_FILENO);
				}		
				if (i < ncmd-1) {
					dup2(pipes[i][1], STDOUT_FILENO);
				}
				if (commands[i].input != NULL) {
					int fd = open(commands[i].input, O_RDONLY);
					dup2(fd, STDIN_FILENO);
				}
				if (commands[i].output != NULL) {
					int fd = open(commands[i].output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
					dup2(fd, STDOUT_FILENO);
				}
				if (commands[i].append != NULL) {
				int fd = open(commands[i].append,O_WRONLY | O_APPEND);
					dup2(fd, STDOUT_FILENO);
				}
				for (int j = 0; j < ncmd-1; j ++) {
					close(pipes[j][0]);
					close(pipes[j][1]);
				}
				execvp(commands[i].argv[0], commands[i].argv);
				exit(1);
			}
			pids[i] = pid;
		}
		for (int i = 0; i < ncmd-1; i ++) {
			close(pipes[i][0]);
			close(pipes[i][1]);
		}	
		for (int i = 0; i < ncmd; i ++) waitpid(pids[i], NULL, 0);
	}
	return 0;
}
