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
// 0xE0, 0xF0, 0x72 (code 72f0e0) = cursor down released

unsigned long add_msb(unsigned long code, unsigned long bt) {
	unsigned long msk = 0xff;
	while (code & msk) {
		bt <<= 8;
		msk <<= 8;
	}
	code |= bt;
	return code;
}

void xt_add_code(Keyboard* kbd, unsigned long d) {
	kbd->outbuf = add_msb(kbd->outbuf, d);
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
//	unsigned long relcode = xt_get_code(kbd, kbd->kent, 1);	// release
	unsigned long prscode = xt_get_code(kbd, kbd->kent, 0); // press
//	kbd->outbuf = add_msb(kbd->outbuf, relcode);
	kbd->outbuf = add_msb(kbd->outbuf, prscode);
	kbd->per = kbd->kper;
	return 1;
}

void xt_press(Keyboard* kbd, keyEntry kent) {
	if (kbd->lock) return;
	kbd->outbuf = add_msb(kbd->outbuf, xt_get_code(kbd, kent, 0));
	kbd->kent = kent;
	kbd->per = kbd->kdel;
	kbd->xirq(IRQ_KBD_DATA, kbd->xptr);
	// printf("xt press, buf = %X\n", kbd->outbuf);
}

void xt_release(Keyboard* kbd, keyEntry kent) {
	if (kbd->lock) return;
	kbd->outbuf = add_msb(kbd->outbuf, xt_get_code(kbd, kent, 1));	// kbd->outbuf = xt_get_code(kbd, kent, 1);
	kbd->per = 0;		// 0 for stopping autorepeat
	kbd->xirq(IRQ_KBD_DATA, kbd->xptr);
}

int xt_read(Keyboard* kbd) {
	int res;
	if (kbd->outbuf & 0xff) {
		res = kbd->outbuf & 0xff;
		kbd->lastkey = res & 0xff;
		kbd->outbuf >>= 8;
	} else {
		res = -1;
	}
	return res;
}

void xt_ack(Keyboard* kbd, unsigned long d) {
	kbd->outbuf = d;
	kbd->xirq(IRQ_KBD_DATA, kbd->xptr);
}

// ack FA at every byte

void kbd_wr(Keyboard* kbd, int d) {
	if (kbd->com < 0) {
		switch (d) {
			case 0xed: xt_ack(kbd, 0xfa); kbd->com = d | 0x100; break;	// leds
			case 0xee: xt_ack(kbd, 0xee); break;	// echo
			case 0xf0: xt_ack(kbd, 0xfa); kbd->com = d | 0x100; break;	// get/set scancode
			case 0xf2: xt_ack(kbd, 0x83abfa); break;	// get dev type (no code = at-keyboard)
			case 0xf3: xt_ack(kbd, 0xfa); kbd->com = d | 0x100; break;	// set repeat rate/delay
			case 0xf4:					// enable sending scancodes
				kbd->lock = 0;
				xt_ack(kbd, 0xfa);
				break;
			case 0xf5:					// disable sending scancodes
				kbd->lock = 1;
				xt_ack(kbd, 0xfa);
				break;
			case 0xf6: xt_ack(kbd, 0xfa); break;		// set default params
			case 0xf7: xt_ack(kbd, 0xfa); break;		// f7..fd: scanset3 specific
			case 0xf8: xt_ack(kbd, 0xfa); break;
			case 0xf9: xt_ack(kbd, 0xfa); break;
			case 0xfa: xt_ack(kbd, 0xfa); break;
			case 0xfb: xt_ack(kbd, 0xfa); break;
			case 0xfc: xt_ack(kbd, 0xfa); break;
			case 0xfd: xt_ack(kbd, 0xfa); break;
			case 0xfe: xt_ack(kbd, kbd->lastkey & 0xff); break;	// resend last byte
			case 0xff:						// reset & run selftest
				xt_ack(kbd, 0xfa);
				break;
		}
	} else {
		switch (kbd->com) {
			case 0x1ed:
				// set leds: b0-scrlck,b1-numlck,b2-caps
				xt_ack(kbd, 0xfa);
				break;
			case 0x1f0:
				// set scancode tab: 0-get current, 1..3-scanset 1..3
				if (d == 0) {
					xt_ack(kbd, 0x02fa);	// (kbd->scanset << 8) | fa)
				} else {
					xt_ack(kbd, 0xfa);
				}
				break;
			case 0x1f3:
				kbd->kdel = (((d >> 5) & 3) + 1) * 250e6;	// 1st delay - 250,500,750,1000ms
				kbd->kper = (33 + 7 * (d & 0x1f)) * 1e6;	// repeat period: 33 to 250 ms
				xt_ack(kbd, 0xfa);
				break;
		}
		kbd->com = -1;
	}
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
	mou->sensitivity = 1.0f;
	mou->pcmode = MOUSE_SERIAL;
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

int mouseGetX(Mouse* mou) {return mou->xpos * mou->sensitivity;}
int mouseGetY(Mouse* mou) {return mou->ypos * mou->sensitivity;}

// interrupt packet (ps/2 mouse):
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
	if (mouse->queueSize > 0) return;
	if (mouse->lock) return;
	switch (mouse->pcmode) {
		case MOUSE_SERIAL:			// microsoft serial mouse
			mouse->outbuf = 0x40;
			if (mouse->lmb) mouse->outbuf |= 0x20;
			if (mouse->rmb) mouse->outbuf |= 0x10;
			mouse->outbuf |= ((mouse->ydelta & 0xc0) >> 4);
			mouse->outbuf |= ((mouse->xdelta & 0xc0) >> 6);
			mouse->outbuf |= ((mouse->xdelta & 0x3f) << 8);
			mouse->outbuf |= ((mouse->ydelta & 0x3f) << 16);
			mouse->queueSize = 3;
			mouse->xirq(IRQ_MOUSE_DATA, mouse->xptr);
			break;
		case MOUSE_PS2:
			// ps/2 mouse
			mouse->outbuf = (abs(mouse->ydelta) & 0xff) << 8;
			mouse->outbuf |= ((abs(mouse->xdelta) & 0xff) << 16);
			if (mouse->lmb) mouse->outbuf |= (1 << 0);
			if (mouse->rmb) mouse->outbuf |= (1 << 1);
			// b2: mmb
			mouse->outbuf |= (1 << 3);
			if (mouse->xdelta < 0) mouse->outbuf |= (1 << 4);
			if (mouse->ydelta < 0) mouse->outbuf |= (1 << 5);
			// b6,7: x,y overflow
			mouse->queueSize = 3;
			mouse->xirq(IRQ_MOUSE_DATA, mouse->xptr);
			break;
		default:
			break;
	}
	mouse->xdelta = 0;
	mouse->ydelta = 0;
}

