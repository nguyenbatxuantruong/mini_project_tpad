#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "tgamepad.h"

#define TGAMEPAD "/dev/tgamepad0"
#define LEN_OF_READ_BYTE 18

static char up = 'w';
static char down = 's';
static char left = 'a';
static char right = 'd';

static char a_but = 'p';
static char b_but = 'q';
static char x_but = '0';
static char y_but = '1';

static int tgamepad_fd = -1;

static bool valid_button(int pos, unsigned char raw)
{
	unsigned char buff[32];
	int rc;
	int count = 0;
	if(pos < 4 || pos > 5)
	{
		return false;
	}
	while(count <2)
	{
		rc = read(tgamepad_fd, buff, 32);
		if(rc < 18)
		{
			continue;
		}
		if(raw == buff[pos])
		{
			count++;
		}
		else
		{
		return false;
		}
	}
	return true;
}

char tgamepad_read()
{
	unsigned char buff[32];
	int rc;
	if(tgamepad_fd < 0)
	{
		tgamepad_fd = open(TGAMEPAD, 0);
		if(tgamepad_fd < 0)
		{
			perror("Open tgamepad");
		}
	}
	
	rc = read(tgamepad_fd, buff, 32);
	while(rc < 18)
	{
	    rc = read(tgamepad_fd, buff, 32);
	}
	if (buff[4] != 0)
	{
		if (xA == buff[4] && valid_button(4, buff[4])) 
		{
				return a_but; 
		}
		if (xB == buff[4] && valid_button(4, buff[4]))
		{
            return b_but; 
		}
		if (xX == buff[4] && valid_button(4, buff[4]))
		{
            return x_but; 
		}
		if (xY == buff[4] && valid_button(4, buff[4]))
		{
			return y_but; 
		}
	}
	if (buff[5] != 0)
	{
		if (xUP == buff[5] && valid_button(5, buff[5]))
		{
			return up; //up
		}
		if (xDOWN == buff[5] && valid_button(5, buff[5]))
		{
			return down; //down
		}
		if (xLEFT == buff[5] && valid_button(5, buff[5]))
		{
			return left; //left
		}
		if (xRIGHT == buff[5] && valid_button(5, buff[5]))
		{
			return right; //right
		}
	}
	return '?';
}

// void bind_button(char input);
// {
// 	có thể cho nhiều biến thể nhiều loại khác nhau (ý là nhiều hàm bind)
// 	xịn thì có thể viết 1 hàm thôi, rồi input nó là 1 cái mảng chả hạn, nhưng sẽ hơi phức tạp tí
// 	cơ bản, mong muốn của anh về 1 cái hàm bind nó sẽ làm nhiệm vụ như sau
// 	down = input;
// }

