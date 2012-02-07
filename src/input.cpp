#include <stdint.h>
#include "input.h"

<<<<<<< HEAD
#define ZK(p1,p2,p3,p4) {{p1,p2},{p3,p4}}

#ifndef WIN32

uint8_t keys[256][2][2]={
	ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),			// 0
	ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),			// 5
	ZK(4,1,0,0),ZK(4,2,0,0),ZK(4,4,0,0),ZK(4,8,0,0),ZK(4,16,0,0),			// 10	1  2  3  4  5
	ZK(3,16,0,0),ZK(3,8,0,0),ZK(3,4,0,0),ZK(3,2,0,0),ZK(3,1,0,0),			// 15	6  7  8  9  0
	ZK(0,2,1,8),ZK(0,2,1,4),ZK(7,1,3,1),ZK(7,1,0,1),				// 20:	-  +  bs tb
	ZK(5,1,0,0),ZK(5,2,0,0),ZK(5,4,0,0),ZK(5,8,0,0),ZK(5,16,0,0),			// 24	q  w  e  r  t
	ZK(2,16,0,0),ZK(2,8,0,0),ZK(2,4,0,0),ZK(2,2,0,0),ZK(2,1,0,0),			// 29	y  u  i  o  p
	ZK(3,4,0,2),ZK(3,2,0,2),ZK(1,1,0,0),ZK(0,2,0,0),				// 34	[  ]  en ct
	ZK(6,1,0,0),ZK(6,2,0,0),ZK(6,4,0,0),ZK(6,8,0,0),ZK(6,16,0,0),			// 38	a  s  d  f  g
	ZK(1,16,0,0),ZK(1,8,0,0),ZK(1,4,0,0),ZK(1,2,0,0),ZK(0,2,2,2),			// 43	h  j  k  l  ;
	ZK(2,1,0,2),ZK(0,2,7,1),ZK(7,1,0,0),						// 48	'  `  ls
	ZK(0,0,0,0),ZK(7,2,0,0),ZK(7,4,0,0),ZK(7,8,0,0),ZK(7,16,0,0),			// 51	\  z  x  c  v
	ZK(0,16,0,0),ZK(0,8,0,0),ZK(0,4,0,0),ZK(0,2,0,8),ZK(0,2,0,4),			// 56	b  n  m  ,  .
	ZK(0,2,7,8),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,1,0,0),			// 61	?           sp
	ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),			// 66
	ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),			// 71
	ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),			// 76
	ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),			// 81
	ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),			// 86
	ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),			// 91
	ZK(0,0,0,0),ZK(0,2,5,1),ZK(7,1,3,8),ZK(7,1,4,4),ZK(7,1,4,16),			// 96-100 97:home(SS+Q) 98:up 99:pgup 100:lft
	ZK(0,0,0,0),ZK(7,1,3,4),ZK(0,2,5,4),ZK(7,1,3,16),ZK(7,1,4,8),			// 101-105 102:right 103:end(SS+E) 104:down 105:pgdn
	ZK(0,2,5,2),ZK(7,1,3,2),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,2,5,1),			//  106:ins(SS+W) 107:delete(CS+9) 110:home
	ZK(7,1,3,8),ZK(7,1,4,4),ZK(7,1,4,16),ZK(7,1,3,4),ZK(0,2,5,4),			// 111:up 112:pgup 113:lf 114:rg 115:end
	ZK(7,1,3,16),ZK(7,1,4,8),ZK(0,2,5,2),ZK(7,1,3,2)				// 116:dn 117:pgup 118:ins 119:del
};

# else

