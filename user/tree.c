#include <lib.h>

void tree(char *path, int depth){
	int fd, n;
    struct File f;

    if ((fd = open(path, O_RDONLY)) < 0) {
        user_panic("open %s: %d", path, fd);
    }
    while ((n = readn(fd, &f, sizeof f)) == sizeof f) {
        if (!f.f_name[0]) {
            continue;
        }
		for(int i = 0; i < depth - 1; i++){
			printf("|   ");
		}
		printf("|-- %s\n", f.f_name);
		strcpy(path + strlen(path), f.f_name);
		strcpy(path + strlen(path), "/");
		if(f.f_type == FTYPE_DIR){
			tree(path, depth + 1);
		}
		path[strlen(path) - strlen(f.f_name) - 1] = 0;
    }
    if (n > 0) {
        user_panic("short read in directory %s", path);
    }
    if (n < 0) {
        user_panic("error reading directory %s: %d", path, n);
    }
}

void usage(void) {
	printf("usage: tree\n");
	exit();
}

int main(int argc, char **argv) {
	
	if(argc != 1){
		usage();
	}

	char cwd[MAXPATHLEN];

	printf(".\n");
	
	getcwd(cwd);

	tree(cwd, 1);
	
	return 0;
}
