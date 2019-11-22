#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "tgamepad.h"

int main()
{
    char input;
    while(1)
    {
    input = tgamepad_read();
    printf("input = %c \n", input);
    }
}