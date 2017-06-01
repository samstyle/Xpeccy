#include <string.h>
#include <math.h>

#include "xcore.h"

// KEYMAPS

keyEntry keyMap[256];			// current keymap (init at start from keyMapInit[]

keyEntry keyMapInit[] = {
	{"1",XKEY_1,{'1',0},{0,0},{'1',0},0x16},
	{"2",XKEY_2,{'2',0},{0,0},{'2',0},0x1e},
	{"3",XKEY_3,{'3',0},{0,0},{'3',0},0x26},
	{"4",XKEY_4,{'4',0},{0,0},{'4',0},0x25},
	{"5",XKEY_5,{'5',0},{0,0},{'5',0},0x2e},
	{"6",XKEY_6,{'6',0},{0,0},{'6',0},0x36},
	{"7",XKEY_7,{'7',0},{0,0},{'7',0},0x3d},
	{"8",XKEY_8,{'8',0},{0,0},{'8',0},0x3e},
	{"9",XKEY_9,{'9',0},{0,0},{'9',0},0x46},
	{"0",XKEY_0,{'0',0},{0,0},{'0',0},0x45},
	{"Q",XKEY_Q,{'q',0},{0,0},{'q',0},0x15},
	{"W",XKEY_W,{'w',0},{0,0},{'w',0},0x1d},
	{"E",XKEY_E,{'e',0},{0,0},{'e',0},0x24},
	{"R",XKEY_R,{'r',0},{0,0},{'r',0},0x2d},
	{"T",XKEY_T,{'t',0},{0,0},{'t',0},0x2c},
	{"Y",XKEY_Y,{'y',0},{0,0},{'y',0},0x35},
	{"U",XKEY_U,{'u',0},{0,0},{'u',0},0x3c},
	{"I",XKEY_I,{'i',0},{0,0},{'i',0},0x43},
	{"O",XKEY_O,{'o',0},{0,0},{'o',0},0x44},
	{"P",XKEY_P,{'p',0},{0,0},{'p',0},0x4d},
	{"A",XKEY_A,{'a',0},{0,0},{'a',0},0x1c},
	{"S",XKEY_S,{'s',0},{0,0},{'s',0},0x1b},
	{"D",XKEY_D,{'d',0},{0,0},{'d',0},0x23},
	{"F",XKEY_F,{'f',0},{0,0},{'f',0},0x2b},
	{"G",XKEY_G,{'g',0},{0,0},{'g',0},0x34},
	{"H",XKEY_H,{'h',0},{0,0},{'h',0},0x33},
	{"J",XKEY_J,{'j',0},{0,0},{'j',0},0x3b},
	{"K",XKEY_K,{'k',0},{0,0},{'k',0},0x42},
	{"L",XKEY_L,{'l',0},{0,0},{'l',0},0x4b},
	{"ENT",XKEY_ENTER,{'E',0},{0,0},{'E',0},0x5a},
	{"LS",XKEY_LSHIFT,{'C',0},{0,0},{MSXK_SHIFT,0},0x12},
	{"Z",XKEY_Z,{'z',0},{0,0},{'z',0},0x1a},
	{"X",XKEY_X,{'x',0},{0,0},{'x',0},0x22},
	{"C",XKEY_C,{'c',0},{0,0},{'c',0},0x21},
	{"V",XKEY_V,{'v',0},{0,0},{'v',0},0x2a},
	{"B",XKEY_B,{'b',0},{0,0},{'b',0},0x32},
	{"N",XKEY_N,{'n',0},{0,0},{'n',0},0x31},
	{"M",XKEY_M,{'m',0},{0,0},{'m',0},0x3a},
	{"LC",XKEY_LCTRL,{'S',0},{0,0},{MSXK_CTRL,0},0x14},
	{"SPC",XKEY_SPACE,{' ',0},{0,0},{' ',0},0x29},

	{"RS",XKEY_RSHIFT,{'C',0},{0,0},{MSXK_SHIFT,0},0x59},
	{"RC",XKEY_RCTRL,{'S',0},{0,0},{MSXK_CTRL,0},0x14e0},

	{"LEFT",XKEY_LEFT,{'C','5'},{'C','5'},{MSXK_LEFT,0},0x6be0},
	{"RIGHT",XKEY_RIGHT,{'C','8'},{'C','8'},{MSXK_RIGHT,0},0x74e0},
	{"DOWN",XKEY_DOWN,{'C','6'},{'C','6'},{MSXK_DOWN,0},0x72e0},
	{"UP",XKEY_UP,{'C','7'},{'C','7'},{MSXK_UP,0},0x75e0},

	{"BSP",XKEY_BSP,{'C','0'},{'C','0'},{MSXK_BSP,0},0x66},
	{"CAPS",XKEY_CAPS,{'C','2'},{'C','2'},{MSXK_CAP,0},0x58},
	{"TAB",XKEY_TAB,{'C',' '},{'C','i'},{MSXK_TAB,0},0x0d},
	{"[",XKEY_LBRACK,{'S','8'},{'S','y'},{'[',0},0x54},
	{"]",XKEY_RBRACK,{'S','9'},{'S','u'},{']',0},0x5b},
	{"`",XKEY_TILDA,{'C','S'},{'S','x'},{'`',0},0x0e},
	{"\\",XKEY_SLASH,{'S','C'},{0,0},{'\\',0},0x5d},

	{"PGDN",XKEY_PGUP,{'C','3'},{'m'|0x80,0},{MSXK_CODE,0},0x7de0},
	{"PGUP",XKEY_PGDN,{'C','4'},{'n'|0x80,0},{MSXK_SEL,0},0x7ae0},

	{"DEL",XKEY_DEL,{'C','9'},{'p'|0x80,0},{MSXK_DEL,0},0x71e0},
	{"INS",XKEY_INS,{0,0},{'o'|0x80,0},{0,MSXK_INS},0x70e0},
	{"HOME",XKEY_HOME,{'S','q'},{'k'|0x80,0},{MSXK_HOME,0},0x6ce0},
	{"END",XKEY_END,{'S','e'},{'l'|0x80,0},{MSXK_STOP,0},0x69e0},

	{";",XKEY_DOTCOM,{'S','o'},{'S','o'},{';',0},0x4c},
	{"\"",XKEY_QUOTE,{'S','p'},{'S','p'},{0X27,0},0x52},
	{"-",XKEY_MINUS,{'S','j'},{'S','j'},{'-',0},0x4e},
	{"+",XKEY_PLUS,{'S','k'},{'S','k'},{'+',0},0x00},
	{",",XKEY_COMMA,{'S','n'},{'S','n'},{',',0},0x41},
	{".",XKEY_PERIOD,{'S','m'},{'S','m'},{'.',0},0x49},
	{"/",XKEY_BSLASH,{'S','c'},{'S','c'},{'/',0},0x4a},

	{"ESC",XKEY_ESC,{0,0},{'C','1'},{0,MSXK_ESC},0x76},
	{"F1",XKEY_F1,{0,0},{'a'|0x80,0},{0,MSXK_F1},0x05},
	{"F2",XKEY_F2,{0,0},{'b'|0x80,0},{0,MSXK_F2},0x06},
	{"F3",XKEY_F3,{0,0},{'c'|0x80,0},{0,MSXK_F3},0x04},
	{"F4",XKEY_F4,{0,0},{'d'|0x80,0},{0,MSXK_F4},0x0C},
	{"F5",XKEY_F5,{0,0},{'e'|0x80,0},{0,MSXK_F5},0x03},
	{"F6",XKEY_F6,{0,0},{'f'|0x80,0},{0,0},0x0B},
	{"F7",XKEY_F7,{0,0},{'g'|0x80,0},{0,0},0x83},
	{"F8",XKEY_F8,{0,0},{'h'|0x80,0},{0,0},0x0A},
	{"F9",XKEY_F9,{0,0},{'i'|0x80,0},{0,0},0x01},
	{"F10",XKEY_F10,{0,0},{'j'|0x80,0},{0,0},0x09},
	{"F11",XKEY_F11,{0,0},{'S','q'},{0,0},0x78},

	{"LA",XKEY_LALT,{0,0},{0,0},{0,0},0x11},
	{"RA",XKEY_RALT,{0,0},{0,0},{MSXK_GRAPH,0},0x11e0},

	{"",ENDKEY,{0,0},{0,0},{0,0},0x00}
};

