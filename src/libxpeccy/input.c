#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "input.h"

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

Keyboard* keyCreate(cbirq cb, void* p) {
	Keyboard* keyb = (Keyboard*)malloc(sizeof(Keyboard));
	memset(keyb, 0x00, sizeof(Keyboard));
	keyb->pcmode = KBD_AT;
	keyb->kdel = 5e8;
	keyb->kper = 5e7;
	keyb->xirq = cb;
	keyb->xptr = p;
	return keyb;
}

void keyDestroy(Keyboard* keyb) {
	free(keyb);
}

void kbdSetMode(Keyboard* kbd, int mode) {
	kbd->mode = mode;
}

// key press/release/trigger

void kbd_press_key(Keyboard* kbd, keyScan* tab, int* mtrx, unsigned char ch) {
	if (!ch) return;
//	printf("kbd_press_key %c\n", ch);
	keyScan key = findKey(tab, ch & 0x7f);
	key.row &= 0x0f;
	kbd->row = key.row;
	kbd->mask = key.mask;
	mtrx[key.row] &= ~key.mask;
	// if (key.mask) printf("row %i : %X\n",key.row, mtrx[key.row]);
	if (ch & 0x80)
		mtrx[key.row] &= ~0x20;
	// update matrix
	for (int i = 0; i < 16; i++) {
		if (key.mask & (1 << i))
			kbd->matrix[key.row][i]++;
	}
}

void kbd_press(Keyboard* kbd, keyScan* tab, int* mtrx, unsigned char* xk) {
	int pos = 0;
	while (xk[pos] != 0x00) {
		kbd_press_key(kbd, tab, mtrx, xk[pos]);
		pos++;
	}
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
	}
}

void kbd_release_key(Keyboard* kbd, keyScan* tab, int* mtrx, unsigned char ch) {
//	if (ch) printf("kbd_release_key %c\n", ch);
	keyScan key = findKey(tab, ch & 0x7f);
	key.row &= 0x0f;
	for (int i = 0; i < 16; i++) {
		if (key.mask & (1 << i)) {
			if (kbd->matrix[key.row][i] > 0)
				kbd->matrix[key.row][i]--;
			if (kbd->matrix[key.row][i] == 0) {
				mtrx[key.row] |= key.mask;
			}
		}
	}
}

void kbd_release(Keyboard* kbd, keyScan* tab, int* mtrx, unsigned char* xk) {
	int pos = 0;
	while (xk[pos] != 0x00) {
		kbd_release_key(kbd, tab, mtrx, xk[pos]);
		pos++;
	}
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
	}
}

void kbdReleaseAll(Keyboard* kbd) {
	int i;
	for (i = 0; i < 8; i++) {
		kbd->map[i] = -1;
		kbd->extMap[i] = -1;
		kbd->msxMap[i] = -1;
		kbd->msxMap[i + 8] = -1;
	}
	for (i = 0; i < 16 * 8; i++) {
		kbd->matrix[(i >> 3) & 15][i & 7] = 0;
	}
	kbd->keycode = 0;
	kbd->lastkey = 0;
	kbd->outbuf = 0;	//kbd->kbuf.pos = 0;
	kbd->flag = 0;
	if (kbd->per > 0) {
		xt_release(kbd, kbd->kent);
	}
	kbd->per = 0;
}

