#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "input.h"

typedef struct {
	char key;
	unsigned char row;
	unsigned char mask;
} keyScan;

static keyScan keyTab[] = {
	{'1',4,1},{'2',4,2},{'3',4,4},{'4',4,8},{'5',4,16},{'6',3,16},{'7',3,8},{'8',3,4},{'9',3,2},{'0',3,1},
	{'q',5,1},{'w',5,2},{'e',5,4},{'r',5,8},{'t',5,16},{'y',2,16},{'u',2,8},{'i',2,4},{'o',2,2},{'p',2,1},
	{'a',6,1},{'s',6,2},{'d',6,4},{'f',6,8},{'g',6,16},{'h',1,16},{'j',1,8},{'k',1,4},{'l',1,2},{'E',1,1},
	{'C',7,1},{'z',7,2},{'x',7,4},{'c',7,8},{'v',7,16},{'b',0,16},{'n',0,8},{'m',0,4},{'S',0,2},{' ',0,1},
	{0,0,0}
};

/*
  Line  Bit_7 Bit_6 Bit_5 Bit_4 Bit_3 Bit_2 Bit_1 Bit_0
   0     "7"   "6"   "5"   "4"   "3"   "2"   "1"   "0"
   1     ";"   "]"   "["   "\"   "="   "-"   "9"   "8"
   2     "B"   "A"   ???   "/"   "."   ","   "'"   "`"
   3     "J"   "I"   "H"   "G"   "F"   "E"   "D"   "C"
   4     "R"   "Q"   "P"   "O"   "N"   "M"   "L"   "K"
   5     "Z"   "Y"   "X"   "W"   "V"   "U"   "T"   "S"
   6     F3    F2    F1   CODE   CAP  GRAPH CTRL  SHIFT
   7     RET   SEL   BS   STOP   TAB   ESC   F5    F4
   8    RIGHT DOWN   UP   LEFT   DEL   INS  HOME  SPACE
 ( 9    NUM4  NUM3  NUM2  NUM1  NUM0  NUM/  NUM+  NUM*  )
 ( 10   NUM.  NUM,  NUM-  NUM9  NUM8  NUM7  NUM6  NUM5  )
*/

static keyScan msxKeyTab[] = {
	{'0',0,1},{'1',0,2},{'2',0,4},{'3',0,8},{'4',0,16},{'5',0,32},{'6',0,64},{'7',0,128},
	{'8',1,1},{'9',1,2},{'-',1,4},{'=',1,8},{'\\',1,16},{'[',1,32},{']',1,64},{';',1,128},
	{'`',2,1},{0X27,2,2},{',',2,4},{'.',2,8},{'/',2,16},{255,2,32},{'a',2,64},{'b',2,128},
	{'c',3,1},{'d',3,2},{'e',3,4},{'f',3,8},{'g',3,16},{'h',3,32},{'i',3,64},{'j',3,128},
	{'k',4,1},{'l',4,2},{'m',4,4},{'n',4,8},{'o',4,16},{'p',4,32},{'q',4,64},{'r',4,128},
	{'s',5,1},{'t',5,2},{'u',5,4},{'v',5,8},{'w',5,16},{'x',5,32},{'y',5,64},{'z',5,128},
	{MSXK_SHIFT,6,1},{MSXK_CTRL,6,2},
	{MSXK_GRAPH,6,4},	// graph
	{MSXK_CAP,6,8},
	{MSXK_CODE,6,16},
	{MSXK_F1,6,32},
	{MSXK_F2,6,64},
	{MSXK_F3,6,128},
	{MSXK_F4,7,1},
	{MSXK_F5,7,2},
	{MSXK_ESC,7,4},
	{MSXK_TAB,7,8},		// tab
	{MSXK_STOP,7,16},	// stop
	{MSXK_BSP,7,32},	// bsp
	{MSXK_SEL,7,64},	// sel
	{'E',7,128},		// ret
	{' ',8,1},
	{MSXK_HOME,8,2},
	{MSXK_INS,8,4},
	{MSXK_DEL,8,8},
	{MSXK_LEFT,8,16},
	{MSXK_UP,8,32},
	{MSXK_DOWN,8,64},
	{MSXK_RIGHT,8,128},
	{0,0,0}
};

keyScan findKey(keyScan* tab, char key) {
	int idx = 0;
	while (tab[idx].key && (tab[idx].key != key)) {
		idx++;
	}
	return tab[idx];
}

// keyboard

Keyboard* keyCreate() {
	Keyboard* keyb = (Keyboard*)malloc(sizeof(Keyboard));
	memset(keyb, 0x00, sizeof(Keyboard));
	return keyb;
}

void keyDestroy(Keyboard* keyb) {
	free(keyb);
}

