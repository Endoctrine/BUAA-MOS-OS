#include <lib.h>

void usage(void) {
	printf("usage: touch <file>\n");
	exit();
}

void failed(char *cwd){
	printf("touch: %s: can't touch\n");
	exit();
}

int main(int argc, char **argv) {
	int i;

	if(argc != 2){
		usage();
	}

	char cwd[MAXPATHLEN];
	getcwd(cwd);
	chcwd(cwd, argv[1]);

	int fd = open(cwd, O_CREAT);
	if(fd < 0){
		failed(cwd);
	}
	close(fd);

	return 0;
}