uint8_t keys[256][2][2]={
	ZK(0,0,0,0),ZK(0,0,0,0),						// 0  :
	ZK(4,1,0,0),ZK(4,2,0,0),ZK(4,4,0,0),ZK(4,8,0,0),ZK(4,16,0,0),		// 2  : 1  2  3  4  5
	ZK(3,16,0,0),ZK(3,8,0,0),ZK(3,4,0,0),ZK(3,2,0,0),ZK(3,1,0,0),		// 7  : 6  7  8  9  0
	ZK(0,2,1,8),ZK(0,2,1,4),ZK(7,1,3,1),ZK(7,1,0,1),			// 12 : +  -  bs tb
	ZK(5,1,0,0),ZK(5,2,0,0),ZK(5,4,0,0),ZK(5,8,0,0),ZK(5,16,0,0),		// 16 : Q  W  E  R  T
	ZK(2,16,0,0),ZK(2,8,0,0),ZK(2,4,0,0),ZK(2,2,0,0),ZK(2,1,0,0),		// 21 : Y  U  I  O  P
	ZK(3,4,0,2),ZK(3,2,0,2),ZK(1,1,0,0),ZK(0,2,0,0),			// 26 : [  ]  en ct
	ZK(6,1,0,0),ZK(6,2,0,0),ZK(6,4,0,0),ZK(6,8,0,0),ZK(6,16,0,0),		// 30 : A  S  D  F  G
	ZK(1,16,0,0),ZK(1,8,0,0),ZK(1,4,0,0),ZK(1,2,0,0),ZK(0,2,2,2),		// 35 : H  J  K  L  ;
	ZK(2,1,0,2),ZK(0,2,7,1),ZK(7,1,0,0),					// 40 : '  `  ls
	ZK(0,2,7,1),ZK(7,2,0,0),ZK(7,4,0,0),ZK(7,8,0,0),ZK(7,16,0,0),		// 43 : \  Z  X  C  V
	ZK(0,16,0,0),ZK(0,8,0,0),ZK(0,4,0,0),ZK(0,2,0,8),ZK(0,2,0,4),		// 48 : B  N  M  ,  .
	ZK(0,2,7,8),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,1,0,0),		// 53 : ?  rs    la sp
	ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),		// 58 :
	ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),		// 63 :
	ZK(0,0,0,0),ZK(0,0,0,0),ZK(0,0,0,0),					// 68 :
	ZK(0,2,5,1),ZK(7,1,3,8),ZK(7,1,4,4),ZK(0,0,0,0),ZK(7,1,4,16),		// 71 : hm up pu    lf
	ZK(0,0,0,0),ZK(7,1,3,4),ZK(0,0,0,0),ZK(0,2,5,4),ZK(7,1,3,16),		// 76 :    rg    ed dn
	ZK(7,1,4,8),ZK(0,2,5,2),ZK(7,1,3,2)					// 81 : pd in dl
=======
struct keyScan {
	char key;
	uint8_t row;
	uint8_t mask;
};

keyScan keyTab[40] = {
	{'1',4,1},{'2',4,2},{'3',4,4},{'4',4,8},{'5',4,16},{'6',3,16},{'7',3,8},{'8',3,4},{'9',3,2},{'0',3,1},
	{'q',5,1},{'w',5,2},{'e',5,4},{'r',5,8},{'t',5,16},{'y',2,16},{'u',2,8},{'i',2,4},{'o',2,2},{'p',2,1},
	{'a',6,1},{'s',6,2},{'d',6,4},{'f',6,8},{'g',6,16},{'h',1,16},{'j',1,8},{'k',1,4},{'l',1,2},{'E',1,1},
	{'C',7,1},{'z',7,2},{'x',7,4},{'c',7,8},{'v',7,16},{'b',0,16},{'n',0,8},{'m',0,4},{'S',0,2},{' ',0,1}
>>>>>>> c82a6a983a155192b6238d6c69034689b0c53679
};

// keyboard

struct Keyboard {
	uint8_t map[8];
};

Keyboard* keyCreate() {
	Keyboard* keyb = new Keyboard;
	keyRelease(keyb,0,0);
	return keyb;
}

void keyDestroy(Keyboard* keyb) {
	delete(keyb);
}

void keyPress(Keyboard* keyb,char key1,char key2) {
	for (int i=0; i<40; i++) {
		if ((keyTab[i].key == key1) || (keyTab[i].key == key2)) {
			keyb->map[keyTab[i].row] &= ~keyTab[i].mask;
		}
	}
}

void keyRelease(Keyboard* keyb,char key1,char key2) {
	if ((key1 == 0) && (key2 == 0)) {
		for (int i=0; i<8; i++) keyb->map[i] = 0x1f;
	} else {
		for (int i=0; i<40; i++) {
			if ((keyTab[i].key == key1) || (keyTab[i].key == key2)) {
				keyb->map[keyTab[i].row] |= keyTab[i].mask;
			}
		}
	}
}

uint8_t keyInput(Keyboard* keyb, uint8_t prt) {
	uint8_t res = 0x1f;
	for (int i = 0; i < 8; i++) {
		if (~prt & 0x80) res &= keyb->map[i];
		prt <<= 1;
	}
	return res;
}

// joystick

struct Joystick {
	int type;
	uint8_t state;
};

Joystick* joyCreate() {
	Joystick* joy = new Joystick;
	joy->type = XJ_KEMPSTON;
	joy->state = 0;
	return joy;
}

void joyDestroy(Joystick* joy) {
	delete(joy);
}

void joyPress(Joystick* joy, uint8_t mask) {
	joy->state |= mask;
}

void joyRelease(Joystick* joy,uint8_t mask) {
	joy->state &= ~mask;
}

uint8_t joyInput(Joystick* joy) {
	uint8_t res = 0xff;
	switch (joy->type) {
		case XJ_KEMPSTON:
			res = (joy->state & 0x1f) | 0xe0;	// high 3 bits is set
			break;
	}
	return res;
}

// mouse

Mouse* mouseCreate() {
	Mouse* mou = new Mouse;
	mou->buttons = 0xff;
	mou->xpos = 0;
	mou->ypos = 0;
	mou->enable = true;
	return mou;
}

void mouseDestroy(Mouse* mou) {
	delete(mou);
}