void kbdSetMode(Keyboard* kbd, int mode) {
	kbd->mode = mode;
}

// key press/release/trigger

void kbd_press_key(Keyboard* kbd, keyScan* tab, unsigned char* mtrx, char ch) {
	keyScan key = findKey(tab, ch & 0x7f);
	key.row &= 0x0f;
	mtrx[key.row] &= ~key.mask;
	if (ch & 0x80)
		mtrx[key.row] &= ~0x20;
	for (int i = 0; i < 8; i++) {
		if (key.mask & (1 << i))
			kbd->matrix[key.row][i]++;
	}
}

void kbd_press(Keyboard* kbd, keyScan* tab, unsigned char* mtrx, xKey xk) {
	kbd_press_key(kbd, tab, mtrx, xk.key1);
	kbd_press_key(kbd, tab, mtrx, xk.key2);
}

void kbdPress(Keyboard* kbd, keyEntry ent) {
	switch(kbd->mode) {
		case KBD_SPECTRUM:
			kbd_press(kbd, keyTab, kbd->map, ent.zxKey);
			break;
		case KBD_PROFI:					// profi = spectrum + ext
			kbd_press(kbd, keyTab, kbd->extMap, ent.extKey);
			kbd_press(kbd, keyTab, kbd->map, ent.zxKey);
			break;
		case KBD_MSX:
			kbd_press(kbd, msxKeyTab, kbd->msxMap, ent.msxKey);
			break;
		case KBD_ATM2:
			switch(ent.key) {
				case XKEY_LSHIFT: kbd->flag1 |= KFL_SHIFT; break;
				case XKEY_RSHIFT: kbd->flag2 |= KFL_RSHIFT; break;
				case XKEY_RCTRL:
				case XKEY_LCTRL: kbd->flag1 |= KFL_CTRL; break;
				case XKEY_RALT:
				case XKEY_LALT: kbd->flag1 |= KFL_ALT; break;
				case XKEY_CAPS: kbd->flag1 ^= KFL_CAPS; break;
			}
			switch (kbd->submode) {
				case kbdZX:
					kbd->keycode = ent.atmCode.rowScan;
					kbd->lastkey = kbd->keycode;
					kbd->map[((ent.atmCode.rowScan >> 4) & 7) ^ 7] &= ~(1 << ((ent.atmCode.rowScan & 7) - 1));
					if (ent.atmCode.rowScan & 0x80) kbd->map[0] &= ~2;	// sym.shift
					if (ent.atmCode.rowScan & 0x08) kbd->map[7] &= ~1;	// cap.shift
					break;
				case kbdCODE:
				case kbdCPM:
					kbd->keycode = ent.atmCode.cpmCode;
					kbd->lastkey = kbd->keycode;
					break;
				case kbdDIRECT:
					break;
			}
			break;
	}
}

void kbd_release_key(Keyboard* kbd, keyScan* tab, unsigned char* mtrx, char ch) {
	keyScan key = findKey(tab, ch & 0x7f);
	key.row &= 0x0f;
	for (int i = 0; i < 8; i++) {
		if (key.mask & (1 << i)) {
			kbd->matrix[key.row][i]--;
			if (!kbd->matrix[key.row][i]) {
				mtrx[key.row] |= key.mask;
			}
		}
	}
}

void kbd_release(Keyboard* kbd, keyScan* tab, unsigned char* mtrx, xKey xk) {
	kbd_release_key(kbd, tab, mtrx, xk.key1);
	kbd_release_key(kbd, tab, mtrx, xk.key2);
}

void kbdRelease(Keyboard* kbd, keyEntry ent) {
	switch(kbd->mode) {
		case KBD_SPECTRUM:
			kbd_release(kbd, keyTab, kbd->map, ent.zxKey);
			break;
		case KBD_PROFI:
			kbd_release(kbd, keyTab, kbd->extMap, ent.extKey);
			kbd_release(kbd, keyTab, kbd->map, ent.zxKey);
			break;
		case KBD_MSX:
			kbd_release(kbd, msxKeyTab, kbd->msxMap, ent.msxKey);
			break;
		case KBD_ATM2:
			switch(ent.key) {
				case XKEY_LSHIFT: kbd->flag1 &= ~KFL_SHIFT; break;
				case XKEY_RSHIFT: kbd->flag2 &= ~KFL_RSHIFT; break;
				case XKEY_RCTRL:
				case XKEY_LCTRL: kbd->flag1 &= ~KFL_CTRL; break;
				case XKEY_RALT:
				case XKEY_LALT: kbd->flag1 &= ~KFL_ALT; break;
			}
			switch (kbd->submode) {
				case kbdZX:
					kbd->keycode = 0;
					kbd->map[((ent.atmCode.rowScan >> 4) & 7) ^ 7] |= (1 << ((ent.atmCode.rowScan & 7) - 1));
					if (ent.atmCode.rowScan & 0x80) kbd->map[0] |= 2;	// sym.shift
					if (ent.atmCode.rowScan & 0x08) kbd->map[7] |= 1;	// cap.shift
					break;
				case kbdCODE:
				case kbdCPM:
					kbd->keycode = 0;
					break;
				case kbdDIRECT:
					break;
			}
			break;
	}
}

