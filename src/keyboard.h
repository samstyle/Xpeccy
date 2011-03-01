#ifndef _KEYB_H
#define _KEYB_H

struct Keyboard {
	Keyboard();
	unsigned char map[8];
	void press(unsigned char);
	void release(unsigned char);
	void releaseall();
	unsigned char getmap(unsigned char);
};

struct Mouse {
	Mouse();
	bool enable;
	unsigned char xpos;
	unsigned char ypos;
	unsigned char buttons;	// b0=LMB; b1=RMB;
};

extern Keyboard *keyb;
extern Mouse *mouse;

#endif
