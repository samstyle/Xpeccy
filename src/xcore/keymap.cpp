#include <string.h>
#include <math.h>

#include "xcore.h"

// KEYMAPS

keyEntry keyMap[256];			// current keymap (init at start from keyMapInit[]

keyEntry keyMapInit[] = {
	{"1",XKEY_1,{'1',0},{0,0},{'1',0},{'1',0x31},0x16},
	{"2",XKEY_2,{'2',0},{0,0},{'2',0},{'2',0x32},0x1e},
	{"3",XKEY_3,{'3',0},{0,0},{'3',0},{'3',0x33},0x26},
	{"4",XKEY_4,{'4',0},{0,0},{'4',0},{'4',0x34},0x25},
	{"5",XKEY_5,{'5',0},{0,0},{'5',0},{'5',0x35},0x2e},
	{"6",XKEY_6,{'6',0},{0,0},{'6',0},{'6',0x45},0x36},
	{"7",XKEY_7,{'7',0},{0,0},{'7',0},{'7',0x44},0x3d},
	{"8",XKEY_8,{'8',0},{0,0},{'8',0},{'8',0x43},0x3e},
	{"9",XKEY_9,{'9',0},{0,0},{'9',0},{'9',0x42},0x46},
	{"0",XKEY_0,{'0',0},{0,0},{'0',0},{'0',0x41},0x45},
	{"Q",XKEY_Q,{'q',0},{0,0},{'q',0},{'Q',0x21},0x15},
	{"W",XKEY_W,{'w',0},{0,0},{'w',0},{'W',0x22},0x1d},
	{"E",XKEY_E,{'e',0},{0,0},{'e',0},{'E',0x23},0x24},
	{"R",XKEY_R,{'r',0},{0,0},{'r',0},{'R',0x24},0x2d},
	{"T",XKEY_T,{'t',0},{0,0},{'t',0},{'T',0x25},0x2c},
	{"Y",XKEY_Y,{'y',0},{0,0},{'y',0},{'Y',0x55},0x35},
	{"U",XKEY_U,{'u',0},{0,0},{'u',0},{'U',0x54},0x3c},
	{"I",XKEY_I,{'i',0},{0,0},{'i',0},{'I',0x53},0x43},
	{"O",XKEY_O,{'o',0},{0,0},{'o',0},{'O',0x52},0x44},
	{"P",XKEY_P,{'p',0},{0,0},{'p',0},{'P',0x51},0x4d},
	{"A",XKEY_A,{'a',0},{0,0},{'a',0},{'A',0x11},0x1c},
	{"S",XKEY_S,{'s',0},{0,0},{'s',0},{'S',0x12},0x1b},
	{"D",XKEY_D,{'d',0},{0,0},{'d',0},{'D',0x13},0x23},
	{"F",XKEY_F,{'f',0},{0,0},{'f',0},{'F',0x14},0x2b},
	{"G",XKEY_G,{'g',0},{0,0},{'g',0},{'G',0x15},0x34},
	{"H",XKEY_H,{'h',0},{0,0},{'h',0},{'H',0x65},0x33},
	{"J",XKEY_J,{'j',0},{0,0},{'j',0},{'J',0x64},0x3b},
	{"K",XKEY_K,{'k',0},{0,0},{'k',0},{'K',0x63},0x42},
	{"L",XKEY_L,{'l',0},{0,0},{'l',0},{'L',0x62},0x4b},
	{"ENT",XKEY_ENTER,{'E',0},{0,0},{'E',0},{0x0d,0x61},0x5a},
	{"LS",XKEY_LSHIFT,{'C',0},{0,0},{MSXK_SHIFT,0},{0,0x08},0x12},
	{"Z",XKEY_Z,{'z',0},{0,0},{'z',0},{'Z',0x02},0x1a},
	{"X",XKEY_X,{'x',0},{0,0},{'x',0},{'X',0x03},0x22},
	{"C",XKEY_C,{'c',0},{0,0},{'c',0},{'C',0x04},0x21},
	{"V",XKEY_V,{'v',0},{0,0},{'v',0},{'V',0x05},0x2a},
	{"B",XKEY_B,{'b',0},{0,0},{'b',0},{'B',0x75},0x32},
	{"N",XKEY_N,{'n',0},{0,0},{'n',0},{'N',0x74},0x31},
	{"M",XKEY_M,{'m',0},{0,0},{'m',0},{'M',0x73},0x3a},
	{"LC",XKEY_LCTRL,{'S',0},{0,0},{MSXK_CTRL,0},{0,0x88},0x14},
	{"SPC",XKEY_SPACE,{' ',0},{0,0},{' ',0},{0x20,0x71},0x29},

	{"RS",XKEY_RSHIFT,{'C',0},{0,0},{MSXK_SHIFT,0},{0,0x08},0x59},
	{"RC",XKEY_RCTRL,{'S',0},{0,0},{MSXK_CTRL,0},{0,0x88},0x14e0},

	{"LEFT",XKEY_LEFT,{'C','5'},{'C','5'},{MSXK_LEFT,0},{0x72,0x3d},0x6be0},
	{"RIGHT",XKEY_RIGHT,{'C','8'},{'C','8'},{MSXK_RIGHT,0},{0x73,0x4b},0x74e0},
	{"DOWN",XKEY_DOWN,{'C','6'},{'C','6'},{MSXK_DOWN,0},{0x71,0x4d},0x72e0},
	{"UP",XKEY_UP,{'C','7'},{'C','7'},{MSXK_UP,0},{0x70,0x4c},0x75e0},

	{"BSP",XKEY_BSP,{'C','0'},{'C','0'},{MSXK_BSP,0},{0x08,0x49},0x66},
	{"CAPS",XKEY_CAPS,{'C','2'},{'C','2'},{MSXK_CAP,0},{0,0x3a},0x58},
	{"TAB",XKEY_TAB,{'C',' '},{'C','i'},{MSXK_TAB,0},{0x09,0x3b},0x0d},
	{"[",XKEY_LBRACK,{'S','8'},{'S','y'},{'[',0},{'[',0xd5},0x54},
	{"]",XKEY_RBRACK,{'S','9'},{'S','u'},{']',0},{']',0xd4},0x5b},
	{"`",XKEY_TILDA,{'C','S'},{'S','x'},{'`',0},{0x60,0x91},0x0e},
	{"\\",XKEY_SLASH,{'S','C'},{0,0},{'\\',0},{0x2f,0x92},0x5d},

	{"PGDN",XKEY_PGUP,{'C','3'},{'m'|0x80,0},{MSXK_CODE,0},{0x75,0x49},0x7de0},
	{"PGUP",XKEY_PGDN,{'C','4'},{'n'|0x80,0},{MSXK_SEL,0},{0x74,0x51},0x7ae0},

	{"DEL",XKEY_DEL,{'C','9'},{'p'|0x80,0},{MSXK_DEL,0},{0x79,0x49},0x71e0},
	{"INS",XKEY_INS,{0,0},{'o'|0x80,0},{0,MSXK_INS},{0x78,0x84},0x70e0},
	{"HOME",XKEY_HOME,{'S','q'},{'k'|0x80,0},{MSXK_HOME,0},{0x76,0},0x6ce0},
	{"END",XKEY_END,{'S','e'},{'l'|0x80,0},{MSXK_STOP,0},{0x77,0},0x69e0},

	{";",XKEY_DOTCOM,{'S','o'},{'S','o'},{';',0},{0x3b,0xd2},0x4c},
	{"\"",XKEY_QUOTE,{'S','p'},{'S','p'},{0X27,0},{0x27,0xd1},0x52},	// ???
	{"-",XKEY_MINUS,{'S','j'},{'S','j'},{'-',0},{0x2d,0xe4},0x4e},
	{"+",XKEY_PLUS,{'S','k'},{'S','k'},{'+',0},{0x3d,0xe2},0x00},		// ???
	{",",XKEY_COMMA,{'S','n'},{'S','n'},{',',0},{0x2c,0xf4},0x41},
	{".",XKEY_PERIOD,{'S','m'},{'S','m'},{'.',0},{0x2e,0xf3},0x49},
	{"/",XKEY_BSLASH,{'S','c'},{'S','c'},{'/',0},{0x5c,0x85},0x4a},

	{"ESC",XKEY_ESC,{0,0},{'C','1'},{0,MSXK_ESC},{0x1b,0x39},0x76},
	{"F1",XKEY_F1,{0,0},{'a'|0x80,0},{0,MSXK_F1},{0x61,0xb1},0x05},
	{"F2",XKEY_F2,{0,0},{'b'|0x80,0},{0,MSXK_F2},{0x62,0xb2},0x06},
	{"F3",XKEY_F3,{0,0},{'c'|0x80,0},{0,MSXK_F3},{0x63,0xb3},0x04},
	{"F4",XKEY_F4,{0,0},{'d'|0x80,0},{0,MSXK_F4},{0x64,0xb4},0x0C},
	{"F5",XKEY_F5,{0,0},{'e'|0x80,0},{0,MSXK_F5},{0x65,0xb5},0x03},
	{"F6",XKEY_F6,{0,0},{'f'|0x80,0},{0,0},{0x66,0xc5},0x0B},
	{"F7",XKEY_F7,{0,0},{'g'|0x80,0},{0,0},{0x67,0xc4},0x83},
	{"F8",XKEY_F8,{0,0},{'h'|0x80,0},{0,0},{0x68,0xc3},0x0A},
	{"F9",XKEY_F9,{0,0},{'i'|0x80,0},{0,0},{0x69,0xc2},0x01},
	{"F10",XKEY_F10,{0,0},{'j'|0x80,0},{0,0},{0x6a,0xc1},0x09},
	{"F11",XKEY_F11,{0,0},{'S','q'},{0,0},{0x6b,0},0x78},

	{"LA",XKEY_LALT,{0,0},{0,0},{0,0},{0,0},0x11},
	{"RA",XKEY_RALT,{0,0},{0,0},{MSXK_GRAPH,0},{0,0},0x11e0},

	{"",ENDKEY,{0,0},{0,0},{0,0},{0,0},0x00}
};