void kbdReleaseAll(Keyboard* kbd) {
	int r,k;
	memset(kbd->map, 0xff, 8);
	memset(kbd->extMap, 0xff, 8);
	memset(kbd->msxMap, 0xff, 16);
	for (r = 0; r < 16; r++) {
		for (k = 0; k < 8; k++) {
			kbd->matrix[r][k] = 0;
		}
	}
	kbd->keycode = 0;
	kbd->lastkey = 0;
	kbd->kBufPos = 0;
	kbd->flag = 0;
}

void kbd_trigger(keyScan* tab, unsigned char* mtrx, xKey xk) {
	keyScan key = findKey(tab, xk.key1 & 0x7f);
	if (xk.key1 & 0x80) key.mask |= 0x20;
	mtrx[key.row] ^= key.mask;
	key = findKey(tab, xk.key2 & 0x7f);
	if (xk.key2 & 0x80) key.mask |= 0x20;
	mtrx[key.row] ^= key.mask;
}

void kbdTrigger(Keyboard* kbd, keyEntry ent) {
	switch(kbd->mode) {
		case KBD_SPECTRUM:
			kbd_trigger(keyTab, kbd->map, ent.zxKey);
			break;
		case KBD_PROFI:
			kbd_trigger(keyTab, kbd->extMap, ent.extKey);
			kbd_trigger(keyTab, kbd->map, ent.zxKey);
			break;
		case KBD_MSX:
			kbd_trigger(msxKeyTab, kbd->msxMap, ent.msxKey);
			break;
	}
	// at/xt ???
}

// at/xt keyboard buffer

void xt_press(Keyboard* kbd, int code) {
	while (code && (kbd->kBufPos < 16)) {
		kbd->kbdBuf[kbd->kBufPos++] = code & 0xff;
		code >>= 8;
	}
}

void xt_release(Keyboard* kbd, int code) {
	while (code && (kbd->kBufPos < 16)) {
		if (code < 0x100)
			kbd->kbdBuf[kbd->kBufPos++] = 0xf0;
		kbd->kbdBuf[kbd->kBufPos++] = (code & 0xff);
		code >>= 8;
	}
}

// at/xt buffer reading
unsigned char keyReadCode(Keyboard* keyb) {
	if (keyb->kBufPos < 1) return 0x00;		// empty
	if (keyb->kBufPos > 14) return 0xff;		// overfill
	unsigned char res = keyb->kbdBuf[0];		// read code
	for (int i = 0; i < 15; i++) {
		keyb->kbdBuf[i] = keyb->kbdBuf[i + 1];
	}
	keyb->kBufPos--;
	return res;
}

#include <time.h>

static unsigned char kmodTab[4] = {kbdZX, kbdCODE, kbdCPM, kbdDIRECT};
static unsigned char kmodVer[4] = {6,0,1,0};

unsigned char kbdScanZX(Keyboard* kbd, unsigned short port) {
	unsigned char res = 0x3f;
	for (int i = 0; i < 8; i++) {
		if (!(port & 0x8000))
			res &= kbd->map[i];
		port <<= 1;
	}
	return res;
}

unsigned char kbdScanProfi(Keyboard* kbd, unsigned short port) {
	unsigned char res = 0x3f;
	for (int i = 0; i < 8; i++) {
		if (!(port & 0x8000)) {
			res &= kbd->extMap[i];
			res &= (kbd->map[i] | 0x20);
		}
		port <<= 1;
	}
	return res;
}

