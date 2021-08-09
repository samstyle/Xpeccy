#include <fstream>

#include <string.h>
#include <math.h>

#include "xcore.h"

// KEYMAPS

static keyEntry keyMap[256];			// current keymap (init at start from keyMapInit[]

static keyEntry keyMapInit[] = {
	{"1",XKEY_1,{'1',0},{0,0},{'1',0},{'1',0x31},0x16,0},
	{"2",XKEY_2,{'2',0},{0,0},{'2',0},{'2',0x32},0x1e,0},
	{"3",XKEY_3,{'3',0},{0,0},{'3',0},{'3',0x33},0x26,0},
	{"4",XKEY_4,{'4',0},{0,0},{'4',0},{'4',0x34},0x25,0},
	{"5",XKEY_5,{'5',0},{0,0},{'5',0},{'5',0x35},0x2e,0},
	{"6",XKEY_6,{'6',0},{0,0},{'6',0},{'6',0x45},0x36,0},
	{"7",XKEY_7,{'7',0},{0,0},{'7',0},{'7',0x44},0x3d,0},
	{"8",XKEY_8,{'8',0},{0,0},{'8',0},{'8',0x43},0x3e,0},
	{"9",XKEY_9,{'9',0},{0,0},{'9',0},{'9',0x42},0x46,0},
	{"0",XKEY_0,{'0',0},{0,0},{'0',0},{'0',0x41},0x45,0},
	{"Q",XKEY_Q,{'q',0},{0,0},{'q',0},{'Q',0x21},0x15,0},
	{"W",XKEY_W,{'w',0},{0,0},{'w',0},{'W',0x22},0x1d,0},
	{"E",XKEY_E,{'e',0},{0,0},{'e',0},{'E',0x23},0x24,0},
	{"R",XKEY_R,{'r',0},{0,0},{'r',0},{'R',0x24},0x2d,0},
	{"T",XKEY_T,{'t',0},{0,0},{'t',0},{'T',0x25},0x2c,0},
	{"Y",XKEY_Y,{'y',0},{0,0},{'y',0},{'Y',0x55},0x35,0},
	{"U",XKEY_U,{'u',0},{0,0},{'u',0},{'U',0x54},0x3c,0},
	{"I",XKEY_I,{'i',0},{0,0},{'i',0},{'I',0x53},0x43,0},
	{"O",XKEY_O,{'o',0},{0,0},{'o',0},{'O',0x52},0x44,0},
	{"P",XKEY_P,{'p',0},{0,0},{'p',0},{'P',0x51},0x4d,0},
	{"A",XKEY_A,{'a',0},{0,0},{'a',0},{'A',0x11},0x1c,0},
	{"S",XKEY_S,{'s',0},{0,0},{'s',0},{'S',0x12},0x1b,0},
	{"D",XKEY_D,{'d',0},{0,0},{'d',0},{'D',0x13},0x23,0},
	{"F",XKEY_F,{'f',0},{0,0},{'f',0},{'F',0x14},0x2b,0},
	{"G",XKEY_G,{'g',0},{0,0},{'g',0},{'G',0x15},0x34,0},
	{"H",XKEY_H,{'h',0},{0,0},{'h',0},{'H',0x65},0x33,0},
	{"J",XKEY_J,{'j',0},{0,0},{'j',0},{'J',0x64},0x3b,0},
	{"K",XKEY_K,{'k',0},{0,0},{'k',0},{'K',0x63},0x42,0},
	{"L",XKEY_L,{'l',0},{0,0},{'l',0},{'L',0x62},0x4b,0},
	{"ENT",XKEY_ENTER,{'E',0},{0,0},{'E',0},{0x0d,0x61},0x5a,0},
	{"LS",XKEY_LSHIFT,{'C',0},{0,0},{MSXK_SHIFT,0},{0,0x08},0x12,0},
	{"Z",XKEY_Z,{'z',0},{0,0},{'z',0},{'Z',0x02},0x1a,0},
	{"X",XKEY_X,{'x',0},{0,0},{'x',0},{'X',0x03},0x22,0},
	{"C",XKEY_C,{'c',0},{0,0},{'c',0},{'C',0x04},0x21,0},
	{"V",XKEY_V,{'v',0},{0,0},{'v',0},{'V',0x05},0x2a,0},
	{"B",XKEY_B,{'b',0},{0,0},{'b',0},{'B',0x75},0x32,0},
	{"N",XKEY_N,{'n',0},{0,0},{'n',0},{'N',0x74},0x31,0},
	{"M",XKEY_M,{'m',0},{0,0},{'m',0},{'M',0x73},0x3a,0},
	{"LC",XKEY_LCTRL,{'S',0},{0,0},{MSXK_CTRL,0},{0,0x80},0x14,0},
	{"SPC",XKEY_SPACE,{' ',0},{0,0},{' ',0},{0x20,0x71},0x29,0},

	{"RS",XKEY_RSHIFT,{0,0},{0,0},{0,0},{0,0},0,0},
	{"RC",XKEY_RCTRL,{0,0},{0,0},{0,0},{0,0},0,0},

	{"LEFT",XKEY_LEFT,{'C','5'},{'C','5'},{MSXK_LEFT,0},{0x72,0x3d},0x6be0,0},
	{"RIGHT",XKEY_RIGHT,{'C','8'},{'C','8'},{MSXK_RIGHT,0},{0x73,0x4b},0x74e0,0},
	{"DOWN",XKEY_DOWN,{'C','6'},{'C','6'},{MSXK_DOWN,0},{0x71,0x4d},0x72e0,0},
	{"UP",XKEY_UP,{'C','7'},{'C','7'},{MSXK_UP,0},{0x70,0x4c},0x75e0,0},

	{"BSP",XKEY_BSP,{'C','0'},{'C','0'},{MSXK_BSP,0},{0x08,0x49},0x66,0},
	{"CAPS",XKEY_CAPS,{'C','2'},{'C','2'},{MSXK_CAP,0},{0,0x3a},0x58,0},
	{"TAB",XKEY_TAB,{'C',' '},{'C','i'},{MSXK_TAB,0},{0x09,0x3b},0x0d,0},
	{"[",XKEY_LBRACK,{'S','8'},{'S','y'},{'[',0},{'[',0xd5},0x54,0},
	{"]",XKEY_RBRACK,{'S','9'},{'S','u'},{']',0},{']',0xd4},0x5b,0},
	{"`",XKEY_TILDA,{'C','S'},{'S','x'},{'`',0},{0x60,0x91},0x0e,0},
	{"\\",XKEY_SLASH,{'S','C'},{0,0},{'\\',0},{0x2f,0x92},0x5d,0},

	{"PGDN",XKEY_PGDN,{'C','3'},{'n'|0x80,0},{MSXK_SEL,0},{0x75,0x51},0x7ae0,0},
	{"PGUP",XKEY_PGUP,{'C','4'},{'m'|0x80,0},{MSXK_CODE,0},{0x74,0x49},0x7de0,0},

	{"DEL",XKEY_DEL,{'C','9'},{'p'|0x80,0},{MSXK_DEL,0},{0x79,0x49},0x71e0,0},
	{"INS",XKEY_INS,{0,0},{'o'|0x80,0},{0,MSXK_INS},{0x78,0x84},0x70e0,0},
	{"HOME",XKEY_HOME,{'S','q'},{'k'|0x80,0},{MSXK_HOME,0},{0x76,0},0x6ce0,0},
	{"END",XKEY_END,{'S','e'},{'l'|0x80,0},{MSXK_STOP,0},{0x77,0},0x69e0,0},

	{";",XKEY_DOTCOM,{'S','o'},{'S','o'},{';',0},{0x3b,0xd2},0x4c,0},
	{"\"",XKEY_APOS,{'S','p'},{'S','p'},{0X27,0},{0x27,0xd1},0x52,0},
	{"-",XKEY_MINUS,{'S','j'},{'S','j'},{'-',0},{0x2d,0xe4},0x4e,0},
	{"+",XKEY_EQUAL,{'S','k'},{'S','k'},{'+',0},{0x3d,0xe2},0x00,0},
	{",",XKEY_COMMA,{'S','n'},{'S','n'},{',',0},{0x2c,0xf4},0x41,0},
	{".",XKEY_PERIOD,{'S','m'},{'S','m'},{'.',0},{0x2e,0xf3},0x49,0},
	{"/",XKEY_BSLASH,{'S','c'},{'S','c'},{'/',0},{0x5c,0x85},0x4a,0},

	{"ESC",XKEY_ESC,{0,0},{'C','1'},{0,MSXK_ESC},{0x1b,0x39},0x76,0},
	{"F1",XKEY_F1,{0,0},{'a'|0x80,0},{0,MSXK_F1},{0x61,0xb1},0x05,0},
	{"F2",XKEY_F2,{0,0},{'b'|0x80,0},{0,MSXK_F2},{0x62,0xb2},0x06,0},
	{"F3",XKEY_F3,{0,0},{'c'|0x80,0},{0,MSXK_F3},{0x63,0xb3},0x04,0},
	{"F4",XKEY_F4,{0,0},{'d'|0x80,0},{0,MSXK_F4},{0x64,0xb4},0x0C,0},
	{"F5",XKEY_F5,{0,0},{'e'|0x80,0},{0,MSXK_F5},{0x65,0xb5},0x03,0},
	{"F6",XKEY_F6,{0,0},{'f'|0x80,0},{0,0},{0x66,0xc5},0x0B,0},
	{"F7",XKEY_F7,{0,0},{'g'|0x80,0},{0,0},{0x67,0xc4},0x83,0},
	{"F8",XKEY_F8,{0,0},{'h'|0x80,0},{0,0},{0x68,0xc3},0x0A,0},
	{"F9",XKEY_F9,{0,0},{'i'|0x80,0},{0,0},{0x69,0xc2},0x01,0},
	{"F10",XKEY_F10,{0,0},{'j'|0x80,0},{0,0},{0x6a,0xc1},0x09,0},
	{"F11",XKEY_F11,{0,0},{'S','q'},{0,0},{0x6b,0},0x78,0},

	{"LA",XKEY_LALT,{0,0},{0,0},{0,0},{0,0},0x11,0},
	{"RA",XKEY_RALT,{0,0},{0,0},{MSXK_GRAPH,0},{0,0},0x11e0,0},

	{"NLOCK", XKEY_NUMLCK,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"NSLASH", XKEY_NSLASH,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"NMUL", XKEY_NMUL,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"NMINUS", XKEY_NMINUS,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"NPLUS", XKEY_NPLUS,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"NENT", XKEY_NENTER,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"NDOT", XKEY_NDOT,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"N0", XKEY_N0,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"N1", XKEY_N1,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"N2", XKEY_N2,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"N3", XKEY_N3,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"N4", XKEY_N4,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"N5", XKEY_N5,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"N6", XKEY_N6,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"N7", XKEY_N7,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"N8", XKEY_N8,{0,0},{0,0},{0,0},{0,0},0x00,0},
	{"N9", XKEY_N9,{0,0},{0,0},{0,0},{0,0},0x00,0},

	{"",ENDKEY,{0,0},{0,0},{0,0},{0,0},0x00,0}
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

void setKey(const char* kname, const char* kstr) {
	int idx = 0;
	int pos,kpos;
	while (keyMap[idx].key != ENDKEY) {
		if (!strcmp(kname, keyMap[idx].name)) {
			memset(keyMap[idx].zxKey, 0, KEYSEQ_MAXLEN);
			pos = 0;
			kpos = 0;
			while (pos < KEYSEQ_MAXLEN-1) {
				switch(kstr[kpos]) {
					case 0x00:
						pos = KEYSEQ_MAXLEN;
						break;
					case 'J':
						kpos++;
						switch (kstr[kpos]) {
							case 'U': keyMap[idx].joyMask = XJ_UP; break;
							case 'D': keyMap[idx].joyMask = XJ_DOWN; break;
							case 'R': keyMap[idx].joyMask = XJ_RIGHT; break;
							case 'L': keyMap[idx].joyMask = XJ_LEFT; break;
							case 'F': keyMap[idx].joyMask = XJ_FIRE; break;
							case '2': keyMap[idx].joyMask = XJ_BUT2; break;
							case '3': keyMap[idx].joyMask = XJ_BUT3; break;
							case '4': keyMap[idx].joyMask = XJ_BUT4; break;
						}
						kpos++;
						break;
					default:
						keyMap[idx].zxKey[pos] = kstr[kpos];
						pos++;
						kpos++;
						break;
				}
			}
//			strncpy((char*)keyMap[idx].zxKey, kstr, KEYSEQ_MAXLEN - 1);
//			printf("%s -> %c %c\n", keyMap[idx].name, key1, key2);
		}
		idx++;
	}
}

void initKeyMap() {
//	printf("init keys\n");
	int idx = -1;
	do {
		idx++;
		keyMap[idx] = keyMapInit[idx];
	} while (keyMapInit[idx].key != ENDKEY);
}

void loadKeys() {
	xProfile* prf = conf.prof.cur;
	if (!prf) return;
	std::string sfnam = conf.path.confDir + SLASH + prf->kmapName;
	initKeyMap();
	if ((prf->kmapName == "") || (prf->kmapName == "default")) return;
	std::ifstream file(sfnam);
	if (!file.good()) {
		sfnam = conf.path.confDir + SLASH + "keymaps" + SLASH + prf->kmapName;
		file.open(sfnam);
		if (!file.good()) {
			printf("Can't open keyboard layout. Default one will be used\n");
			return;
		}
	}
	char buf[1024];
//	std::pair<std::string,std::string> spl;
	std::string line;
	std::vector<std::string> vec;
	char keys[8];
	int rlen;
	unsigned int i;
	while (!file.eof()) {
		file.getline(buf,1023);
		line = std::string(buf);
		vec = splitstr(line,"\t");
		memset(keys, 0, 8);
		rlen = 0;
		if (vec.size() > 0) {
			for(i = 1; (rlen < KEYSEQ_MAXLEN) && (i < vec.size()); i++) {
				rlen += vec[i].size();
				if (rlen < KEYSEQ_MAXLEN)
					strcat(keys, vec[i].c_str());
			}
			setKey(vec[0].c_str(), keys);
		}
	}
}

// key translation qt->xkey

struct keyTrans {
	int keyLat;		// Qt::Key_Q : qt keycode @ QWERTY layout
	int keyRus;		// 0x419 : qt keycode @ russian 'JZUKEN' layout
	int keyId;		// internal key id = XKEY_*
};

static keyTrans ktTab[] = {
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
	{Qt::Key_Plus, Qt::Key_Plus, XKEY_EQUAL},
	{Qt::Key_Equal, Qt::Key_Equal, XKEY_EQUAL},
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
	{Qt::Key_Apostrophe, 1069, XKEY_APOS},
	{Qt::Key_Return, Qt::Key_Enter, XKEY_ENTER},

	{Qt::Key_Shift, Qt::Key_Shift, XKEY_LSHIFT},
	{Qt::Key_Z, 1071, XKEY_Z},
	{Qt::Key_X, 1063, XKEY_X},
	{Qt::Key_C, 1057, XKEY_C},
	{Qt::Key_V, 1052, XKEY_V},
	{Qt::Key_B, 1048, XKEY_B},
	{Qt::Key_N, 1058, XKEY_N},
	{Qt::Key_M, 1068, XKEY_M},
	{Qt::Key_Comma, 0x411, XKEY_COMMA},			// ,
	{Qt::Key_Period, 0x42e, XKEY_PERIOD},			// .
	{Qt::Key_Slash, Qt::Key_Slash, XKEY_BSLASH},		// ?
	{Qt::Key_Apostrophe, 0x44d, XKEY_APOS},			// '

#ifdef __APPLE__
	{Qt::Key_Meta, Qt::Key_Meta, XKEY_LCTRL},
#else
	{Qt::Key_Control, Qt::Key_Control, XKEY_LCTRL},
#endif
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

static keyTrans numPadTab[] = {
	{Qt::Key_0, Qt::Key_Insert, XKEY_N0},
	{Qt::Key_1, Qt::Key_End, XKEY_N1},
	{Qt::Key_2, Qt::Key_Down, XKEY_N2},
	{Qt::Key_3, Qt::Key_PageDown, XKEY_N3},
	{Qt::Key_4, Qt::Key_Left, XKEY_N4},
	{Qt::Key_5, Qt::Key_5, XKEY_N5},
	{Qt::Key_6, Qt::Key_Right, XKEY_N6},
	{Qt::Key_7, Qt::Key_Home, XKEY_N7},
	{Qt::Key_8, Qt::Key_Up, XKEY_N8},
	{Qt::Key_9, Qt::Key_PageUp, XKEY_N9},
	{Qt::Key_Period, Qt::Key_Delete, XKEY_NDOT},
	{Qt::Key_Slash, Qt::Key_Slash, XKEY_NSLASH},
	{Qt::Key_Asterisk, Qt::Key_Asterisk, XKEY_NMUL},
	{Qt::Key_Minus, Qt::Key_Minus, XKEY_NMINUS},
	{Qt::Key_Plus, Qt::Key_Plus, XKEY_NPLUS},
	{Qt::Key_Enter, Qt::Key_Return, XKEY_NENTER},
	{Qt::Key_unknown, Qt::Key_unknown, ENDKEY}
};

int qKey2id(int qkey, Qt::KeyboardModifiers mod) {
	int idx = 0;
	keyTrans* tab = (mod & Qt::KeypadModifier) ? numPadTab : ktTab;
	while ((tab[idx].keyLat != qkey) && (tab[idx].keyRus != qkey) && (tab[idx].keyLat != Qt::Key_unknown)) {
		idx++;
	}
	return tab[idx].keyId;
}

int key2qid(int key) {
	int idx = 0;
	while ((ktTab[idx].keyId != key) && (ktTab[idx].keyLat != Qt::Key_unknown)) {
		idx++;
	}
	return ktTab[idx].keyLat;
}

// shortcuts

static xShortcut short_tab[] = {
	{SCG_MAIN | SCG_DEBUGA, XCUT_OPTIONS, "key.options", "Options", QKeySequence(), QKeySequence(Qt::Key_F1)},
	{SCG_MAIN | SCG_DEBUGA, XCUT_DEBUG, "key.debuger", "deBUGa", QKeySequence(), QKeySequence(Qt::Key_Escape)},
	{SCG_MAIN, XCUT_PAUSE, "key.pause", "Pause", QKeySequence(), QKeySequence(Qt::Key_Pause)},
	{SCG_MAIN, XCUT_FAST, "key.fast", "Fast mode", QKeySequence(), QKeySequence(Qt::Key_Insert)},
	{SCG_MAIN | SCG_DEBUGA | SCG_DISASM, XCUT_SAVE, "key.save", "Save", QKeySequence(), QKeySequence(Qt::Key_F2)},
	{SCG_MAIN | SCG_DEBUGA, XCUT_LOAD, "key.load", "Open", QKeySequence(), QKeySequence(Qt::Key_F3)},
	{SCG_MAIN, XCUT_FASTSAVE, "key.fastsave", "Fast saving", QKeySequence(), QKeySequence(Qt::Key_F9)},
	{SCG_MAIN, XCUT_MOUSE, "key.mouse.grab", "Grab mouse", QKeySequence(), QKeySequence(Qt::ALT + Qt::Key_M)},
	{SCG_MAIN | SCG_DEBUGA, XCUT_KEYBOARD, "key.keywin", "Show keyboard", QKeySequence(), QKeySequence(Qt::ALT + Qt::Key_K)},
	{SCG_MAIN, XCUT_TAPWIN, "key.tapewin", "Show tape player", QKeySequence(), QKeySequence()},
	{SCG_MAIN, XCUT_RZXWIN, "key.rzxwin", "Show rzx player", QKeySequence(), QKeySequence()},
	{SCG_MAIN, XCUT_TAPLAY, "key.tape.play", "Tape play", QKeySequence(), QKeySequence(Qt::Key_F4)},
	{SCG_MAIN, XCUT_TAPREC, "key.tape.rec", "Tape rec", QKeySequence(), QKeySequence(Qt::Key_F5)},
	{SCG_MAIN, XCUT_SCRSHOT, "key.scrshot", "Screenshot", QKeySequence(), QKeySequence(Qt::Key_F7)},
	{SCG_MAIN, XCUT_COMBOSHOT, "key.scrshot.combo", "Screenshot combo", QKeySequence(), QKeySequence(Qt::ALT + Qt::Key_F7)},
	{SCG_MAIN, XCUT_SIZEX1, "key.size.x1", "Size x1", QKeySequence(), QKeySequence(Qt::ALT + Qt::Key_1)},
	{SCG_MAIN, XCUT_SIZEX2, "key.size.x2", "Size x2", QKeySequence(), QKeySequence(Qt::ALT + Qt::Key_2)},
	{SCG_MAIN, XCUT_SIZEX3, "key.size.x3", "Size x3", QKeySequence(), QKeySequence(Qt::ALT + Qt::Key_3)},
	{SCG_MAIN, XCUT_SIZEX4, "key.size.x4", "Size x4", QKeySequence(), QKeySequence(Qt::ALT + Qt::Key_4)},
	{SCG_MAIN, XCUT_FULLSCR, "key.fullscreen", "Fullscreen", QKeySequence(), QKeySequence(Qt::ALT + Qt::Key_Return)},
	{SCG_MAIN, XCUT_RATIO, "key.ratio", "Keep aspect ratio", QKeySequence(), QKeySequence(Qt::ALT + Qt::Key_R)},
	{SCG_MAIN, XCUT_NOFLICK, "key.noflick", "Noflick", QKeySequence(), QKeySequence(Qt::ALT + Qt::Key_N)},
	{SCG_MAIN, XCUT_NMI, "key.nmi", "NMI", QKeySequence(), QKeySequence(Qt::Key_F10)},
	{SCG_MAIN | SCG_DEBUGA, XCUT_RESET, "key.reset", "Reset", QKeySequence(), QKeySequence(Qt::Key_F12)},
	{SCG_MAIN, XCUT_RES_DOS, "key.reset.dos", "Reset to DOS", QKeySequence(), QKeySequence(Qt::ALT + Qt::Key_F12)},
	{SCG_MAIN, XCUT_TURBO, "key.turbo", "Switch turbo", QKeySequence(), QKeySequence(Qt::ALT + Qt::Key_T)},
	{SCG_MAIN, XCUT_TVLINES, "key.scanlines", "Switch scanlines", QKeySequence(), QKeySequence()},
	{SCG_MAIN, XCUT_WAV_OUT, "key.write.wav", "Start/stop wav output", QKeySequence(), QKeySequence()},
	{SCG_MAIN, XCUT_RELOAD_SHD, "key.reload.shader", "Reload shader", QKeySequence(), QKeySequence()},

	{SCG_DEBUGA, XCUT_STEPIN, "key.dbg.stepin", "DeBUGa: Step in", QKeySequence(), QKeySequence(Qt::Key_F7)},
	{SCG_DEBUGA, XCUT_STEPOVER, "key.dbg.stepover", "DeBUGa: Step over", QKeySequence(), QKeySequence(Qt::Key_F8)},
	{SCG_DEBUGA, XCUT_FASTSTEP, "key.dbg.faststep", "DeBUGa: Fast step", QKeySequence(), QKeySequence(Qt::ALT + Qt::Key_F7)},
	{SCG_DEBUGA, XCUT_TMPBRK, "key.dbg.runtohere", "DeBUGa: Stop here", QKeySequence(), QKeySequence(Qt::Key_F9)},
	{SCG_DEBUGA, XCUT_TRACE, "key.dbg.trace", "DeBUGa: Trace", QKeySequence(), QKeySequence(Qt::CTRL + Qt::Key_T)},
	{SCG_DEBUGA, XCUT_OPEN_DUMP, "key.dbg.opendump", "DeBUGa: Load dump", QKeySequence(), QKeySequence(Qt::CTRL + Qt::Key_O)},
	{SCG_DEBUGA, XCUT_SAVE_DUMP, "key.dbg.savedump", "DeBUGa: Save dump", QKeySequence(), QKeySequence(Qt::CTRL + Qt::Key_S)},
	{SCG_DEBUGA, XCUT_FINDER, "key.dbg.finder", "DeBUGa: Find pattern", QKeySequence(), QKeySequence(Qt::CTRL + Qt::Key_F)},
	{SCG_DEBUGA, XCUT_LABELS, "key.dbg.labels", "DeBUGa: Switch labels", QKeySequence(), QKeySequence(Qt::CTRL + Qt::Key_L)},

	{SCG_DISASM, XCUT_TOPC, "key.disasm.topc", "Diasm: Jump to PC", QKeySequence(), QKeySequence(Qt::Key_Home)},
	{SCG_DISASM, XCUT_SETPC, "key.disasm.setpc", "Diasm: Set PC", QKeySequence(), QKeySequence(Qt::Key_End)},
	{SCG_DISASM, XCUT_SETBRK, "key.disasm.setbrk", "Diasm: Breakpoint", QKeySequence(), QKeySequence(Qt::Key_Space)},
	{SCG_DISASM, XCUT_JUMPTO, "key.disasm.jump", "Diasm: Jump to operand", QKeySequence(), QKeySequence(Qt::Key_F4)},
	{SCG_DISASM, XCUT_RETFROM, "key.disasm.ret", "Diasm: Return", QKeySequence(), QKeySequence(Qt::Key_F5)},

	{0, -1, NULL, NULL, QKeySequence(), QKeySequence()}
};

void shortcut_init() {
	int i = 0;
	while (short_tab[i].text != NULL) {
		short_tab[i].seq = short_tab[i].def;
		i++;
	}
}

xShortcut* find_shortcut_id(int id) {
	int i = 0;
	while ((short_tab[i].id != id) && (short_tab[i].id >= 0))
		i++;
	return (short_tab[i].id < 0) ? NULL : &short_tab[i];
}

xShortcut* find_shortcut_name(const char* name) {
	int i = 0;
	while ((short_tab[i].id >= 0) && (strcmp(name, short_tab[i].name)))
		i++;
	return (short_tab[i].id < 0) ? NULL : &short_tab[i];
}

void set_shortcut_id(int id, QKeySequence seq) {
	xShortcut* cut = find_shortcut_id(id);
	if (cut != NULL)
		cut->seq = seq;
}

void set_shortcut_name(const char* name, QKeySequence seq) {
	xShortcut* cut = find_shortcut_name(name);
	if (cut != NULL)
		cut->seq = seq;
}

int shortcut_check(int grp, QKeySequence seq) {
	int res = -1;
	int i = 0;
	while (short_tab[i].id >= 0) {
		if (short_tab[i].seq.matches(seq) && !short_tab[i].seq.isEmpty() && (short_tab[i].grp & grp)) {
			res = short_tab[i].id;
		}
		i++;
	}
	return res;
}

int shortcut_match(int grp, int id, QKeySequence seq) {
	int res = QKeySequence::NoMatch;
	int i = 0;
	while(short_tab[i].id >= 0) {
		if ((short_tab[i].id == id) && (short_tab[i].grp & grp) && !short_tab[i].seq.isEmpty()) {
			res = short_tab[i].seq.matches(seq);
		}
		i++;
	}
	return res;
}

xShortcut* shortcut_tab() {
	return short_tab;
}
