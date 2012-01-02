#include <SDL_keysym.h>
#include <stdint.h>

#include "input.h"

struct ZXKey {
	uint8_t row;
	uint8_t mask;
};

struct XPKey {
	SDLKey key;
	ZXKey zxkey1;
	ZXKey zxkey2;
};

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
};

#endif

// keyboard

struct Keyboard {
	uint8_t map[8];
};

Keyboard* keyCreate() {
	Keyboard* keyb = new Keyboard;
	keyRelease(keyb,0);
	return keyb;
}

void keyDestroy(Keyboard* keyb) {
	delete(keyb);
}

void keyPress(Keyboard* keyb, uint8_t cod) {
	keyb->map[keys[cod][0][0]] &= ~keys[cod][0][1];
	keyb->map[keys[cod][1][0]] &= ~keys[cod][1][1];
}

void keyRelease(Keyboard* keyb, uint8_t cod) {
	if (cod == 0) {
		for (int i=0; i<8; i++) keyb->map[i] = 0x1f;
	} else {
		keyb->map[keys[cod][0][0]] |= keys[cod][0][1];
		keyb->map[keys[cod][1][0]] |= keys[cod][1][1];
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
