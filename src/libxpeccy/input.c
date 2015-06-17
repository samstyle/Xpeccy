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

// keyboard

Keyboard* keyCreate() {
	Keyboard* keyb = (Keyboard*)malloc(sizeof(Keyboard));
	memset(keyb, 0x00, sizeof(Keyboard));
	return keyb;
}

void keyDestroy(Keyboard* keyb) {
	free(keyb);
}

void keyPress(Keyboard* keyb, unsigned char key, int ext) {
	unsigned char* tab = ext ? keyb->extMap : keyb->map;
	int i = 0;
	while (keyTab[i].key) {
		if (keyTab[i].key == (key & 0x7f)) {
			tab[keyTab[i].row] &= ~keyTab[i].mask;
			if (key & 0x80)
				tab[keyTab[i].row] &= ~0x20;
		}
		i++;
	}
}

void keyRelease(Keyboard* keyb, unsigned char key, int ext) {
	if (key == 0xff) {
		memset(keyb->map, 0x3f, 8);
		memset(keyb->extMap, 0x3f, 8);
		keyb->kBufPos = 0;
	} else {
		unsigned char* tab = ext ? keyb->extMap : keyb->map;
		int i = 0;
		while (keyTab[i].key) {
			if (keyTab[i].key == (key & 0x7f)) {
				tab[keyTab[i].row] |= keyTab[i].mask;
				if (key & 0x80)
					tab[keyTab[i].row] |= 0x20;
			}
			i++;
		}
	}
}

unsigned char keyInput(Keyboard* keyb, unsigned char prt, int ext) {
	unsigned char* ptr = ext ? keyb->extMap : keyb->map;
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
