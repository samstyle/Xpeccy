#ifndef _KEYB_H
#define _KEYB_H

#ifdef __cplusplus
extern "C" {
#endif

// mouse
//enum {
//	XM_WHEELUP = 0,
//	XM_WHEELDN
//};
// joystick type
enum {
	XJ_NONE = 0,
	XJ_KEMPSTON,
	XJ_SINCLAIR_R,
	XJ_SINCLAIR_L
};

enum {
	XM_NONE = 0,
	XM_UP,
	XM_DOWN,
	XM_LEFT,
	XM_RIGHT,
	XM_LMB,
	XM_MMB,
	XM_RMB,
	XM_WHEELUP,
	XM_WHEELDN
};

// keyboard mode
enum {
	KBD_NONE = 0,
	KBD_SPECTRUM,
	KBD_PROFI,
	KBD_MSX,
	KBD_ATM2,
	KBD_C64
};

// atm2 mode submodes
enum {
	kbdZX = 0,
	kbdCODE,
	kbdCPM,
	kbdDIRECT
};

#define KFL_SHIFT	(1)
#define KFL_CTRL	(1<<1)
#define KFL_ALT		(1<<2)
#define KFL_CAPS	(1<<4)
#define KFL_NUMLOCK	(1<<5)
#define KFL_SCRLOCK	(1<<6)
#define KFL_RUS		(1<<7)
#define KFL_RSHIFT	KFL_SHIFT

// joystick contacts
#define XJ_NONE		0
#define	XJ_RIGHT	1
#define	XJ_LEFT		(1<<1)
#define	XJ_DOWN		(1<<2)
#define	XJ_UP		(1<<3)
#define	XJ_FIRE		(1<<4)
#define XJ_BUT2		(1<<5)
#define XJ_BUT3		(1<<6)
#define XJ_BUT4		(1<<7)
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
// pc keys
#define	XKEY_1	10
#define	XKEY_2	11
#define	XKEY_3	12
#define	XKEY_4	13
#define	XKEY_5	14
#define	XKEY_6	15
#define	XKEY_7	16
#define	XKEY_8	17
#define	XKEY_9	18
#define	XKEY_0	19
#define	XKEY_MINUS	20
#define	XKEY_PLUS	21
#define	XKEY_BSP	22
#define	XKEY_TAB	23
#define	XKEY_Q	24
#define	XKEY_W	25
#define	XKEY_E	26
#define	XKEY_R	27
#define	XKEY_T	28
#define	XKEY_Y	29
#define	XKEY_U	30
#define	XKEY_I	31
#define	XKEY_O	32
#define	XKEY_P	33
#define	XKEY_LBRACE	34
#define	XKEY_RBRACE	35
#define	XKEY_ENTER	36
#define	XKEY_LCTRL	37
#define	XKEY_A	38
#define	XKEY_S	39
#define	XKEY_D	40
#define	XKEY_F	41
#define	XKEY_G	42
#define	XKEY_H	43
#define	XKEY_J	44
#define	XKEY_K	45
#define	XKEY_L	46
#define	XKEY_DOTCOM	47	// ;
#define	XKEY_QUOTE	48	// "
#define	XKEY_TILDA	49	// ~
#define	XKEY_LSHIFT	50
#define	XKEY_SLASH	51
#define	XKEY_Z	52
#define	XKEY_X	53
#define	XKEY_C	54
#define	XKEY_V	55
#define	XKEY_B	56
#define	XKEY_N	57
#define	XKEY_M	58
#define	XKEY_PERIOD	59
#define	XKEY_COMMA	60
#define	XKEY_BSLASH	61	// /
#define	XKEY_SPACE	65
#define	XKEY_CAPS	66
#define	XKEY_RSHIFT	62
#define	XKEY_RCTRL	105
#define XKEY_LALT	64
#define	XKEY_RALT	108
#define	XKEY_HOME	110
#define	XKEY_UP		111
#define	XKEY_PGUP	112
#define	XKEY_LEFT	113
#define	XKEY_RIGHT	114
#define	XKEY_END	115
#define	XKEY_DOWN	116
#define	XKEY_PGDN	117
#define	XKEY_INS	118
#define	XKEY_DEL	119
#define	XKEY_MENU	135
#define XKEY_ESC	9
#define XKEY_F1		67
#define XKEY_F2		68
#define XKEY_F3		69
#define XKEY_F4		70
#define XKEY_F5		71
#define XKEY_F6		72
#define XKEY_F7		73
#define XKEY_F8		74
#define XKEY_F9		75
#define XKEY_F10	76
#define XKEY_F11	95
#define XKEY_F12	96
#define XKEY_LBRACK	256	// {
#define XKEY_RBRACK	257	// }
#define XKEY_QUEST	258
#define XKEY_SYSRQ	259
#define XKEY_PAUSE	260
#define XKEY_SCRLCK	261
#define XKEY_NUMLCK	262
#define XKEY_APOS	263
#define ENDKEY		0

typedef struct {
	unsigned used:1;
	unsigned enable:1;
	unsigned hasWheel:1;
	unsigned swapButtons:1;

	unsigned lmb:1;
	unsigned rmb:1;
	unsigned mmb:1;
	unsigned char wheel;

	unsigned char xpos;
	unsigned char ypos;
	int autox;
	int autoy;
//	unsigned char buttons;	// b0=LMB; b1=RMB; b2=MMB; b4-7=wheel
} Mouse;

typedef struct {
	unsigned reset:1;		// RES signal to CPU
	unsigned used:1;
	unsigned caps:1;
	unsigned shift:1;
	unsigned lang:1;
	unsigned char port;		// high byte of xxFE port
	int mode;
	// i8031 block
	unsigned wcom:1;		// i8031 waiting for command
	unsigned warg:1;		// i8031 waiting for argument
	int submode;			// i8031 mode
	unsigned char com;
	unsigned char arg;
	unsigned char keycode;		// current pressed key code
	unsigned char lastkey;		// last pressed key code
	unsigned char flag1;		// [7] rus.scrlock.numlock.caps.0.alt.ctrl.shift [0]
	unsigned char flag2;		// [7] 0.0.0.0.0.0.0.rshift [0]
	// key matrix
	unsigned char map[8];		// ZX keyboard half-row bits
	unsigned char extMap[8];	// Profi XT-keyboard extend
	unsigned char msxMap[16];	// MSX keys map
	// pc keyboard keybuffer
	unsigned char kbdBuf[16];	// PS/2 key buffer
	int kBufPos;
} Keyboard;

typedef struct {
	unsigned used:1;
	unsigned extbuttons:1;
	int type;
	unsigned char state;
} Joystick;

typedef struct {
	unsigned char key1;
	unsigned char key2;
} xKey;

typedef struct {
	unsigned char cpmCode;
	unsigned char rowScan;
} atmKey;

typedef struct {
	const char* name;
	signed int key;		// XKEY_*
	xKey zxKey;
	xKey extKey;
	xKey msxKey;
	atmKey atmCode;
	int keyCode;		// 0xXXYYZZ = ZZ,YY,XX in buffer ([ZZ],[YY],0xf0,XX if released)
} keyEntry;

Keyboard* keyCreate();
void keyDestroy(Keyboard*);
void kbdSetMode(Keyboard*, int);
void kbdPress(Keyboard*, keyEntry);
void kbdRelease(Keyboard*, keyEntry);
void kbdTrigger(Keyboard*, keyEntry);
void kbdReleaseAll(Keyboard*);
unsigned char kbdRead(Keyboard*, unsigned short);
unsigned char keyReadCode(Keyboard*);

Mouse* mouseCreate();
void mouseDestroy(Mouse*);
void mousePress(Mouse*, int, int);
void mouseRelease(Mouse*, int);
void mouseReleaseAll(Mouse*);

Joystick* joyCreate();
void joyDestroy(Joystick*);
void joyPress(Joystick*,unsigned char);
void joyRelease(Joystick*,unsigned char);
unsigned char joyInput(Joystick*);

#ifdef __cplusplus
}
#endif

#endif