keyEntry getKeyEntry(signed int qkey) {
	int idx = 0;
	while ((keyMap[idx].key != ENDKEY) && (keyMap[idx].key != qkey)) {
		idx++;
	}
	return keyMap[idx];
}

int getKeyIdByName(const char* name) {
	int idx = 0;
	while ((keyMap[idx].key != ENDKEY) && strcmp(name, keyMap[idx].name)) {
		idx++;
	}
	return keyMap[idx].key;
}

const char* getKeyNameById(int id) {
	int idx = 0;
	while ((keyMap[idx].key != ENDKEY) && (keyMap[idx].key != id)) {
		idx++;
	}
	return keyMap[idx].name;
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
	{Qt::Key_QuoteLeft, 1025, XKEY_TILDA},		// Ё

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
	{Qt::Key_Comma, 0x411, XKEY_COMMA},		// ,
	{Qt::Key_Period, 0x42e, XKEY_PERIOD},		// .
	{Qt::Key_Slash, Qt::Key_Slash, XKEY_BSLASH},		// ?
	{Qt::Key_Apostrophe, 0x44d, XKEY_APOS},	// '

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
	{Qt::Key_F12, Qt::Key_F12, XKEY_F12},

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

	{Qt::Key_SysReq, Qt::Key_SysReq, XKEY_SYSRQ},
	{Qt::Key_Pause, Qt::Key_Pause, XKEY_PAUSE},
	{Qt::Key_ScrollLock, Qt::Key_ScrollLock, XKEY_SCRLCK},
	{Qt::Key_NumLock, Qt::Key_NumLock, XKEY_NUMLCK},

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

int key2qid(int key) {
	int idx = 0;
	while ((ktTab[idx].keyId != key) && (ktTab[idx].keyLat != Qt::Key_unknown)) {
		idx++;
	}
	return ktTab[idx].keyLat;
}
