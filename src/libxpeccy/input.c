#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "input.h"

typedef struct {
	char key;
	unsigned char row;
	unsigned char mask;
} keyScan;

keyScan keyTab[] = {
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

keyScan msxKeyTab[] = {
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
	{MSXK_LEFT,8,16},{MSXK_UP,8,32},{MSXK_DOWN,8,64},{MSXK_RIGHT,8,128},	// space,home,ins,del,left,up,down,right
	{0,0,0}
};

// keyboard

Keyboard* keyCreate() {
	Keyboard* keyb = (Keyboard*)malloc(sizeof(Keyboard));
	memset(keyb, 0x00, sizeof(Keyboard));
	return keyb;
}

void keyDestroy(Keyboard* keyb) {
	free(keyb);
}

void keyPress(Keyboard* keyb, xKey key, int mode) {
	unsigned char* tab;
	keyScan* ktab;
	switch (mode) {
		case 1: tab = keyb->extMap; ktab = keyTab; break;	// Profi ext
		case 2: tab = keyb->msxMap; ktab = msxKeyTab; break;	// MSX
		default: tab = keyb->map; ktab = keyTab; break;	// common
	}
	int i = 0;
	while (ktab[i].key) {
		if (ktab[i].key == (key.key1 & 0x7f)) {
			tab[ktab[i].row] &= ~ktab[i].mask;
			if (key.key1 & 0x80) tab[keyTab[i].row] &= ~0x20;
		}
		if (ktab[i].key == (key.key2 & 0x7f)) {
			tab[ktab[i].row] &= ~ktab[i].mask;
			if (key.key2 & 0x80) tab[keyTab[i].row] &= ~0x20;
		}
		i++;
	}
	i = 0;
}

void keyReleaseAll(Keyboard* keyb) {
	memset(keyb->map, 0x3f, 8);
	memset(keyb->extMap, 0x3f, 8);
	memset(keyb->msxMap, 0xff, 16);
	keyb->kBufPos = 0;
}

void keyRelease(Keyboard* keyb, xKey key, int mode) {
	unsigned char* tab;
	keyScan* ktab;
	switch (mode) {
		case 1: tab = keyb->extMap; ktab = keyTab; break;	// Profi ext
		case 2: tab = keyb->msxMap; ktab = msxKeyTab; break;	// MSX
		default: tab = keyb->map; ktab = keyTab; break;	// common
	}
	int i = 0;
	while (ktab[i].key) {
		if (ktab[i].key == (key.key1 & 0x7f)) {
			tab[ktab[i].row] |= ktab[i].mask;
			if (key.key1 & 0x80) tab[ktab[i].row] |= 0x20;
		}
		if (ktab[i].key == (key.key2 & 0x7f)) {
			tab[ktab[i].row] |= ktab[i].mask;
			if (key.key2 & 0x80) tab[ktab[i].row] |= 0x20;
		}
		i++;
	}
	i = 0;
}

void keyTrigger(Keyboard* keyb, xKey key, int mode) {
	unsigned char* tab;
	keyScan* ktab;
	switch (mode) {
		case 1: tab = keyb->extMap; ktab = keyTab; break;	// Profi ext
		case 2: tab = keyb->msxMap; ktab = msxKeyTab; break;	// MSX
		default: tab = keyb->map; ktab = keyTab; break;	// common
	}
	int i = 0;
	while (ktab[i].key) {
		if (ktab[i].key == (key.key1 & 0x7f)) {
			tab[ktab[i].row] ^= ktab[i].mask;
			if (key.key1 & 0x80) tab[ktab[i].row] |= 0x20;
		}
		if (ktab[i].key == (key.key2 & 0x7f)) {
			tab[ktab[i].row] ^= ktab[i].mask;
			if (key.key2 & 0x80) tab[ktab[i].row] |= 0x20;
		}
		i++;
	}
	i = 0;
}

// read 40-key zxKeyboard port.
// prt.high = rows selector.
// prt.bit0 = use profi ext keys
unsigned char keyInput(Keyboard* keyb, unsigned short prt) {
	unsigned char* ptr = (prt & 1) ? keyb->extMap : keyb->map;
	prt >>= 8;
	unsigned char res = 0x3f;
	keyb->port &= prt;
	keyb->used = 1;
	int i;
	for (i = 0; i < 8; i++) {
		if (~prt & 0x80) res &= ptr[i];
		prt <<= 1;
	}
	return res;
}

void keyPressXT(Keyboard* keyb, int kcod) {
	if (keyb->kBufPos > 13) return;
	while (kcod) {
		keyb->kbdBuf[keyb->kBufPos++] = (kcod & 0xff);
		kcod >>= 8;
	}
}

void keyReleaseXT(Keyboard* keyb, int kcod) {
	if (keyb->kBufPos > 13) return;
	while (kcod) {
		if (kcod < 0x100) keyb->kbdBuf[keyb->kBufPos++] = 0xf0;
		keyb->kbdBuf[keyb->kBufPos++] = (kcod & 0xff);
		kcod >>= 8;
	}
}

unsigned char keyReadCode(Keyboard* keyb) {
	if (keyb->kBufPos < 1) return 0x00;		// empty
	if (keyb->kBufPos > 14) return 0xff;		// overfill
	unsigned char res = keyb->kbdBuf[0];		// read code
	memcpy(keyb->kbdBuf,keyb->kbdBuf + 1,15);	// delete code
	keyb->kBufPos--;
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
			res = (joy->state & 0x1f) | 0xe0;	// high 3 bits is set
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