unsigned char kbdRead(Keyboard* kbd, unsigned short port) {
	unsigned char res = 0xff;
	int hi;

	time_t rtime;
	time(&rtime);
	struct tm* ctime = localtime(&rtime);

	switch (kbd->mode) {
		case KBD_SPECTRUM:
			res = kbdScanZX(kbd, port);
			break;
		case KBD_PROFI:
			res = kbdScanProfi(kbd, port);
			break;
		case KBD_MSX:			// port = row register
			res = kbd->msxMap[port & 0x0f];
			break;
		case KBD_ATM2:
			hi = (port >> 14) & 3;
			if (kbd->wcom) {
				kbd->com = (port >> 8) & 0x3f;
				kbd->wcom = 0;
				switch(kbd->com) {
					case 0x01: res = kmodVer[hi]; break;
					case 0x07: kbd->kBufPos = 0; break;
					case 0x09:
						switch(hi) {
							case 0:
								res = kbd->keycode;
								kbd->keycode = 0;
								break;
							case 1: res = kbd->lastkey; break;
							case 2: res = kbd->flag1; break;
							case 3: res = kbd->flag2; break;
						}
						break;
					case 0x0a: kbd->flag1 |= KFL_RUS; break;
					case 0x0b: kbd->flag1 &= ~KFL_RUS; break;
					case 0x0c: break;			// TODO: pause
					case 0x0d:
						kbd->reset = 1;
						break;
					case 0x10:
						switch(hi) {
							case 0: res = ctime->tm_sec & 0xff; break;
							case 1: res = ctime->tm_min & 0xff; break;
							case 2: res = ctime->tm_hour & 0xff; break;
						}
						break;
					case 0x12:
						switch(hi) {
							case 0: res = ctime->tm_mday & 0xff; break;
							case 1: res = ctime->tm_mon & 0xff; break;
							case 2: res = ctime->tm_year & 0xff; break;
						}
						//printf("%i = %i\n",hi,res);
						break;
					case 0x16: break;
					case 0x17: break;
					case 0x11:				// set time
					case 0x13:				// set date
					case 0x14:				// set P1 bits
					case 0x15:				// res P1 bits
					case 0x08:				// set mode
						kbd->warg = 1;
						break;
					default: res = 0xff; break;
				}
			} else if (kbd->warg) {
				kbd->arg = (port >> 8) & 0xff;
				kbd->warg = 0;
				switch (kbd->com) {
					case 0x08:
						kbd->submode = kmodTab[kbd->arg & 3];
						break;
				}
			} else if ((port & 0xff00) == 0x5500) {
				kbd->wcom = 1;
				kbd->warg = 0;
				res = 0xaa;
			} else {
				switch(kbd->submode) {
					case kbdZX:
						res = kbdScanZX(kbd, port);
						break;
					case kbdCODE:
						res = kbd->keycode;
						kbd->keycode = 0;
						break;
					case kbdCPM:
						switch(hi) {
							case 0: res = kbd->keycode;
								kbd->keycode = 0;
								break;
							case 1: res = kbd->flag2;
								break;
							case 2: res = kbd->flag1;
								break;
						}
						break;
					case kbdDIRECT:
						break;
				}
			}
			break;
	}
	return res;
}

// joystick

Joystick* joyCreate() {
	Joystick* joy = (Joystick*)malloc(sizeof(Joystick));
	joy->type = XJ_KEMPSTON;
	joy->state = 0;
	return joy;
}

void joyDestroy(Joystick* joy) {
	free(joy);
}

void joyPress(Joystick* joy, unsigned char mask) {
	joy->state |= mask;
}

void joyRelease(Joystick* joy, unsigned char mask) {
	joy->state &= ~mask;
}

unsigned char joyInput(Joystick* joy) {
	unsigned char res = 0xff;
	joy->used = 1;
	switch (joy->type) {
		case XJ_KEMPSTON:
			res = joy->state;
			if (!joy->extbuttons)
				res |= ~0x1f;
			break;
	}
	return res;
}

// mouse

Mouse* mouseCreate() {
	Mouse* mou = (Mouse*)malloc(sizeof(Mouse));
	memset(mou,0x00,sizeof(Mouse));
	return mou;
}

void mouseDestroy(Mouse* mou) {
	free(mou);
}

void mouseReleaseAll(Mouse* mou) {
	mou->lmb = 0;
	mou->rmb = 0;
	mou->mmb = 0;
	mou->autox = 0;
	mou->autoy = 0;
}

void mousePress(Mouse* mou, int wut, int val) {
	switch(wut) {
		case XM_LMB: mou->lmb = 1; break;
		case XM_RMB: mou->rmb = 1; break;
		case XM_MMB: mou->mmb = 1; break;
		case XM_WHEELDN: mou->wheel++; break;
		case XM_WHEELUP: mou->wheel--; break;
		case XM_UP: mou->autoy = val; break;
		case XM_DOWN: mou->autoy = -val; break;
		case XM_LEFT: mou->autox = -val; break;
		case XM_RIGHT: mou->autox = val; break;
	}
}

void mouseRelease(Mouse* mou, int wut) {
	switch(wut) {
		case XM_LMB: mou->lmb = 0; break;
		case XM_RMB: mou->rmb = 0; break;
		case XM_MMB: mou->mmb = 0; break;
		case XM_UP:
		case XM_DOWN: mou->autoy = 0; break;
		case XM_LEFT:
		case XM_RIGHT: mou->autox = 0; break;
	}
}
