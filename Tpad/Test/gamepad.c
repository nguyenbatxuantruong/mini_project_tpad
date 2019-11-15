#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define TGAMEPAD "/dev/tgamepad0"
int main()
{
    char c[32];
    int fd;
    int rc;

    fd = open(TGAMEPAD, 0);
    if(fd < 0)
    {
	perror("open error\n");
	exit(0);
    }
    while(1)
    {
        rc = read(fd, &c, 32);
	if(rc > 0)
        {
	    printf("input = %d, %d, %d, %d, %d, %d, %d, %d -- %d\n", 
			    c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7], rc);
	}
    }
    return 0;
}
