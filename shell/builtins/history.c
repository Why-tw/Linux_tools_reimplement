#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "../include/history.h"

void init_history() {
	int fd = open("history", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	close(fd);
}

void add_history(char line[]) {
	int fd = open("history", O_WRONLY | O_CREAT | O_APPEND, 0644);
	write(fd, line, strlen(line));
	write(fd, "\n", 1);
	close(fd);
}

void display_history() {
	int fd = open("history", O_RDONLY);
	char buffer[4096];
	read(fd, buffer, 4096);
	char *tok = strtok(buffer, "\n");
	while (tok) {
		printf("%s", tok);
		tok = strtok(buffer, NULL);
	}
}


