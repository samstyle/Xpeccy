#ifndef _KEYB_H
#define _KEYB_H

#include <stdint.h>

// joystick type
#define XJ_NONE		0
#define XJ_KEMPSTON	1
#define	XJ_SINCLAIR_R	2
#define	XJ_SINCLAIR_L	3
// joystick contacts
#define	XJ_LEFT		1
#define	XJ_RIGHT	(1<<1)
#define	XJ_UP		(1<<2)
#define	XJ_DOWN		(1<<3)
#define	XJ_FIRE		(1<<4)

struct Mouse {
	bool enable;
	uint8_t xpos;
	uint8_t ypos;
	uint8_t buttons;	// b0=LMB; b1=RMB;
};

struct Keyboard;
struct Joystick;

Keyboard* keyCreate();
void keyDestroy(Keyboard*);

void keyPress(Keyboard*,char,char);
void keyRelease(Keyboard*,char,char);
uint8_t keyInput(Keyboard*,uint8_t);

Mouse* mouseCreate();
void mouseDestroy(Mouse*);

Joystick* joyCreate();
void joyDestroy(Joystick*);
void joyPress(Joystick*,uint8_t);
void joyRelease(Joystick*,uint8_t);
uint8_t joyInput(Joystick*);

#endif
