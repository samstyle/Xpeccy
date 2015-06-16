#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "input.h"

typedef struct {
	char key;
	unsigned char row;
	unsigned char mask;
} keyScan;

keyScan keyTab[40] = {
	{'1',4,1},{'2',4,2},{'3',4,4},{'4',4,8},{'5',4,16},{'6',3,16},{'7',3,8},{'8',3,4},{'9',3,2},{'0',3,1},
	{'q',5,1},{'w',5,2},{'e',5,4},{'r',5,8},{'t',5,16},{'y',2,16},{'u',2,8},{'i',2,4},{'o',2,2},{'p',2,1},
	{'a',6,1},{'s',6,2},{'d',6,4},{'f',6,8},{'g',6,16},{'h',1,16},{'j',1,8},{'k',1,4},{'l',1,2},{'E',1,1},
	{'C',7,1},{'z',7,2},{'x',7,4},{'c',7,8},{'v',7,16},{'b',0,16},{'n',0,8},{'m',0,4},{'S',0,2},{' ',0,1}
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

void keyPress(Keyboard* keyb, char key1, char key2, char key3, int kcod) {
	int i;
	for (i=0; i<40; i++) {
		if ((keyTab[i].key == key1) || (keyTab[i].key == key2) || (keyTab[i].key == key3)) {
			if (key1) {
				keyb->map[keyTab[i].row] &= ~keyTab[i].mask;
			} else {
				keyb->extMap[keyTab[i].row] &= ~keyTab[i].mask;
			}
		}
	}
	if (keyb->kBufPos < 14) {
		while (kcod) {
			keyb->kbdBuf[keyb->kBufPos++] = (kcod & 0xff);
			kcod >>= 8;
		}
	}
}

void keyRelease(Keyboard* keyb, char key1, char key2, char key3, int kcod) {
	int i;
	if ((key1 == 0) && (key2 == 0) && (key3 == 0)) {
		memset(keyb->map, 0x1f, 8);
		memset(keyb->extMap, 0x1f, 8);
	} else {
		for (i = 0; i < 40; i++) {
			if ((keyTab[i].key == key1) || (keyTab[i].key == key2) || (keyTab[i].key == key3)) {
				if (key1) {
					keyb->map[keyTab[i].row] |= keyTab[i].mask;
				} else {
					keyb->extMap[keyTab[i].row] |= keyTab[i].mask;
				}
			}
		}
	}
	if (keyb->kBufPos < 14) {
		while (kcod) {
			if (kcod < 0x100) keyb->kbdBuf[keyb->kBufPos++] = 0xf0;
			keyb->kbdBuf[keyb->kBufPos++] = (kcod & 0xff);
			kcod >>= 8;
		}
	}
}

unsigned char keybScan(Keyboard* keyb, unsigned char prt, unsigned char* ptr) {
	unsigned char res = 0x1f;
	keyb->port &= prt;
	keyb->used = 1;
	int i;
	for (i = 0; i < 8; i++) {
		if (~prt & 0x80) res &= ptr[i];
		prt <<= 1;
	}
	return res;
}

unsigned char keyInput(Keyboard* keyb, unsigned char prt) {
	return keybScan(keyb, prt, keyb->map);
}

unsigned char keyInputExt(Keyboard* keyb, unsigned char prt) {
	return keybScan(keyb, prt, keyb->extMap);
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
	mou->buttons = 0xff;
	return mou;
}

void mouseDestroy(Mouse* mou) {
	free(mou);
}

void mouseWheel(Mouse* mou, int dir) {
	switch (dir) {
		case XM_WHEELDN:
			mou->buttons += 0x10;
			break;
		case XM_WHEELUP:
			mou->buttons -= 0x10;
			break;
	}
}
