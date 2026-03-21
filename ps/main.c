#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

typedef struct {
    char name[256];
    int pid;
    int ppid;
    char state[32];
} proc_info;

void init_proc(proc_info *p) {
    p->name[0] = '\0';
    p->pid = -1;
    p->ppid = -1;
    p->state[0] = '\0';
}

int only_number(const char *s) {
    for (int i = 0; s[i]; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

void parse_status(const char *filepath, proc_info *proc) {
    init_proc(proc);

    char buffer[4096];
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) return;

    ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (n <= 0) return;
    buffer[n] = '\0';

    char *tok = strtok(buffer, "\n");
    char key[64];
    char value[256];

    while (tok != NULL) {
        if (sscanf(tok, " %63[^:]: %255[^\n]", key, value) == 2) {
            if (proc->name[0] == '\0' && strcmp(key, "Name") == 0) {
                strcpy(proc->name, value);
            }
            if (proc->pid == -1 && strcmp(key, "Pid") == 0) {
                proc->pid = atoi(value);
            }
            if (proc->ppid == -1 && strcmp(key, "PPid") == 0) {
                proc->ppid = atoi(value);
            }
            if (proc->state[0] == '\0' && strcmp(key, "State") == 0) {
                strcpy(proc->state, value);
            }
        }
        tok = strtok(NULL, "\n");
    }
}

int main(int argc, char **argv) {
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    proc_info proc_list[1000];
    int idx = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (only_number(entry->d_name)) {
            char filepath[256];
            snprintf(filepath, sizeof(filepath), "/proc/%s/status", entry->d_name);

            proc_info cur_proc;
            parse_status(filepath, &cur_proc);

            if (cur_proc.pid != -1 && idx < 1000) {
                proc_list[idx] = cur_proc;
                idx++;
            }
        }
    }

    closedir(dir);
	printf("%-8s %-8s %-12s %-20s\n", "PID", "PPID", "STATE", "NAME");
	for (int i = 0; i < idx; i++) {
		printf("%-8d %-8d %-12s %-20s\n",
			   proc_list[i].pid,
			   proc_list[i].ppid,
			   proc_list[i].state,
			   proc_list[i].name);
	}
    return 0;
}
