#ifndef _KEYB_H
#define _KEYB_H

#include <stdint.h>

struct Mouse {
	bool enable;
	uint8_t xpos;
	uint8_t ypos;
	uint8_t buttons;	// b0=LMB; b1=RMB;
};

struct Keyboard;

Keyboard* keyCreate();
void keyDestroy(Keyboard*);
void keyPress(Keyboard*,uint8_t);
void keyRelease(Keyboard*,uint8_t);
uint8_t keyInput(Keyboard*,uint8_t);

Mouse* mouseCreate();
void mouseDestroy(Mouse*);

#endif
