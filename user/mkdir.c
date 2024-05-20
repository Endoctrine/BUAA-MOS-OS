#include <lib.h>

void usage(void) {
	printf("usage: mkdir <path>\n");
	exit();
}

void failed(char *cwd){
	printf("mkdir: %s: can't mkdir\n", cwd);
	exit();
}

int main(int argc, char **argv) {
	if(argc != 2){
		usage();
	}

    char cwd[MAXPATHLEN];
    getcwd(cwd);
	chcwd(cwd, argv[1]);

	if(mkdir(cwd)){
		failed(cwd);
	}

	return 0;
}