void kbd_trigger(keyScan* tab, int* mtrx, unsigned char* xk) {
	keyScan key;
	int pos = 0;
	while (xk[pos] != 0x00) {
		key = findKey(tab, xk[pos] & 0x7f);
		if (xk[pos] & 0x80)
			key.mask |= 0x20;
		mtrx[key.row] ^= key.mask;
		pos++;
	}
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
// example (at code):
// 0xE0, 0x72 (code 0x72e0) = cursor down pressed
// 0xE0, 0xF0, 0x72 (code 72e0) = cursor down released

unsigned long add_msb(unsigned long code, unsigned long bt) {
	unsigned long msk = 0xff;
	while (code & msk) {
		bt <<= 8;
		msk <<= 8;
	}
	code |= bt;
	return code;
}

unsigned long xt_get_code(Keyboard* kbd, keyEntry kent, int rel) {
	unsigned long res = 0;
	int code;
	if (rel) {
		switch(kbd->pcmode) {
			case KBD_AT:			// insert F0 before each byte with bit7=0
				code = kent.atCode;
				while(code) {
					if (!(code & 0x80))
						res = add_msb(res, 0xf0);
					res = add_msb(res, code & 0xff);
					code >>= 8;
				}
				break;
			case KBD_XT:			// set every b7
				code = kent.xtCode;
				while(code) {
					res = add_msb(res, (code & 0xff) | 0x80);
					code >>= 8;
				}
				break;
		}
	} else {		// press
		switch(kbd->pcmode) {
			case KBD_AT: res = kent.atCode; break;
			case KBD_XT: res = kent.xtCode; break;
		}
	}
	return res;
}

int xt_sync(Keyboard* kbd, int ns) {
	if (kbd->per == 0) return 0;
	kbd->per -= ns;
	if (kbd->per > 0) return 0;
	unsigned long relcode = xt_get_code(kbd, kbd->kent, 1);	// release
	unsigned long prscode = xt_get_code(kbd, kbd->kent, 0); // press
	kbd->outbuf = add_msb(kbd->outbuf, relcode);
	kbd->outbuf = add_msb(kbd->outbuf, prscode);
	kbd->per = kbd->kper;
	return 1;
}

void xt_press(Keyboard* kbd, keyEntry kent) {
	if (kbd->lock) return;
	kbd->outbuf = add_msb(kbd->outbuf, xt_get_code(kbd, kent, 0));
	kbd->kent = kent;
	kbd->per = kbd->kdel;
	// printf("xt press, buf = %X\n", kbd->outbuf);
}

void xt_release(Keyboard* kbd, keyEntry kent) {
	if (kbd->lock) return;
	kbd->outbuf = add_msb(kbd->outbuf, xt_get_code(kbd, kent, 1));	// kbd->outbuf = xt_get_code(kbd, kent, 1);
	kbd->per = kbd->kdel;
}

int xt_read(Keyboard* kbd) {
	int res = kbd->outbuf & 0xff;
	kbd->outbuf >>= 8;
	return res;
}

unsigned char kbdScanZX(Keyboard* kbd, int port) {
	unsigned char res = 0x3f;
	for (int i = 0; i < 8; i++) {
		if (!(port & 0x8000))
			res &= kbd->map[i];
		port <<= 1;
	}
	return res;
}

unsigned char kbdScanProfi(Keyboard* kbd, int port) {
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

unsigned char kbdRead(Keyboard* kbd, int port) {
	unsigned char res = 0xff;

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

void joyPress(Joystick* joy, int mask) {
	joy->state |= mask;
}

void joyRelease(Joystick* joy, int mask) {
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

Mouse* mouseCreate(cbirq cb, void* p) {
	Mouse* mou = (Mouse*)malloc(sizeof(Mouse));
	memset(mou,0x00,sizeof(Mouse));
	mou->xirq = cb;
	mou->xptr = p;
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

// interrupt packet:
// byte1:	b7:Y overflow
//		b6:X overflow
//		b5:Y delta sign
//		b4:X delta sign
//		b3: 1
//		b2: mmb
//		b1: rmb
//		b0: lmb
// byte2	abs X delta
// byte3	abs Y delta

void mouse_interrupt(Mouse* mouse) {
	mouse->outbuf = (abs(mouse->ydelta) & 0xff) << 8;
	mouse->outbuf |= ((abs(mouse->xdelta) & 0xff) << 16);
	if (mouse->lmb) mouse->outbuf |= (1 << 0);
	if (mouse->rmb) mouse->outbuf |= (1 << 1);
	// b2: mmb
	mouse->outbuf |= (1 << 3);
	if (mouse->xdelta < 0) mouse->outbuf |= (1 << 4);
	if (mouse->ydelta < 0) mouse->outbuf |= (1 << 5);
	// b6,7: x,y overflow
	mouse->xdelta = 0;
	mouse->ydelta = 0;
	mouse->intrq = 1;
}
