#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define TGAMEPAD "/dev/tgamepad0"
#define LEN_OF_READ_BYTE 18

int main()
{
    unsigned char c[LEN_OF_READ_BYTE];
    int i;
    int fd;
    int rc;

    fd = open(TGAMEPAD, 0);
    if (fd < 0)
    {
        perror("open error\n");
        exit(0);
    }
    while (1)
    {
        rc = read(fd, &c, LEN_OF_READ_BYTE);
        if (rc > 0)
        {
            for(i = 0; i < rc; i++)
            {
                printf(" %u,",c[i]);
            }
            printf(" -- %u\n", rc);
        }
    }
    return 0;
}
