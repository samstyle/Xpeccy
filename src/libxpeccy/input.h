#ifndef _KEYB_H
#define _KEYB_H

#ifdef __cplusplus
extern "C" {
#endif
// flags
#define	INF_ENABLED	1
#define	INF_WHEEL	(1<<1)
// mouse
#define	XM_WHEELUP	0
#define	XM_WHEELDN	1
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

typedef struct {
	int flags;
	unsigned char xpos;
	unsigned char ypos;
	unsigned char buttons;	// b0=LMB; b1=RMB;
	unsigned char wheel;
} Mouse;

typedef struct {
	int flags;
	unsigned char map[8];		// ZX keyboard half-row bits
	unsigned char kbdBuf[16];	// PS/2 key buffer
} Keyboard;

typedef struct {
	int type;
	unsigned char state;
} Joystick;

Keyboard* keyCreate();
void keyDestroy(Keyboard*);

void keyPress(Keyboard*,char,char,char);
void keyRelease(Keyboard*,char,char,char);
unsigned char keyInput(Keyboard*,unsigned char);

Mouse* mouseCreate();
void mouseDestroy(Mouse*);
void mouseWheel(Mouse*, int);

Joystick* joyCreate();
void joyDestroy(Joystick*);
void joyPress(Joystick*,unsigned char);
void joyRelease(Joystick*,unsigned char);
unsigned char joyInput(Joystick*);

#ifdef __cplusplus
}
#endif

#endif
