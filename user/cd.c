#include <lib.h>

void usage(void) {
	printf("usage: cd <path>\n");
	exit();
}

void path_not_exist(char *cwd) {
	printf("cd: %s: path not exist\n", cwd);
	exit();
}

void not_directory(char *cwd) {
	printf("cd: %s: not directory\n", cwd);
	exit();
}

int main(int argc, char **argv) {
	
	if(argc != 2){
		usage();
	}

	char cwd[MAXPATHLEN];
	getcwd(cwd);
	chcwd(cwd, argv[1]);

	struct Stat st;	
	if(stat(cwd, &st) < 0){
		path_not_exist(cwd);
	}
	if(!st.st_isdir){
		not_directory(cwd);
	}

	putcwd(cwd);

	return 0;
}
