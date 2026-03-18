#include <fcntl.h>
#include <stdio.h>

int main() {
	open("/proc/1/status", O_RDONLY);
}