keyEntry getKeyEntry(signed int qkey) {
	int idx = 0;
	while ((keyMap[idx].key != ENDKEY) && (keyMap[idx].key != qkey)) {
		idx++;
	}
	return keyMap[idx];
}

void setKey(const char* key,const char key1, const char key2) {
	int idx = 0;
	while (keyMap[idx].key != ENDKEY) {
		if (strcmp(key,keyMap[idx].name) == 0) {
			keyMap[idx].zxKey.key1 = key1;
			keyMap[idx].zxKey.key2 = key2;
		}
		idx++;
	}
}

void initKeyMap() {
	int idx = -1;
	do {
		idx++;
		keyMap[idx] = keyMapInit[idx];
	} while (keyMapInit[idx].key != ENDKEY);
}

// key translation

struct keyTrans {
	int keyLat;		// Qt::Key_Q : qt keycode @ QWERTY layout
	int keyRus;		// 0x419 : qt keycode @ russian 'JZUKEN' layout
	int keyId;		// internal key id = XKEY_*
};

keyTrans ktTab[] = {
	{Qt::Key_0, Qt::Key_0, XKEY_0},
	{Qt::Key_1, Qt::Key_1, XKEY_1},
	{Qt::Key_2, Qt::Key_2, XKEY_2},
	{Qt::Key_3, Qt::Key_3, XKEY_3},
	{Qt::Key_4, Qt::Key_4, XKEY_4},
	{Qt::Key_5, Qt::Key_5, XKEY_5},
	{Qt::Key_6, Qt::Key_6, XKEY_6},
	{Qt::Key_7, Qt::Key_7, XKEY_7},
	{Qt::Key_8, Qt::Key_8, XKEY_8},
	{Qt::Key_9, Qt::Key_9, XKEY_9},
	{Qt::Key_Minus, Qt::Key_Minus, XKEY_MINUS},
	{Qt::Key_Equal, Qt::Key_Equal, XKEY_PLUS},
	{Qt::Key_Backspace, Qt::Key_Backspace, XKEY_BSP},
	{Qt::Key_QuoteLeft, 1025, XKEY_TILDA},

	{Qt::Key_Exclam, Qt::Key_Exclam, XKEY_1},		// !
	{Qt::Key_At, Qt::Key_QuoteDbl, XKEY_2},			// @ (")
	{Qt::Key_NumberSign, 8470, XKEY_3},			// # (№)
	{Qt::Key_Dollar, Qt::Key_Dollar, XKEY_4},		// $
	{Qt::Key_Percent, Qt::Key_Percent, XKEY_5},		// %
	{Qt::Key_AsciiCircum, Qt::Key_AsciiCircum, XKEY_6},	// ^
	{Qt::Key_Ampersand, Qt::Key_Ampersand, XKEY_7},		// &
	{Qt::Key_Asterisk, Qt::Key_Asterisk, XKEY_8},		// *
	{Qt::Key_ParenLeft, Qt::Key_ParenLeft, XKEY_9},		// (
	{Qt::Key_ParenRight, Qt::Key_ParenRight, XKEY_0},	// )

	{Qt::Key_Tab, Qt::Key_Tab, XKEY_TAB},
	{Qt::Key_Q, 1049, XKEY_Q},
	{Qt::Key_W, 1062, XKEY_W},
	{Qt::Key_E, 1059, XKEY_E},
	{Qt::Key_R, 1050, XKEY_R},
	{Qt::Key_T, 1045, XKEY_T},
	{Qt::Key_Y, 1053, XKEY_Y},
	{Qt::Key_U, 1043, XKEY_U},
	{Qt::Key_I, 1064, XKEY_I},
	{Qt::Key_O, 1065, XKEY_O},
	{Qt::Key_P, 1047, XKEY_P},
	{Qt::Key_BracketLeft, 1061, XKEY_LBRACK},		// [
	{Qt::Key_BracketRight, 1066, XKEY_RBRACK},		// ]
//	{Qt::Key_BraceLeft, 1061, XKEY_LBRACE},			// { == Shift + [
//	{Qt::Key_BraceRight, 1066, XKEY_LBRACE},		// } == Shift + ]
	{Qt::Key_Backslash, Qt::Key_Backslash, XKEY_SLASH},	// |

	{Qt::Key_CapsLock, Qt::Key_CapsLock, XKEY_CAPS},
	{Qt::Key_A, 1060, XKEY_A},
	{Qt::Key_S, 1067, XKEY_S},
	{Qt::Key_D, 1042, XKEY_D},
	{Qt::Key_F, 1040, XKEY_F},
	{Qt::Key_G, 1055, XKEY_G},
	{Qt::Key_H, 1056, XKEY_H},
	{Qt::Key_J, 1054, XKEY_J},
	{Qt::Key_K, 1051, XKEY_K},
	{Qt::Key_L, 1044, XKEY_L},
	{Qt::Key_Semicolon, 1046, XKEY_DOTCOM},
	{Qt::Key_Apostrophe, 1069, XKEY_QUOTE},
	{Qt::Key_Return, Qt::Key_Enter, XKEY_ENTER},

	{Qt::Key_Shift, Qt::Key_Shift, XKEY_LSHIFT},
	{Qt::Key_Z, 1071, XKEY_Z},
	{Qt::Key_X, 1063, XKEY_X},
	{Qt::Key_C, 1057, XKEY_C},
	{Qt::Key_V, 1052, XKEY_V},
	{Qt::Key_B, 1048, XKEY_B},
	{Qt::Key_N, 1058, XKEY_N},
	{Qt::Key_M, 1068, XKEY_M},
	{Qt::Key_Period, 1041, XKEY_PERIOD},
	{Qt::Key_Comma, Qt::Key_Comma, XKEY_COMMA},
	{Qt::Key_Slash, Qt::Key_Slash, XKEY_BSLASH},		// ?

	{Qt::Key_Control, Qt::Key_Control, XKEY_LCTRL},
	{Qt::Key_Alt, Qt::Key_Alt, XKEY_LALT},
	{Qt::Key_Space, Qt::Key_Space, XKEY_SPACE},

	{Qt::Key_Escape, Qt::Key_Escape, XKEY_ESC},
	{Qt::Key_F1, Qt::Key_F1, XKEY_F1},
	{Qt::Key_F2, Qt::Key_F2, XKEY_F2},
	{Qt::Key_F3, Qt::Key_F3, XKEY_F3},
	{Qt::Key_F4, Qt::Key_F4, XKEY_F4},
	{Qt::Key_F5, Qt::Key_F5, XKEY_F5},
	{Qt::Key_F6, Qt::Key_F6, XKEY_F6},
	{Qt::Key_F7, Qt::Key_F7, XKEY_F7},
	{Qt::Key_F8, Qt::Key_F8, XKEY_F8},
	{Qt::Key_F9, Qt::Key_F9, XKEY_F9},
	{Qt::Key_F10, Qt::Key_F10, XKEY_F10},
	{Qt::Key_F11, Qt::Key_F11, XKEY_F11},
	// {Qt::Key_F12, Qt::Key_F12, XKEY_F12},

	{Qt::Key_Up, Qt::Key_Up, XKEY_UP},
	{Qt::Key_Down, Qt::Key_Down, XKEY_DOWN},
	{Qt::Key_Left, Qt::Key_Left, XKEY_LEFT},
	{Qt::Key_Right, Qt::Key_Right, XKEY_RIGHT},

	{Qt::Key_Home, Qt::Key_Home, XKEY_HOME},
	{Qt::Key_End, Qt::Key_End, XKEY_END},
	{Qt::Key_Insert, Qt::Key_Insert, XKEY_INS},
	{Qt::Key_Delete, Qt::Key_Delete, XKEY_DEL},
	{Qt::Key_PageUp, Qt::Key_PageUp, XKEY_PGUP},
	{Qt::Key_PageDown, Qt::Key_PageDown, XKEY_PGDN},

	// TODO: complete this table
	{Qt::Key_unknown, Qt::Key_unknown, ENDKEY}
};

int qKey2id(int qkey) {
	int idx = 0;
	while ((ktTab[idx].keyLat != qkey) && (ktTab[idx].keyRus != qkey) && (ktTab[idx].keyLat != Qt::Key_unknown)) {
		idx++;
	}
	return ktTab[idx].keyId;
}

int sign(int val) {
	if (val < 0) return -1;
	if (val > 0) return 1;
	return 0;
}

void mapJoystick(Computer* comp, int type, int num, int state) {
	xJoyMap xjm;
	keyEntry kent;
	foreach(xjm, conf.joy.map) {
		if ((type == xjm.type) && (num == xjm.num) && (sign(state) == sign(xjm.state))) {
			switch(xjm.dev) {
				case JMAP_KEY:
					kent = getKeyEntry(xjm.key);
					if (state) {
						keyPress(comp->keyb, kent.zxKey, 0);
					} else {
						keyRelease(comp->keyb, kent.zxKey, 0);
					}
					break;
				case JMAP_JOY:
					if (state) {
						// press
						comp->joy->state |= xjm.key;
					} else {
						// release
						comp->joy->state &= ~xjm.key;
					}
					break;
			}
		}
	}
}
