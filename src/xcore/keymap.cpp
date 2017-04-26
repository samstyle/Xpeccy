#include <string.h>

#include "xcore.h"

#define	XKEY_1	10
#define	XKEY_2	11
#define	XKEY_3	12
#define	XKEY_4	13
#define	XKEY_5	14
#define	XKEY_6	15
#define	XKEY_7	16
#define	XKEY_8	17
#define	XKEY_9	18
#define	XKEY_0	19
#define	XKEY_MINUS	20
#define	XKEY_PLUS	21
#define	XKEY_BSP	22
#define	XKEY_TAB	23
#define	XKEY_Q	24
#define	XKEY_W	25
#define	XKEY_E	26
#define	XKEY_R	27
#define	XKEY_T	28
#define	XKEY_Y	29
#define	XKEY_U	30
#define	XKEY_I	31
#define	XKEY_O	32
#define	XKEY_P	33
#define	XKEY_LBRACE	34
#define	XKEY_RBRACE	35
#define	XKEY_ENTER	36
#define	XKEY_LCTRL	37
#define	XKEY_A	38
#define	XKEY_S	39
#define	XKEY_D	40
#define	XKEY_F	41
#define	XKEY_G	42
#define	XKEY_H	43
#define	XKEY_J	44
#define	XKEY_K	45
#define	XKEY_L	46
#define	XKEY_DOTCOM	47	// ;
#define	XKEY_QUOTE	48	// "
#define	XKEY_TILDA	49	// ~
#define	XKEY_LSHIFT	50
#define	XKEY_SLASH	51
#define	XKEY_Z	52
#define	XKEY_X	53
#define	XKEY_C	54
#define	XKEY_V	55
#define	XKEY_B	56
#define	XKEY_N	57
#define	XKEY_M	58
#define	XKEY_PERIOD	59
#define	XKEY_COMMA	60
#define	XKEY_BSLASH	61	// /
#define	XKEY_SPACE	65
#define	XKEY_CAPS	66
#define	XKEY_RSHIFT	62
#define	XKEY_RCTRL	105
#define XKEY_LALT	64
#define	XKEY_RALT	108
#define	XKEY_HOME	110
#define	XKEY_UP		111
#define	XKEY_PGUP	112
#define	XKEY_LEFT	113
#define	XKEY_RIGHT	114
#define	XKEY_END	115
#define	XKEY_DOWN	116
#define	XKEY_PGDN	117
#define	XKEY_INS	118
#define	XKEY_DEL	119
#define	XKEY_MENU	135
#define XKEY_ESC	9
#define XKEY_F1		67
#define XKEY_F2		68
#define XKEY_F3		69
#define XKEY_F4		70
#define XKEY_F5		71
#define XKEY_F6		72
#define XKEY_F7		73
#define XKEY_F8		74
#define XKEY_F9		75
#define XKEY_F10	76
#define XKEY_F11	95

#define XKEY_LBRACK	256	// {
#define XKEY_RBRACK	257	// }
#define XKEY_QUEST	258

#define ENDKEY 0

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
