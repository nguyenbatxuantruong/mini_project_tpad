#ifndef _TGAMEPAD_H_
#define _TGAMEPAD_H_

#include <stdbool.h>

#define xUP (0x01 << 0)
#define xDOWN (0x01 << 1)
#define xLEFT (0x01 << 2)
#define xRIGHT (0x01 << 3)
#define xA (0x01 << 4)
#define xB (0x01 << 5)
#define xX (0x01 << 6)
#define xY (0x01 << 7)

static bool valid_button(int pos, unsigned char raw);
char tgamepad_read();

#endif