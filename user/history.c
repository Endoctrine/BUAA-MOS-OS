#include <lib.h>

void usage(void) {
	printf("usage: history\n");
	exit();
}

#define LINE_SIZE 1024

int main(int argc, char **argv) {

	if(argc != 1) {
		usage();
	}

	int fd = open(".history", O_RDONLY);
	char current_line[LINE_SIZE];

	while(fgets(fd, current_line) != -1){
		printf("%s", current_line);
	}

	return 0;
}
