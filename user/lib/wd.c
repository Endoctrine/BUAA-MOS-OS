#include <lib.h>

void go_up(char *path){
    if(strcmp(path, "/") == 0){
        return;
    }
    path[strlen(path) - 1] = 0;
    for(int i = strlen(path) - 1; path[i] != '/'; i--){
        path[i] = 0;
    }
}

void chdir(char *cwd, char *buf){
    int end_flag = 0;
    if(*buf == '/'){
        buf++;
    }
	if(!*buf){
		return;
	}
    char *current_token = buf;
    while(*buf && *buf != '/'){
        buf++;
    }
    if(*buf == 0){
        end_flag = 1;
    }
    *buf = 0;
    buf++;

    if(strcmp(current_token, ".") == 0){
        // do nothing
    } else if(strcmp(current_token, "..") == 0){
        go_up(cwd);
    } else {
        strcat(cwd, current_token);
        strcat(cwd, "/");
    }

    if(end_flag){
        return;
    }
    chdir(cwd, buf);
}

void chcwd(char *cwd, const char *nwd){
    char buf[MAXPATHLEN];
    strcpy(buf, nwd);
    if(buf[0] == '/'){
        cwd[0] = '/';
        cwd[1] = 0;
    }
    chdir(cwd, buf);
}

void getcwd(char *buf){
	int wd_fd = open(".wd", O_RDONLY);
	fgets(wd_fd, buf);
	while(buf[strlen(buf) - 1] == '\r' || buf[strlen(buf) - 1] == '\n'){
		buf[strlen(buf) - 1] = 0;
	}
	close(wd_fd);
}

void putcwd(const char *cwd){
	int wd_fd = open(".wd", O_CREAT);
	close(wd_fd);
	remove(".wd");
	wd_fd = open(".wd", O_CREAT);
	close(wd_fd);

	wd_fd = open(".wd", O_WRONLY);
	if(cwd[0] != '/'){
		fprintf(wd_fd, "/");
	}
	fprintf(wd_fd, "%s", cwd);
	if(cwd[strlen(cwd) - 1] != '/'){
		fprintf(wd_fd, "/");
	}
	close(wd_fd);
}

void printcwd(){
	char wd[MAXPATHLEN];
	getcwd(wd);
	printf("%s", wd);
}