int mouse_rd(Mouse* mouse) {
	int res = -1;
	if (mouse->queueSize > 0) {
		res = mouse->outbuf & 0xff;
		mouse->data = res;
		mouse->outbuf >>= 8;
		mouse->queueSize--;
	}
	return res;
}

void mouse_ack(Mouse* mou, int d) {
	if (mou->lock) return;
	mou->outbuf = d;
	mou->queueSize = 1;
	mou->xirq(IRQ_MOUSE_DATA, mou->xptr);
}

void mouse_wr(Mouse* mou, int d) {
	if (mou->com < 0) {
		switch (d) {
			case 0xe6: mouse_ack(mou, 0xfa); break;	// set scale 1:1
			case 0xe7: mouse_ack(mou, 0xfa); break;	// set scale 2:1
			case 0xe8: mouse_ack(mou, 0xfa);
				mou->com = 0x1e8;
				break;		// +data: set resolution
			case 0xe9: break;				// status request
			case 0xea: mouse_ack(mou, 0xfa); break;	// set stream mode
			case 0xeb: break;				// read data
			case 0xec: mouse_ack(mou, 0xfa); break;	// reset wrap mode
			case 0xee: mouse_ack(mou, 0xfa); break;	// set wrap mode
			case 0xf0: mouse_ack(mou, 0xfa); break;	// set remote mode
			case 0xf2: mouse_ack(mou, 0x00); break;	// get device id (00 - standard ps/2 mouse)
			case 0xf3: mouse_ack(mou, 0xfa);
				mou->com = 0x1f3;
				break;		// set sample rate
			case 0xf4: mou->lock = 0;
				mouse_ack(mou, 0xfa);
				break;		// enable data reporting
			case 0xf5: mou->lock = 1;
				mouse_ack(mou, 0xfa);
				break;		// disable data reporting
			case 0xf6: mouse_ack(mou, 0xfa); break;	// set defaults
			case 0xfe: mouse_ack(mou, mou->data); break;	// resend
			case 0xff: mouse_ack(mou, 0x00); break;	// reset & send id
		}
	} else {
		switch (mou->com) {
			case 0x1e8:
				break;				// d - resolution
			case 0x1f3:
				break;				// d - sample rate
		}
		mou->com = -1;
	}
//	printf("%s : %X\n",__FUNCTION__,d);
}
