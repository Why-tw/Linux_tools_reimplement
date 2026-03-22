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
	int vmrss;
} proc_info;

void init_proc(proc_info *p) {
    p->name[0] = '\0';
    p->pid = -1;
    p->ppid = -1;
    p->state[0] = '\0';
	p->vmrss = -1;
}

int only_number(const char *s) {
    if (s[0] == '\0') return 0;
    for (int i = 0; s[i]; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

void print_usage(const char *prog) {
    printf("Usage: %s [-p PID] [-n NAME] [-l LIMIT] [--sort=pid|ppid]\n", prog);
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
			if (proc->vmrss == -1 && strcmp(key, "VmRSS") == 0) {
				proc->vmrss = atoi(value);
			}
        }
        tok = strtok(NULL, "\n");
    }
}

int pid_cmp(const void *a, const void *b) {
	const proc_info *pa = a;
	const proc_info *pb = b;
	if (pa->pid < pb->pid) return -1;
	if (pa->pid > pb->pid) return 1;
	return 0;
}

int ppid_cmp(const void *a, const void *b) {
	const proc_info *pa = a;
	const proc_info *pb = b;
	if (pa->ppid < pb->ppid) return -1;
	if (pa->ppid > pb->ppid) return 1;
	return 0;
}

int vmrss_cmp(const void *a, const void *b) {
	const proc_info *pa = a;
	const proc_info *pb = b;
	if (pa->vmrss < pb->vmrss) return -1;
	if (pa->vmrss > pb->vmrss) return 1;
	return 0;
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
	
	int noutput = idx;
	int target_pid = -1;
	char *target_name = NULL;

	qsort(proc_list, idx, sizeof(proc_info), pid_cmp);

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-l")) {
			if (i + 1 >= argc) {
				print_usage(argv[0]);
				return 1;
			}
			i++;
			noutput = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-p")) {
			if (i + 1 >= argc) {
				print_usage(argv[0]);
				return 1;
			}
			i++;
			target_pid = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-n")) {
			if (i + 1 >= argc) {
				print_usage(argv[0]);
				return 1;
			}
			i++;
			target_name = argv[i];
		}
		else if (!strcmp(argv[i], "--sort=pid")) {
			qsort(proc_list, idx, sizeof(proc_info), pid_cmp);
		}
		else if (!strcmp(argv[i], "--sort=ppid")) {
			qsort(proc_list, idx, sizeof(proc_info), ppid_cmp);
		}
		else if (!strcmp(argv[i], "--sort=vmrss")) {
			qsort(proc_list, idx, sizeof(proc_info), vmrss_cmp);
		}
		else {
			print_usage(argv[0]);
			return 1;
		}
	}
	if (noutput > idx) noutput = idx;
	if (noutput < 0) noutput = 0;
	printf("%-8s %-8s %-12s %-10s %-20s\n",
		   "PID", "PPID", "STATE", "RSS(KB)", "NAME");

	int printed = 0;
	for (int i = 0; i < idx; i++) {
		if (target_pid != -1 && proc_list[i].pid != target_pid) continue;
		if (target_name != NULL && strcmp(proc_list[i].name, target_name) != 0) continue;

		if (proc_list[i].vmrss == -1) {
			printf("%-8d %-8d %-12c %-10s %-20s\n",
				   proc_list[i].pid,
				   proc_list[i].ppid,
				   proc_list[i].state[0],
				   "-",
				   proc_list[i].name);
		} else {
			printf("%-8d %-8d %-12c %-10d %-20s\n",
				   proc_list[i].pid,
				   proc_list[i].ppid,
				   proc_list[i].state[0],
				   proc_list[i].vmrss,
				   proc_list[i].name);
		}

		printed++;
		if (printed >= noutput) break;
	}
    return 0;
}
