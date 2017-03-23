#ifndef _KEYB_H
#define _KEYB_H

#ifdef __cplusplus
extern "C" {
#endif

// mouse
enum {
	XM_WHEELUP = 0,
	XM_WHEELDN
};
// joystick type
enum {
	XJ_NONE = 0,
	XJ_KEMPSTON,
	XJ_SINCLAIR_R,
	XJ_SINCLAIR_L
};
// joystick contacts
#define	XJ_LEFT		1
#define	XJ_RIGHT	(1<<1)
#define	XJ_UP		(1<<2)
#define	XJ_DOWN		(1<<3)
#define	XJ_FIRE		(1<<4)
// msx extend keys
#define MSXK_SHIFT	'S'
#define MSXK_CTRL	'C'
#define MSXK_BSP	'B'
#define	MSXK_CAP	'P'
#define MSXK_TAB	'T'
#define MSXK_CODE	'D'
#define MSXK_SEL	'L'
#define MSXK_HOME	'H'
#define MSXK_INS	'I'
#define MSXK_DEL	'X'
#define MSXK_STOP	'Z'
#define MSXK_GRAPH	'G'
#define	MSXK_F1		0x01
#define MSXK_F2		0x02
#define MSXK_F3		0x03
#define MSXK_F4		0x04
#define MSXK_F5		0x05
#define	MSXK_ESC	0x0a
#define	MSXK_LEFT	0x1c
#define MSXK_UP		0x1d
#define	MSXK_DOWN	0x1e
#define	MSXK_RIGHT	0x1f

typedef struct {
	unsigned used:1;
	unsigned enable:1;
	unsigned hasWheel:1;
	unsigned swapButtons:1;

	unsigned char xpos;
	unsigned char ypos;
	unsigned char buttons;	// b0=LMB; b1=RMB;
	unsigned char wheel;
} Mouse;

typedef struct {
	unsigned used:1;
	unsigned char port;
	unsigned char map[8];		// ZX keyboard half-row bits
	unsigned char extMap[8];	// Profi XT-keyboard extend
	unsigned char kbdBuf[16];	// PS/2 key buffer
	unsigned char msxMap[16];	// MSX keys map
	int kBufPos;
} Keyboard;

typedef struct {
	unsigned used:1;
	int type;
	unsigned char state;
} Joystick;

typedef struct {
	unsigned char key1;
	unsigned char key2;
} xKey;

Keyboard* keyCreate();
void keyDestroy(Keyboard*);

unsigned char keyInput(Keyboard*,unsigned short);

void keyPress(Keyboard*,xKey,int);
void keyRelease(Keyboard*,xKey,int);
void keyReleaseAll(Keyboard*);
void keyTrigger(Keyboard*, xKey, int);

void keyPressXT(Keyboard*,int);
void keyReleaseXT(Keyboard*,int);
unsigned char keyReadCode(Keyboard*);

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
