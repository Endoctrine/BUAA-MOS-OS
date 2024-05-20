#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

void usage(void) {
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
int _gettoken(char *s, char **p1, char **p2) {
	*p1 = 0;
	*p2 = 0;
	if (s == 0) {
		return 0;
	}

	while (strchr(WHITESPACE, *s)) {
		*s++ = 0;
	}
	if (*s == 0) {
		return 0;
	}

	if (strchr(SYMBOLS, *s)) {
		int t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		return t;
	}

	*p1 = s;
	if(*s == '"'){
		s++;
		while(*s && *s != '"'){
			s++;
		}
		if(*s == '"'){
			s++;
		}else{
			usage();
		}
	}else{
		while (*s && !strchr(WHITESPACE SYMBOLS, *s)) {
			s++;
		}
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1) {
	static int c, nc;
	static char *np1, *np2;

	if (s) {
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 128

int parsecmd(char **argv, int *rightpipe, int *background) {
	int argc = 0;
	*background = 0;
	while (1) {
		char *t;
		int fd, r;
		int c = gettoken(0, &t);
		char cwd[MAXPATHLEN];
		switch (c) {
		case 0:
			return argc;
		case 'w':
			if (argc >= MAXARGS) {
				debugf("too many arguments\n");
				exit();
			}
			if(*t == '"'){
				t++;
				t[strlen(t) - 1] = 0;
			}
			argv[argc++] = t;
			break;
		case '<':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			/* Exercise 6.5: Your code here. (1/3) */
			getcwd(cwd);
			chcwd(cwd, t);
			fd = open(cwd, O_RDONLY);
			dup(fd, 0);
			close(fd);

			break;
		case '>':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, dup it onto fd 1, and then close the original fd.
			/* Exercise 6.5: Your code here. (2/3) */
			getcwd(cwd);
			chcwd(cwd, t);
			fd = open(cwd, O_CREAT);
			close(fd);
			fd = open(cwd, O_WRONLY);
			dup(fd, 1);
			close(fd);

			break;
		case '|':;
			/*
			 * First, allocate a pipe.
			 * Then fork, set '*rightpipe' to the returned child envid or zero.
			 * The child runs the right side of the pipe:
			 * - dup the read end of the pipe onto 0
			 * - close the read end of the pipe
			 * - close the write end of the pipe
			 * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
			 *   command line.
			 * The parent runs the left side of the pipe:
			 * - dup the write end of the pipe onto 1
			 * - close the write end of the pipe
			 * - close the read end of the pipe
			 * - and 'return argc', to execute the left of the pipeline.
			 */
			int p[2];
			/* Exercise 6.5: Your code here. (3/3) */
			panic_on(pipe(p));
			if((*rightpipe = fork()) == 0){
				dup(p[0], 0);
				close(p[0]);
				close(p[1]);
				return parsecmd(argv, rightpipe, background);
			}else{
				dup(p[1], 1);
				close(p[1]);
				close(p[0]);
				return argc;
			}

			break;
		case '&':
			if((*rightpipe = fork()) == 0){
				return parsecmd(argv, rightpipe, background);
			}else{
				*background = 1;
				return argc;
			}
			break;
		}
	}

	return argc;
}

#define READLINE_NORMAL 0
#define READLINE_ESC 1
#define READLINE_EXT 2
#define LINE_SIZE 1024
char buf[LINE_SIZE];
u_int cmd_count = 0;

void runcmd(char *s) {
	gettoken(s, 0);
	char *argv[MAXARGS];
	int rightpipe = 0;
	int background = 0;
	
	int argc = parsecmd(argv, &rightpipe, &background);
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;

	int child = spawn(argv[0], argv);
	if(child < 0){
		char temp[LINE_SIZE] = {0};
		strcpy(temp, argv[0]);
		temp[strlen(argv[0])] = '.';
		temp[strlen(argv[0]) + 1] = 'b';
		argv[0] = temp;
		child = spawn(temp, argv);
	}
	close_all();
	if (child >= 0) {
		if(!background){
			wait(child);
		}
	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}
	if (rightpipe) {
		wait(rightpipe);
	}

	exit();
}

//line_num begin at 1
void history_gets_clean(int line_num, char *linebuf){
	int history_read_fd = open(".history", O_RDONLY);
	for(int i = 0; i < line_num; i++) {
		fgets(history_read_fd, linebuf);
	}
	for(int i = strlen(linebuf) - 1; linebuf[i] == '\r' || linebuf[i] == '\n'; i--){
		linebuf[i] = 0;
	}
	close(history_read_fd);
}

void readline(u_int n) {
	int r;
	char c;
	int mode = READLINE_NORMAL;
	int current_cmd = cmd_count;
	int i = 0;
	for (;;) {
		if(i >= n){
			break;
		}
		if ((r = read(0, &c, 1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		}
		if(c == '\r' || c == '\n') {
			return;
		}
		if(c == '\t') {
			printf("\b\b\b\b");
			continue;
		}
		if(mode == READLINE_NORMAL && c == 27) {
			mode = READLINE_ESC;
			continue;
		}
		if(mode == READLINE_ESC && c == 91) {
			mode = READLINE_EXT;
			continue;
		}
		if(mode == READLINE_ESC && c != 91) {
			mode = READLINE_NORMAL;
		}
		u_int old_cmd;
		if(mode == READLINE_EXT) {
			switch(c){
				case 'A'://up
					printf("%c%c%c", 27, 91, 'B');
					if(current_cmd <= 0){
						break;
					}
					current_cmd--;

					for(int j = 0; j < i; j++){
						printf("\b");
					}
					for(int j = 0; j < strlen(buf); j++){
						printf(" ");
					}
					for(int j = 0; j < strlen(buf); j++){
						printf("\b");
					}

					history_gets_clean(current_cmd + 1, buf);
					printf("%s", buf);
					i = strlen(buf);
					
					break;
				case 'B'://down
					if(current_cmd >= cmd_count){
						break;
					}
					current_cmd++;

					for(int j = 0; j < i; j++){
						printf("\b");
					}
					for(int j = 0; j < strlen(buf); j++){
						printf(" ");
					}
					for(int j = 0; j < strlen(buf); j++){
						printf("\b");
					}
					if(current_cmd == cmd_count){
						memset(buf, 0, LINE_SIZE);
						i = 0;
						break;
					}
					history_gets_clean(current_cmd + 1, buf);
					printf("%s", buf);
					i = strlen(buf);

					break;
				case 'C':
					if(buf[i]){
						i++;		
					}else{
						printf("\b");
					}
					break;
				case 'D':
					if(i > 0){
						i--;
					}else{
						printf(" ");
					}
					break;
				default:
					break;
			}
			mode = READLINE_NORMAL;
			continue;
		}
		current_cmd = cmd_count;
		if (c == 0x7f) {
			if (i > 0) {
				printf("\b");
				printf("%s", buf + i);
				printf(" ");
				for(int j = 0; j < strlen(buf + i) + 1; j++){
					printf("\b");
					buf[i + j - 1] = buf[i + j];
				}
				i--;
			}
			continue;
		}
		char current_c = buf[i];
		int count = 0;
		for(int j = i; j < LINE_SIZE; j++){
			if(!current_c){
				break;
			}
			printf("%c", current_c);
			char temp = buf[j + 1];
			buf[j + 1] = current_c;
			current_c = temp;
			count++;
		}
		while(count--){
			printf("\b");
		}
		buf[i] = c;
		i++;
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
		;
	}
	buf[0] = 0;
}


void split_cmd(char *cmdline, char *buffer[], char *temp_buf){
	strcpy(temp_buf, cmdline);
	int cnt = 0;
	buffer[cnt++] = temp_buf;
	for(int i = 0; temp_buf[i]; i++){
		if(temp_buf[i] == '"'){
			i++;
			while(temp_buf[i] && temp_buf[i] != '"'){
				i++;
			}
		}
		if(temp_buf[i] == ';' && temp_buf[i + 1]){
			temp_buf[i] = 0;
			buffer[cnt++] = temp_buf + i + 1;
		}
	}
	buffer[cnt] = NULL;
}

int main(int argc, char **argv) {
	int r;
	int interactive = iscons(0);
	int echocmds = 0;
	debugf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                         ::\n");
	debugf("::                     MOS Shell 2023                      ::\n");
	debugf("::                                                         ::\n");
	debugf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	ARGBEGIN {
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND
	if (argc > 1) {
		usage();
	}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[1], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[1], r);
		}
		user_assert(r == 0);
	}

	int history_write_fd = open(".history", O_CREAT);
	close(history_write_fd);
	history_write_fd = open(".history", O_WRONLY);
	
	putcwd("/");

	for (;;) {
		if (interactive) {
			printf("\n");
			printcwd();
			printf(" $ ");
		}
		memset(buf, 0, LINE_SIZE);
		readline(LINE_SIZE);
		
		if (buf[0] == '#') {
			continue;
		}
		if (echocmds) {
			printf("# %s\n", buf);
		}
		
		write(history_write_fd, buf, strlen(buf));
		write(history_write_fd, "\n", 1);
		char *cmds[LINE_SIZE];
		char temp_buf[LINE_SIZE];
		split_cmd(buf, cmds, temp_buf);
		for(int i = 0; cmds[i]; i++){
			if ((r = fork()) < 0) {
				user_panic("fork: %d", r);
			}
			if (r == 0) {	
				runcmd(cmds[i]);
				exit();
			} else {
				wait(r);
			}	
		}
		cmd_count++;
	}
	return 0;
}
