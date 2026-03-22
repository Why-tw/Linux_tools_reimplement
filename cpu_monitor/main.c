#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int read_cpus(unsigned long stat[65][8]) {
    int cpufd = open("/proc/stat", O_RDONLY);    
    if (cpufd < 0) { perror("open"); return -1; }

    char buffer[8192];
    ssize_t bytes = read(cpufd, buffer, sizeof(buffer) - 1);    
    if (bytes < 0) { perror("read"); close(cpufd); return -1; }

    buffer[bytes] = '\0';
    close(cpufd);

    char *line = strtok(buffer, "\n");
    int idx = 0;

    while (line) {
        if (strncmp(line, "cpu", 3) == 0) {
            if (idx > 64) break;
            sscanf(line, "%*s %lu %lu %lu %lu %lu %lu %lu %lu",
                   &stat[idx][0], &stat[idx][1], &stat[idx][2], &stat[idx][3],
                   &stat[idx][4], &stat[idx][5], &stat[idx][6], &stat[idx][7]);
            idx ++;
        } else {
            break; // cpu section 結束
        }
        line = strtok(NULL, "\n");  // 抓下一行
    }
    return idx;
}

int main () {
	while (1) {
		printf("\033[H\033[J"); 
		unsigned long cpus_usage1[65][8]; // 第0位是總使用量
		unsigned long cpus_usage2[65][8]; // 最多允許64個CPU
		int ncpu;
		ncpu = read_cpus(cpus_usage1);
		sleep(1);
		read_cpus(cpus_usage2);
		printf("number of cpu: %d\n", ncpu);
		double cpus_list[65];
		for (int i = 0; i < ncpu; i ++) {
			unsigned long total1 = 0;
			unsigned long total2 = 0;
			for (int j = 0; j < 8; j ++) {
				total1 += cpus_usage1[i][j];
				total2 += cpus_usage2[i][j];
			}
			double total_diff = (double)total2-total1;
			double idle_diff = (double)((cpus_usage2[i][3]+cpus_usage2[i][4])-(cpus_usage1[i][3]+cpus_usage1[i][4]));
			cpus_list[i] = (1-idle_diff/total_diff) * 100.0;
		}
		printf("Total CPU Usage: %.2f%%\n", cpus_list[0]);
		for (int i = 0; i < ncpu; i ++) printf("CPU%d Usage: %.2f%%\n", i, cpus_list[i+1]);
	}
	return 0;
}
