#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "input.h"

// common matrix

keyScan findKey(keyScan* tab, char key) {
	int idx = 0;
	while (tab[idx].key && (tab[idx].key != key)) {
		idx++;
	}
	return tab[idx];
}

void key_press(Keyboard* kbd, keyScan* tab, int* mtrx, unsigned char ch) {
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

void key_press_seq(Keyboard* kbd, keyScan* tab, int* mtrx, unsigned char* xk) {
	while (*xk != 0x00) {
		key_press(kbd, tab, mtrx, *xk);
		xk++;
	}
}

void key_release(Keyboard* kbd, keyScan* tab, int* mtrx, unsigned char ch) {
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

void key_release_seq(Keyboard* kbd, keyScan* tab, int* mtrx, unsigned char* xk) {
	while (*xk != 0x00) {
		key_release(kbd, tab, mtrx, *xk);
		xk++;
	}
}

void key_trigger(Keyboard* kbd, keyScan* tab, int* mtrx, unsigned char ch) {
	keyScan key = findKey(tab, ch & 0x7f);
	if (ch & 0x80) key.mask |= 0x20;
	mtrx[key.row] ^= key.mask;
	for (int i = 0; i < 16; i++) {
		if (key.mask & (1 << i)) {
			if (mtrx[key.row] & key.mask) {		// is pressed now
				kbd->matrix[key.row][i]++;
			} else {
				kbd->matrix[key.row][i]--;
			}
		}
	}
}

void key_trigger_seq(Keyboard* kbd, keyScan* tab, int* mtrx, unsigned char* xk) {
	while (*xk != 0x00) {
		key_trigger(kbd, tab, mtrx, *xk & 0x7f);
		xk++;
	}
}

// zx spectrum std keyboard

keyScan keyTab[] = {
	{'1',4,1},{'2',4,2},{'3',4,4},{'4',4,8},{'5',4,16},{'6',3,16},{'7',3,8},{'8',3,4},{'9',3,2},{'0',3,1},
	{'q',5,1},{'w',5,2},{'e',5,4},{'r',5,8},{'t',5,16},{'y',2,16},{'u',2,8},{'i',2,4},{'o',2,2},{'p',2,1},
	{'a',6,1},{'s',6,2},{'d',6,4},{'f',6,8},{'g',6,16},{'h',1,16},{'j',1,8},{'k',1,4},{'l',1,2},{'E',1,1},
	{'C',7,1},{'z',7,2},{'x',7,4},{'c',7,8},{'v',7,16},{'b',0,16},{'n',0,8},{'m',0,4},{'S',0,2},{' ',0,1},
	{0,0,0}
};

void kbd_zx_press(Keyboard* kbd, keyEntry* ent) {
	key_press_seq(kbd, keyTab, kbd->map, ent->zxKey);
}

void kbd_zx_release(Keyboard* kbd, keyEntry* ent) {
	key_release_seq(kbd, keyTab, kbd->map, ent->zxKey);
}

int kbdScanZX(Keyboard* kbd, int port) {
	int res = 0x3f;
	for (int i = 0; i < 8; i++) {
		if (!(port & 0x8000))
			res &= kbd->map[i];
		port <<= 1;
	}
	return res;
}

// profi = zx + ext.keys

void kbd_prf_press(Keyboard* kbd, keyEntry* ent) {
	key_press_seq(kbd, keyTab, kbd->extMap, ent->extKey);
	key_press_seq(kbd, keyTab, kbd->map, ent->zxKey);
}

void kbd_prf_release(Keyboard* kbd, keyEntry* ent) {
	key_release_seq(kbd, keyTab, kbd->extMap, ent->extKey);
	key_release_seq(kbd, keyTab, kbd->map, ent->zxKey);
}

int kbdScanProfi(Keyboard* kbd, int port) {
	int res = 0x3f;
	for (int i = 0; i < 8; i++) {
		if (!(port & 0x8000)) {
			res &= kbd->extMap[i];
			res &= (kbd->map[i] | 0x20);
		}
		port <<= 1;
	}
	return res;
}

// msx 1/2

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

void kbd_msx_press(Keyboard* kbd, keyEntry* ent) {
	key_press_seq(kbd, msxKeyTab, kbd->msxMap, ent->msxKey);
}

void kbd_msx_release(Keyboard* kbd, keyEntry* ent) {
	key_release_seq(kbd, msxKeyTab, kbd->msxMap, ent->msxKey);
}

int kbd_msx_read(Keyboard* kbd, int adr) {
	return kbd->msxMap[adr & 0x0f];
}

// specialist

static keyScan spc_keys[] = {
	//f1		f2		f3		f4	f5		f6		f7	f8		f9	f10		lock		clear
	{'!',7,0x800},{'@',7,0x400},{'#',7,0x200},{'$',7,0x100},{'%',7,0x80},{'^',7,0x40},{'&',7,0x20},{'*',7,0x10},{'(',7,0x8},{')',7,0x04},{'O',7,0x02},{'C',7,0x01},
	{';',6,0x800},{'1',6,0x400},{'2',6,0x200},{'3',6,0x100},{'4',6,0x80},{'5',6,0x40},{'6',6,0x20},{'7',6,0x10},{'8',6,0x8},{'9',6,0x04},{'0',6,0x02},{'-',6,0x01},
	{'j',5,0x800},{'c',5,0x400},{'u',5,0x200},{'k',5,0x100},{'e',5,0x80},{'n',5,0x40},{'g',5,0x20},{'[',5,0x10},{']',5,0x8},{'z',5,0x04},{'h',5,0x02},{':',5,0x01},
	{'f',4,0x800},{'y',4,0x400},{'w',4,0x200},{'a',4,0x100},{'p',4,0x80},{'r',4,0x40},{'o',4,0x20},{'l',4,0x10},{'d',4,0x8},{'v',4,0x04},{'/',4,0x02},{'>',4,0x01},
	{'q',3,0x800},{'|',3,0x400},{'s',3,0x200},{'m',3,0x100},{'i',3,0x80},{'t',3,0x40},{'x',3,0x20},{'b',3,0x10},{'A',3,0x8},{'<',3,0x04},{'?',3,0x02},{'B',3,0x01},
	{'X',2,0x800},{'H',2,0x400},{'U',2,0x200},{'D',2,0x100},{'T',2,0x80},{'_',2,0x40},{0x20,2,0x20},{'L',2,0x10},{'V',2,0x8},{'R',2,0x04},{'S',2,0x02},{'E',2,0x01},
	{'P',1,0x800},	/* HP button */
	{0, 0, 0}
};

void kbd_spc_press(Keyboard* kbd, keyEntry* ent) {
	key_press_seq(kbd, spc_keys, kbd->map, ent->zxKey);
}

void kbd_spc_release(Keyboard* kbd, keyEntry* ent) {
	key_release_seq(kbd, spc_keys, kbd->map, ent->zxKey);
}

int kbd_spc_read(Keyboard* kbd, int adr) {
	int row;
	int res = -1;
	for (row = 2; row < 8; row++) {
		if ((kbd->map[row] & adr) != adr) {
			res &= ~(1 << row);
		}
	}
	if (kbd->map[1] != -1)	// HP key
		res ^= 2;
	return res;
}

// commodore
// TODO: make same as ZX: keys sequence in ent->zxKey

struct {
	int code;
	int row;
	int mask;
} c64matrix[] = {
	{XKEY_BSP,0,1},{XKEY_ENTER,0,2},{XKEY_RIGHT,0,4},{XKEY_F7,0,8}, {XKEY_F1,0,16},{XKEY_F3,0,32},{XKEY_F5,0,64},{XKEY_DOWN,0,128},
	{XKEY_3,1,1},{XKEY_W,1,2},{XKEY_A,1,4},{XKEY_4,1,8},{XKEY_Z,1,16},{XKEY_S,1,32},{XKEY_E,1,64},{XKEY_LSHIFT,1,128},
	{XKEY_5,2,1},{XKEY_R,2,2},{XKEY_D,2,4},{XKEY_6,2,8},{XKEY_C,2,16},{XKEY_F,2,32},{XKEY_T,2,64},{XKEY_X,2,128},
	{XKEY_7,3,1},{XKEY_Y,3,2},{XKEY_G,3,4},{XKEY_8,3,8},{XKEY_B,3,16},{XKEY_H,3,32},{XKEY_U,3,64},{XKEY_V,3,128},
	{XKEY_9,4,1},{XKEY_I,4,2},{XKEY_J,4,4},{XKEY_0,4,8},{XKEY_M,4,16},{XKEY_K,4,32},{XKEY_O,4,64},{XKEY_N,4,128},
	{XKEY_EQUAL,5,1},{XKEY_P,5,2},{XKEY_L,5,4},{XKEY_MINUS,5,8},{XKEY_PERIOD,5,16},{XKEY_APOS,5,32},{XKEY_TILDA,5,64},{XKEY_SLASH,5,128},
	{XKEY_RBRACK,6,1},{XKEY_COMMA,5,128},{XKEY_DOTCOM,6,4},{XKEY_HOME,6,8},{XKEY_RSHIFT,6,16},{XKEY_BSLASH,6,32},{XKEY_UP,6,64},{XKEY_BSLASH,6,128},
	{XKEY_1,7,1},{XKEY_LEFT,7,2},{XKEY_LCTRL,7,4},{XKEY_2,7,8},{XKEY_SPACE,7,16},{XKEY_RCTRL,7,32},{XKEY_Q,7,64},{XKEY_ESC,7,128},
	{0,0,0}
};

void kbd_c64_press(Keyboard* kbd, keyEntry* ent) {
	int idx = 0;
	while(c64matrix[idx].code > 0) {
		if (c64matrix[idx].code == ent->key) {
			kbd->msxMap[c64matrix[idx].row] &= ~c64matrix[idx].mask;
		}
		idx++;
	}
}

void kbd_c64_release(Keyboard* kbd, keyEntry* ent) {
	int idx = 0;
	while(c64matrix[idx].code > 0) {
		if (c64matrix[idx].code == ent->key) {
			kbd->msxMap[c64matrix[idx].row] |= c64matrix[idx].mask;
		}
		idx++;
	}
}

int kbd_c64_read(Keyboard* kbd, int adr) {
	int res = 0xff;
	for (int idx = 0; idx < 8; idx++) {
		if ((adr & 1) == 0) {
			res &= kbd->msxMap[idx];
		}
		adr >>= 1;
	}
	return res;
}

// atm2-zx (same as KBD_SPECTRUM)
// atm2-code
// atm2-cpm
// atm2-direct (TODO: read docs one more time)

int kbd_atm2code_rd(Keyboard* kbd, int adr) {
	int res = kbd->keycode;
	kbd->keycode = 0;
	return res;
}

int kbd_atm2cpm_rd(Keyboard* kbd, int adr) {
	int res = -1;
	switch(adr) {
		case 0: res = kbd->keycode;
			kbd->keycode = 0;
			break;
		case 1: res = kbd->flag2;
			break;
		case 2: res = kbd->flag1;
			break;
	}
	return res;
}

void kbd_atm2code_press(Keyboard* kbd, keyEntry* ent) {
	kbd->keycode = ent->atmCode.cpmCode;
	kbd->lastkey = kbd->keycode;
}

void kbd_atm2code_release(Keyboard* kbd, keyEntry* ent) {
	kbd->keycode = 0;
}

// bk0010/11

typedef struct {
	int xkey;
	unsigned char code;
} bkKeyCode;

static bkKeyCode bkey_big_lat[] = {
	{XKEY_Q,'Q'},{XKEY_W,'W'},{XKEY_E,'E'},{XKEY_R,'R'},{XKEY_T,'T'},
	{XKEY_Y,'Y'},{XKEY_U,'U'},{XKEY_I,'I'},{XKEY_O,'O'},{XKEY_P,'P'},
	{XKEY_A,'A'},{XKEY_S,'S'},{XKEY_D,'D'},{XKEY_F,'F'},{XKEY_G,'G'},
	{XKEY_H,'H'},{XKEY_J,'J'},{XKEY_K,'K'},{XKEY_L,'L'},
	{XKEY_Z,'Z'},{XKEY_X,'X'},{XKEY_C,'C'},{XKEY_V,'V'},{XKEY_B,'B'},
	{XKEY_N,'N'},{XKEY_M,'M'},
	{ENDKEY, 0}
};

static bkKeyCode bkey_small_lat[] = {
	{XKEY_Q,'q'},{XKEY_W,'w'},{XKEY_E,'e'},{XKEY_R,'r'},{XKEY_T,'t'},
	{XKEY_Y,'y'},{XKEY_U,'u'},{XKEY_I,'i'},{XKEY_O,'o'},{XKEY_P,'p'},
	{XKEY_A,'a'},{XKEY_S,'s'},{XKEY_D,'d'},{XKEY_F,'f'},{XKEY_G,'g'},
	{XKEY_H,'h'},{XKEY_J,'j'},{XKEY_K,'k'},{XKEY_L,'l'},
	{XKEY_Z,'z'},{XKEY_X,'x'},{XKEY_C,'c'},{XKEY_V,'v'},{XKEY_B,'b'},
	{XKEY_N,'n'},{XKEY_M,'m'},
	{ENDKEY, 0}
};

static bkKeyCode bkey_big_rus[] = {
	{XKEY_Q,0152},{XKEY_W,0143},{XKEY_E,0165},{XKEY_R,0153},{XKEY_T,0145},
	{XKEY_Y,0156},{XKEY_U,0147},{XKEY_I,0173},{XKEY_O,0175},{XKEY_P,0172},
	{XKEY_LBRACK,0150},{XKEY_RBRACK,0177}, /*{XKEY_LBRACE,0150},{XKEY_RBRACE,0177},*/
	{XKEY_A,0146},{XKEY_S,0171},{XKEY_D,0167},{XKEY_F,0141},{XKEY_G,0160},
	{XKEY_H,0162},{XKEY_J,0157},{XKEY_K,0154},{XKEY_L,0144},{XKEY_DOTCOM,0166},{XKEY_APOS,0174},
	{XKEY_Z,0161},{XKEY_X,0176},{XKEY_C,0163},{XKEY_V,0155},{XKEY_B,0111},
	{XKEY_N,0164},{XKEY_M,0170},{XKEY_COMMA,0142},{XKEY_PERIOD,0140},
	{ENDKEY, 0}
};

static bkKeyCode bkey_small_rus[] = {
	{XKEY_Q,0112},{XKEY_W,0103},{XKEY_E,0125},{XKEY_R,0113},{XKEY_T,0105},
	{XKEY_Y,0116},{XKEY_U,0107},{XKEY_I,0133},{XKEY_O,0135},{XKEY_P,0132},
	{XKEY_LBRACK,0110},{XKEY_RBRACK,0137}, /*{XKEY_LBRACE,0110},{XKEY_RBRACE,0137},*/
	{XKEY_A,0106},{XKEY_S,0131},{XKEY_D,0127},{XKEY_F,0101},{XKEY_G,0120},
	{XKEY_H,0122},{XKEY_J,0117},{XKEY_K,0114},{XKEY_L,0104},{XKEY_DOTCOM,0126},{XKEY_APOS,0134},
	{XKEY_Z,0121},{XKEY_X,0136},{XKEY_C,0123},{XKEY_V,0115},{XKEY_B,0111},
	{XKEY_N,0124},{XKEY_M,0130},{XKEY_COMMA,0102},{XKEY_PERIOD,0100},
	{ENDKEY, 0}
};

static bkKeyCode bkey_shift[] = {
	{XKEY_1, '!'},{XKEY_2, '@'},{XKEY_3, '#'},{XKEY_4, '$'},{XKEY_5, '%'},
	{XKEY_6, '^'},{XKEY_7, '&'},{XKEY_8, '*'},{XKEY_9, '('},{XKEY_0, ')'},
	{XKEY_MINUS,'_'},{XKEY_EQUAL,'='},
	{ENDKEY, 0}
};

static bkKeyCode bkey_noshift[] = {
	{XKEY_1,'1'},{XKEY_2,'2'},{XKEY_3,'3'},{XKEY_4,'4'},{XKEY_5,'5'},
	{XKEY_6,'6'},{XKEY_7,'7'},{XKEY_8,'8'},{XKEY_9,'9'},{XKEY_0,'0'},
	{XKEY_MINUS,'-'},{XKEY_EQUAL,'+'},
	{ENDKEY, 0}
};

static bkKeyCode bkeyTab[] = {
	{XKEY_LBRACK,'('},{XKEY_RBRACK,')'},
	{XKEY_DOTCOM, ';'},{XKEY_APOS,'"'},
	{XKEY_COMMA, ','},{XKEY_PERIOD, '.'},{XKEY_BSLASH,'/'},{XKEY_SLASH,'\\'},
	{XKEY_SPACE,' '},{XKEY_ENTER,10},
	{XKEY_BSP,24},
	{XKEY_TAB,13},
	{XKEY_DOWN,27},{XKEY_LEFT,8},{XKEY_RIGHT,25},{XKEY_UP,26},
//	{XKEY_ESC,3},
	{ENDKEY, 0}
};

int bkey_code(bkKeyCode* itm, int xkey) {
	while ((itm->xkey != ENDKEY) && (itm->xkey != xkey))
		itm++;
	return itm->code;		// 0 if ENDKEY
}

void bk_press_keycode(Keyboard* kbd, int code) {
	if (!kbd->kpress) {			// only 1 key can be pressed
		kbd->keycode = code & 0x7f;
		kbd->kpress = 1;
		kbd->drq = 1;
		if (kbd->inten) {		// keyboard interrupt enabled
			kbd->xirq(IRQ_KBD_DATA, kbd->xptr);
			// comp->cpu->intvec = (code & 0x80) ? 0274 : 060;
//			comp->cpu->intvec = comp->keyb->ar2 ? 0274 : 060;
//			comp->cpu->intrq |= PDP_INT_VIRQ;
		}
	}
}

void kbd_bk_press(Keyboard* kbd, keyEntry* xkey) {
	int code = 0;
	switch(xkey->key) {
		case XKEY_LSHIFT:
			kbd->shift = 1;
//			code = 0274;
			break;
		case XKEY_CAPS:
			kbd->caps ^= 1;
			code = kbd->caps ? 0274 : 0273;
			break;
		case XKEY_LCTRL:
			kbd->lang ^= 1;
			code = kbd->lang ? 016 : 017;
			break;
		case XKEY_RCTRL:
			kbd->ar2 = 1;
			break;
	}
	if (code == 0) {
		if (kbd->caps ^ kbd->shift) {
			code = bkey_code(kbd->lang ? bkey_big_rus : bkey_big_lat, xkey->key);
		} else {
			code = bkey_code(kbd->lang ? bkey_small_rus : bkey_small_lat, xkey->key);
		}
	}
	if (code == 0) {
		if (kbd->shift) {
			code = bkey_code(bkey_shift, xkey->key);
		} else {
			code = bkey_code(bkey_noshift, xkey->key);
		}
	}
	if (code == 0)
		code = bkey_code(bkeyTab, xkey->key);
	if (code != 0) {
		bk_press_keycode(kbd, code);
	}
}

void kbd_bk_release(Keyboard* kbd, keyEntry* xkey) {
	switch (xkey->key) {
		case XKEY_LSHIFT:
			kbd->shift = 0;
//			bk_press_keycode(comp, 0273);
			break;
		case XKEY_RCTRL:
			kbd->ar2 = 0;
			break;
	}
	kbd->kpress = 0;		// key released
	kbd->drq = 0;
}

int kbd_bk_rd(Keyboard* kbd, int adr) {
	kbd->drq = 0;
	return kbd->keycode;
}

void kbd_bk_reset(Keyboard* kbd) {
	kbd->kpress = 0;
	kbd->inten = 0;
	kbd->drq = 0;
	kbd->keycode = 0x00;
}

// common codes

void xt_ack(Keyboard* kbd, unsigned long d) {
	kbd->outbuf = d;
	kbd->xirq(IRQ_KBD_ACK, kbd->xptr);
}

int xt_read(Keyboard* kbd) {
	int res;
	if (kbd->outbuf & 0xff) {
		res = kbd->outbuf & 0xff;
		kbd->lastkey = res;
		kbd->outbuf >>= 8;
	} else {
		res = -1;
	}
	return res;
}

// bk (!)

// peka common

unsigned long add_msb(unsigned long code, unsigned long bt) {
	unsigned long msk = 0xff;
	while (code & msk) {
		bt <<= 8;
		msk <<= 8;
	}
	code |= bt;
	return code;
}

void kbd_ibm_press(Keyboard* kbd, keyEntry* kent, int code) {
	if (kbd->lock) return;
	kbd->outbuf = add_msb(kbd->outbuf, code);
	kbd->kent = *kent;
	kbd->per = kbd->kdel;
	kbd->xirq(IRQ_KBD_DATA, kbd->xptr);
}

void kbd_ibm_release(Keyboard* kbd, keyEntry* kent, int code) {
	if (kbd->lock) return;
	kbd->outbuf = add_msb(kbd->outbuf, code);
	kbd->per = 0;
	kbd->xirq(IRQ_KBD_DATA, kbd->xptr);
}

// kbd->pcmode is default mode, set in options; at/ps2 keyboards can switch between modes
void kbd_ibm_res(Keyboard* kbd) {
	switch(kbd->pcmode) {
		case KBD_AT: kbd_set_type(kbd, KBD_PC_AT); break;
		case KBD_XT: kbd_set_type(kbd, KBD_PC_XT); break;
		case KBD_PS2: kbd_set_type(kbd, KBD_PC_PS2); break;
	}
}

// at

void kbd_at_press(Keyboard* kbd, keyEntry* ent) {
	kbd_ibm_press(kbd, ent, ent->atCode);
}

void kbd_at_release(Keyboard* kbd, keyEntry* ent) {
	int code = ent->atCode;
	int res = 0;
	while(code) {
		if (!(code & 0x80))
			res = add_msb(res, 0xf0);
		res = add_msb(res, code & 0xff);
		code >>= 8;
	}
	kbd_ibm_release(kbd, ent, res);
}

// xt

void kbd_xt_press(Keyboard* kbd, keyEntry* ent) {
	kbd_ibm_press(kbd, ent, ent->xtCode);
}

void kbd_xt_release(Keyboard* kbd, keyEntry* ent) {
	int code = ent->xtCode;
	int res = 0;
	while(code) {
		res = add_msb(res, (code & 0xff) | 0x80);
		code >>= 8;
	}
	kbd_ibm_release(kbd, ent, res);
}

// ps2

void kbd_ps2_press(Keyboard* kbd, keyEntry* ent) {
	kbd_ibm_press(kbd, ent, ent->psCode);
}

void kbd_ps2_release(Keyboard* kbd, keyEntry* ent) {
	kbd_ibm_release(kbd, ent, (ent->psCode & 0xff) | 0xff00);
}

// pc98

void kbd_nec_press(Keyboard* kbd, keyEntry* ent) {
	kbd_ibm_press(kbd, ent, ent->necCode);
}

void kbd_nec_release(Keyboard* kbd, keyEntry* ent) {
	kbd_ibm_release(kbd, ent, ent->necCode | 0x80);
}

void kbd_nec_write(Keyboard* kbd, int adr, int val) {
	if (kbd->com < 0) {
		switch(val) {
			case 0x95: kbd->com = val;  xt_ack(kbd, 0xfa); break;
			case 0x96: xt_ack(kbd, 0x85a0); break;
			case 0x99: xt_ack(kbd, 0xfb); break;
			case 0x9c: kbd->com = val; xt_ack(kbd, 0xfa); break;
			case 0x9d: kbd->com = val; xt_ack(kbd, 0xfa); break;		// leds
			case 0x9e: xt_ack(kbd, 0xfa); break;
			case 0x9f: xt_ack(kbd, 0xfa); break;
			default: xt_ack(kbd, 0xfc); break;		// nack
		}
	} else {
		switch (kbd->com) {
			case 0x95: xt_ack(kbd, 0xfa); break;
			case 0x9c: xt_ack(kbd, 0xfa); break;
			case 0x9d: xt_ack(kbd, 0xfa); break;
			default: xt_ack(kbd, 0xfc); break;
		}
		kbd->com = -1;
	}
}

// common for at/xt/ps2/pc98
int kbd_ibm_rd(Keyboard* kbd, int adr) {
	return xt_read(kbd);
}

// keyboard

void kbd_reset(Keyboard* kbd) {
	kbd->com = -1;
	kbd->kdel = 5e8;
	kbd->kper = 5e7;
	if (kbd->core) {
		if (kbd->core->reset) {
			kbd->core->reset(kbd);
		}
	}
}

Keyboard* kbd_create(cbirq cb, void* p) {
	Keyboard* keyb = (Keyboard*)malloc(sizeof(Keyboard));
	memset(keyb, 0x00, sizeof(Keyboard));
	keyb->xirq = cb;
	keyb->xptr = p;
	kbd_reset(keyb);
	return keyb;
}

void kbd_destroy(Keyboard* keyb) {
	free(keyb);
}

// TODO: make this obsolete
void kbdSetMode(Keyboard* kbd, int mode) {
	kbd->mode = mode;
}

// key press/release/trigger

void kbd_press(Keyboard* kbd, keyEntry* ent) {
	if (kbd->core) {
		if (kbd->core->press) {
			kbd->core->press(kbd, ent);
		}
	}
}

void kbd_release(Keyboard* kbd, keyEntry* ent) {
	if (kbd->core) {
		if (kbd->core->release) {
			kbd->core->release(kbd, ent);
		}
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
//	kbd->outbuf = 0;	//kbd->kbuf.pos = 0;
//	kbd->flag = 0;
	if (kbd->per > 0) {
		kbd_release(kbd, &kbd->kent);
//		xt_release(kbd, kbd->kent);
	}
	kbd->per = 0;
}

// trigger is using by kbd-window only

void kbdTrigger(Keyboard* kbd, keyEntry* ent) {
	switch(kbd->mode) {
		case KBD_SPECTRUM:
			key_trigger_seq(kbd, keyTab, kbd->map, ent->zxKey);
			break;
		case KBD_PROFI:
			key_trigger_seq(kbd, keyTab, kbd->extMap, ent->extKey);
			key_trigger_seq(kbd, keyTab, kbd->map, ent->zxKey);
			break;
		case KBD_MSX:
			key_trigger_seq(kbd, msxKeyTab, kbd->msxMap, ent->msxKey);
			break;
	}
	// at/xt ???
}

// at/xt keyboard buffer
// example (at code):
// 0xE0, 0x72 (code 0x72e0) = cursor down pressed
// 0xE0, 0xF0, 0x72 (code 72f0e0) = cursor down released

void xt_add_code(Keyboard* kbd, unsigned long d) {
	kbd->outbuf = add_msb(kbd->outbuf, d);
}

unsigned long xt_get_code(Keyboard* kbd, keyEntry* kent, int rel) {
	unsigned long res = 0;
	int code;
	int mode = kbd->pcmodeovr ? kbd->pcmodeovr : kbd->pcmode;
	if (rel) {
		switch(mode) {
			case KBD_PS2:			// F0,code
				res = 0xf000 | (kent->psCode & 0xff);
				break;
			case KBD_AT:			// insert F0 before each byte with bit7=0
				code = kent->atCode;
				while(code) {
					if (!(code & 0x80))
						res = add_msb(res, 0xf0);
					res = add_msb(res, code & 0xff);
					code >>= 8;
				}
				break;
			case KBD_XT:			// set every b7
				code = kent->xtCode;
				while(code) {
					res = add_msb(res, (code & 0xff) | 0x80);
					code >>= 8;
				}
				break;
			case KBD_PC98:			// 1byte with b7=1
				code = kent->necCode | 0x80;
				break;
		}
	} else {		// press
		switch(mode) {
			case KBD_PS2: res = kent->psCode; break;
			case KBD_AT: res = kent->atCode; break;
			case KBD_XT: res = kent->xtCode; break;
			case KBD_PC98: res = kent->necCode; break;
		}
	}
	return res;
}

void xt_sync(Keyboard* kbd, int ns) {
	if (kbd->per == 0) return;
	kbd->per -= ns;
	if (kbd->per > 0) return;
#if 1
//	kbdRelease(kbd, &kbd->kent);
	kbd_press(kbd, &kbd->kent);
#else
//	unsigned long relcode = xt_get_code(kbd, kbd->kent, 1);	// release
	unsigned long prscode = xt_get_code(kbd, kbd->kent, 0); // press
//	kbd->outbuf = add_msb(kbd->outbuf, relcode);
	kbd->outbuf = add_msb(kbd->outbuf, prscode);
#endif
	kbd->per = kbd->kper;
//	return 1;
}

void xt_press(Keyboard* kbd, keyEntry* kent) {
	if (kbd->lock) return;
	kbd->outbuf = add_msb(kbd->outbuf, xt_get_code(kbd, kent, 0));
	kbd->kent = *kent;
	kbd->per = kbd->kdel;
	kbd->xirq(IRQ_KBD_DATA, kbd->xptr);
	// printf("xt press, buf = %X\n", kbd->outbuf);
}

void xt_release(Keyboard* kbd, keyEntry* kent) {
	if (kbd->lock) return;
	kbd->outbuf = add_msb(kbd->outbuf, xt_get_code(kbd, kent, 1));	// kbd->outbuf = xt_get_code(kbd, kent, 1);
	kbd->per = 0;		// 0 for stopping autorepeat
	kbd->xirq(IRQ_KBD_DATA, kbd->xptr);
}

// ack FA at every byte
void kbd_ibm_wr(Keyboard* kbd, int adr, int d) {
	if (kbd->com < 0) {
		switch (d) {
			case 0xed: xt_ack(kbd, 0xfa); kbd->com = d; break;	// leds
			case 0xee: xt_ack(kbd, 0xee); break;	// echo
			case 0xf0: xt_ack(kbd, 0xfa); kbd->com = d; break;	// get/set scancode
			case 0xf2: xt_ack(kbd, 0x83abfa); break;	// get dev type (no code = at-keyboard)
			case 0xf3: xt_ack(kbd, 0xfa); kbd->com = d; break;	// set repeat rate/delay
			case 0xf4:					// enable sending scancodes
				kbd->lock = 0;
				xt_ack(kbd, 0xfa);
				break;
			case 0xf5:					// disable sending scancodes
				kbd->lock = 1;
				xt_ack(kbd, 0xfa);
				break;
			case 0xf6: kbd_set_type(kbd, KBD_PC_AT); xt_ack(kbd, 0xfa); break;		// set default params
			case 0xf7: xt_ack(kbd, 0xfa); break;		// f7..fd: scanset3 specific
			case 0xf8: xt_ack(kbd, 0xfa); break;
			case 0xf9: xt_ack(kbd, 0xfa); break;
			case 0xfa: xt_ack(kbd, 0xfa); break;
			case 0xfb: xt_ack(kbd, 0xfa); break;
			case 0xfc: xt_ack(kbd, 0xfa); break;
			case 0xfd: xt_ack(kbd, 0xfa); break;
			case 0xfe: xt_ack(kbd, kbd->lastkey & 0xff); break;	// resend last byte
			case 0xff: kbd_reset(kbd); xt_ack(kbd, 0xaafa); break;			// reset & run selftest: ack FA, then AA (passed)
			default: xt_ack(kbd, 0xfe); break;			// unknown: resend (FE)
		}
	} else {
		switch (kbd->com) {
			case 0xed:
				// set leds: b0-scrlck,b1-numlck,b2-caps
				xt_ack(kbd, 0xfa);
				break;
			case 0xf0:
				// set scancode tab: 0-get current, 1..3-scanset 1..3
				switch(d) {
					case 0: switch(kbd->core->id) {
							case KBD_PC_XT: xt_ack(kbd, 0x43fa);
								break;
							case KBD_PC_AT: xt_ack(kbd, 0x41fa);
								break;
							case KBD_PC_PS2: xt_ack(kbd, 0x3ffa);
								break;
						}
						break;
					case 1: kbd_set_type(kbd, KBD_PC_XT);
						xt_ack(kbd, 0xfa);
						break;
					case 2: kbd_set_type(kbd, KBD_PC_AT);
						xt_ack(kbd, 0xfa);
						break;
					case 3: kbd_set_type(kbd, KBD_PC_PS2);
						xt_ack(kbd, 0xfa);
						break;
					default:
						xt_ack(kbd, 0xfe);
						break;
				}
				break;
			case 0xf3:
				kbd->kdel = (((d >> 5) & 3) + 1) * 250e6;	// 1st delay - 250,500,750,1000ms
				kbd->kper = (33 + 7 * (d & 0x1f)) * 1e6;	// repeat period: 33 to 250 ms
				xt_ack(kbd, 0xfa);
				break;
		}
		kbd->com = -1;
	}
}

int kbd_rd(Keyboard* kbd, int port) {
	int res = -1;
	if (kbd->core) {
		if (kbd->core->read) {
			res = kbd->core->read(kbd, port);
		}
	}
	return res;
}

void kbd_wr(Keyboard* kbd, int adr, int val) {
	if (kbd->core) {
		if (kbd->core->write) {
			kbd->core->write(kbd, adr, val);
		}
	}
}

void kbd_sync(Keyboard* kbd, int ns) {
	if (kbd->core) {
		if (kbd->core->sync) {
			kbd->core->sync(kbd, ns);
		}
	}
}

// id,cbReset,cbRead,cbWrite,cbPress,cbRelease,cbSync
xKbdCore kbdTypeTab[] = {
	{KBD_SPECTRUM, NULL, kbdScanZX, NULL, kbd_zx_press, kbd_zx_release, NULL},
	{KBD_PROFI, NULL, kbdScanProfi, NULL, kbd_prf_press, kbd_prf_release, NULL},
	{KBD_ATM2_CODE, NULL, kbd_atm2code_rd, NULL, kbd_atm2code_press, kbd_atm2code_release, NULL},
	{KBD_ATM2_CPM, NULL, kbd_atm2cpm_rd, NULL, kbd_atm2code_press, kbd_atm2code_release, NULL},
	{KBD_ATM2_DIRECT, NULL, NULL, NULL, NULL, NULL, NULL},			// TODO
	{KBD_MSX, NULL, kbd_msx_read, NULL, kbd_msx_press, kbd_msx_release, NULL},
	{KBD_SPCLST, NULL, kbd_spc_read, NULL, kbd_spc_press, kbd_spc_release, NULL},
	{KBD_C64, NULL, kbd_c64_read, NULL, kbd_c64_press, kbd_c64_release, NULL},
	{KBD_BK, kbd_bk_reset, kbd_bk_rd, NULL, kbd_bk_press, kbd_bk_release, NULL},
	{KBD_PC_AT, kbd_ibm_res, kbd_ibm_rd, kbd_ibm_wr, kbd_at_press, kbd_at_release, xt_sync},
	{KBD_PC_XT, kbd_ibm_res, kbd_ibm_rd, kbd_ibm_wr, kbd_xt_press, kbd_xt_press, xt_sync},
	{KBD_PC_PS2, kbd_ibm_res, kbd_ibm_rd, kbd_ibm_wr, kbd_ps2_press, kbd_ps2_release, xt_sync},
	{KBD_NEC98XX, NULL, kbd_ibm_rd, kbd_nec_write, kbd_nec_press, kbd_nec_release, xt_sync},
	{-1, NULL, NULL, NULL, NULL, NULL}
};

void kbd_set_core(Keyboard* kbd, xKbdCore* core) {
	kbd->core = core;
}

void kbd_set_type(Keyboard* kbd, int t) {
	xKbdCore* itm = kbdTypeTab;
	while(itm->id >= 0) {
		if (itm->id == t) {
			kbd->core = itm;
		}
		itm++;
	}
}
