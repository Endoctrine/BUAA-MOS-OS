#include <lib.h>

void usage(void) {
	printf("usage: waitsec <time>\n");
	exit();
}

int main(int argc, char **argv) {

	if(argc != 2) {
		usage();
	}
	char *second_a = argv[1];
	u_long second = 0;
	while(*second_a){
		second *= 10;
		second += (*second_a) - '0';
		second_a++;
	}

	for(u_long i = 0; i < second * (u_long)14000000; i++);

	printf("\n");
	printf("------------\n");
	printf("| wait end |\n");
	printf("------------\n");

	return 0;
}
