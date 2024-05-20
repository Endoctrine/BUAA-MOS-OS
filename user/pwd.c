#include <lib.h>

void usage(void) {
	printf("usage: pwd\n");
	exit();
}

int main(int argc, char **argv) {
	
	if(argc != 1){
		usage();
	}

	printcwd();
	
	return 0;
}
