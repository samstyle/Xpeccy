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
enum {
	ENDKEY = 0,

	XKEY_ESC,
	XKEY_F1,
	XKEY_F2,
	XKEY_F3,
	XKEY_F4,
	XKEY_F5,
	XKEY_F6,
	XKEY_F7,
	XKEY_F8,
	XKEY_F9,
	XKEY_F10,
	XKEY_F11,
	XKEY_F12,

	XKEY_TILDA,
	XKEY_1,
	XKEY_2,
	XKEY_3,
	XKEY_4,
	XKEY_5,
	XKEY_6,
	XKEY_7,
	XKEY_8,
	XKEY_9,
	XKEY_0,
	XKEY_MINUS,XKEY_BLINE,
	XKEY_EQUAL,XKEY_PLUS,
	XKEY_BSP,

	XKEY_TAB,
	XKEY_Q,
	XKEY_W,
	XKEY_E,
	XKEY_R,
	XKEY_T,
	XKEY_Y,
	XKEY_U,
	XKEY_I,
	XKEY_O,
	XKEY_P,
	XKEY_LBRACE,XKEY_LBRACK,
	XKEY_RBRACE,XKEY_RBRACK,
	XKEY_SLASH,

	XKEY_CAPS,
	XKEY_A,
	XKEY_S,
	XKEY_D,
	XKEY_F,
	XKEY_G,
	XKEY_H,
	XKEY_J,
	XKEY_K,
	XKEY_L,
	XKEY_DOTCOM,
	XKEY_QUOTE,XKEY_APOS,
	XKEY_ENTER,

	XKEY_LSHIFT,
	XKEY_Z,
	XKEY_X,
	XKEY_C,
	XKEY_V,
	XKEY_B,
	XKEY_N,
	XKEY_M,
	XKEY_PERIOD,
	XKEY_COMMA,
	XKEY_BSLASH,XKEY_QUEST,
	XKEY_RSHIFT,

	XKEY_LCTRL,
	XKEY_LALT,
	XKEY_SPACE,
	XKEY_RALT,
	XKEY_MENU,
	XKEY_RCTRL,

	XKEY_UP,
	XKEY_LEFT,
	XKEY_DOWN,
	XKEY_RIGHT,

	XKEY_DEL,
	XKEY_INS,
	XKEY_HOME,
	XKEY_END,
	XKEY_PGUP,
	XKEY_PGDN,

	XKEY_SYSRQ,
	XKEY_PAUSE,
	XKEY_SCRLCK,

	XKEY_NUMLCK
};

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
} Mouse;

typedef struct {
	int key;
	const char seq[8];
} xKeySeq;

typedef struct {
	char key;
	unsigned char row;
	unsigned char mask;
} xKeyMtrx;

typedef struct {
	unsigned reset:1;		// RES signal to CPU
	unsigned used:1;
	unsigned caps:1;
	unsigned shift:1;
	unsigned lang:1;
	unsigned char port;		// high byte of xxFE port
	int mode;
	unsigned char flag;
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
