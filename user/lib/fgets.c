#include <lib.h>
int fgets(int fd, char *linebuf){
    int cnt = 0;
    while(read(fd, linebuf + cnt, 1) == 1){
        if(linebuf[cnt] == '\r' || linebuf[cnt] == '\n'){
            linebuf[cnt + 1] = 0;
            return cnt + 1;
        }
        cnt++;
    }
    linebuf[cnt] = 0;
    return -1;
}
