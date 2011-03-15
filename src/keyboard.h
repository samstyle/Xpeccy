#ifndef _KEYB_H
#define _KEYB_H

#include <stdint.h>

class Keyboard {
	public:
	Keyboard();
	uint8_t map[8];
	void press(uint8_t);
	void release(uint8_t);
	void releaseall();
	uint8_t getmap(uint8_t);
};

class Mouse {
	public:
	Mouse();
	bool enable;
	uint8_t xpos;
	uint8_t ypos;
	uint8_t buttons;	// b0=LMB; b1=RMB;
};

//extern Keyboard *keyb;
//extern Mouse *mouse;

#endif
