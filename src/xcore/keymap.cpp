#include <string.h>

#include "xcore.h"

#if __linux

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

#elif _WIN32

#define	XKEY_1	2
#define	XKEY_2	3
#define	XKEY_3	4
#define	XKEY_4	5
#define	XKEY_5	6
#define	XKEY_6	7
#define	XKEY_7	8
#define	XKEY_8	9
#define	XKEY_9	10
#define	XKEY_0	11
#define	XKEY_MINUS	12
#define	XKEY_PLUS	13
#define	XKEY_BSP	14
#define	XKEY_TAB	15
#define	XKEY_Q	16
#define	XKEY_W	17
#define	XKEY_E	18
#define	XKEY_R	19
#define	XKEY_T	20
#define	XKEY_Y	21
#define	XKEY_U	22
#define	XKEY_I	23
#define	XKEY_O	24
#define	XKEY_P	25
#define	XKEY_LBRACE	26
#define	XKEY_RBRACE	27
#define	XKEY_ENTER	28
#define	XKEY_LCTRL	29
#define	XKEY_A	30
#define	XKEY_S	31
#define	XKEY_D	32
#define	XKEY_F	33
#define	XKEY_G	34
#define	XKEY_H	35
#define	XKEY_J	36
#define	XKEY_K	37
#define	XKEY_L	38
#define	XKEY_DOTCOM	39	// ;
#define	XKEY_QUOTE	40	// "
#define	XKEY_TILDA	41	// ~
#define	XKEY_LSHIFT	42
#define	XKEY_SLASH	43
#define	XKEY_Z	44
#define	XKEY_X	45
#define	XKEY_C	46
#define	XKEY_V	47
#define	XKEY_B	48
#define	XKEY_N	49
#define	XKEY_M	50
#define	XKEY_PERIOD	51
#define	XKEY_COMMA	52
#define	XKEY_BSLASH	53	// /
#define	XKEY_RSHIFT	54
#define	XKEY_SPACE	57
#define	XKEY_CAPS	58
#define XKEY_RCTRL	XKEY_LCTRL
#define	XKEY_RALT	312
#define XKEY_LALT	56
#define	XKEY_HOME	327
#define	XKEY_UP		328
#define	XKEY_PGUP	329
#define	XKEY_LEFT	331
#define	XKEY_RIGHT	333
#define	XKEY_END	335
#define	XKEY_DOWN	336
#define	XKEY_PGDN	337
#define	XKEY_INS	338
#define	XKEY_DEL	339
#define	XKEY_MENU	349
#define XKEY_ESC	1
#define XKEY_F1		59
#define XKEY_F2		60
#define XKEY_F3		61
#define XKEY_F4		62
#define XKEY_F5		63
#define XKEY_F6		64
#define XKEY_F7		65
#define XKEY_F8		66
#define XKEY_F9		67
#define XKEY_F10	68
#define XKEY_F11	87

#endif

#define ENDKEY 0

// KEYMAPS

keyEntry keyMap[256];			// current keymap (init at start from keyMapInit[]

keyEntry keyMapInit[] = {
	{"1",XKEY_1,'1',0,0,0,0x16},{"2",XKEY_2,'2',0,0,0,0x1e},{"3",XKEY_3,'3',0,0,0,0x26},{"4",XKEY_4,'4',0,0,0,0x25},{"5",XKEY_5,'5',0,0,0,0x2e},
	{"6",XKEY_6,'6',0,0,0,0x36},{"7",XKEY_7,'7',0,0,0,0x3d},{"8",XKEY_8,'8',0,0,0,0x3e},{"9",XKEY_9,'9',0,0,0,0x46},{"0",XKEY_0,'0',0,0,0,0x45},
	{"Q",XKEY_Q,'q',0,0,0,0x15},{"W",XKEY_W,'w',0,0,0,0x1d},{"E",XKEY_E,'e',0,0,0,0x24},{"R",XKEY_R,'r',0,0,0,0x2d},{"T",XKEY_T,'t',0,0,0,0x2c},
	{"Y",XKEY_Y,'y',0,0,0,0x35},{"U",XKEY_U,'u',0,0,0,0x3c},{"I",XKEY_I,'i',0,0,0,0x43},{"O",XKEY_O,'o',0,0,0,0x44},{"P",XKEY_P,'p',0,0,0,0x4d},
	{"A",XKEY_A,'a',0,0,0,0x1c},{"S",XKEY_S,'s',0,0,0,0x1b},{"D",XKEY_D,'d',0,0,0,0x23},{"F",XKEY_F,'f',0,0,0,0x2b},{"G",XKEY_G,'g',0,0,0,0x34},
	{"H",XKEY_H,'h',0,0,0,0x33},{"J",XKEY_J,'j',0,0,0,0x3b},{"K",XKEY_K,'k',0,0,0,0x42},{"L",XKEY_L,'l',0,0,0,0x4b},{"ENT",XKEY_ENTER,'E',0,0,0,0x5a},
	{"LS",XKEY_LSHIFT,'C',0,0,0,0x12},{"Z",XKEY_Z,'z',0,0,0,0x1a},{"X",XKEY_X,'x',0,0,0,0x22},{"C",XKEY_C,'c',0,0,0,0x21},{"V",XKEY_V,'v',0,0,0,0x2a},
	{"B",XKEY_B,'b',0,0,0,0x32},{"N",XKEY_N,'n',0,0,0,0x31},{"M",XKEY_M,'m',0,0,0,0x3a},{"LC",XKEY_LCTRL,'S',0,0,0,0x14},{"SPC",XKEY_SPACE,' ',0,0,0,0x29},

	{"RS",XKEY_RSHIFT,'C',0,0,0,0x59},{"RC",XKEY_RCTRL,'S',0,0,0,0x14e0},

	{"LEFT",XKEY_LEFT,'C','5','C','5',0x6be0},{"RIGHT",XKEY_RIGHT,'C','8','C','8',0x74e0},
	{"DOWN",XKEY_DOWN,'C','6','C','6',0x72e0},{"UP",XKEY_UP,'C','7','C','7',0x75e0},
	{"BSP",XKEY_BSP,'C','0','C','0',0x66},
	{"CAPS",XKEY_CAPS,'C','2','C','2',0x58},
	{"TAB",XKEY_TAB,'C',' ','C','i',0x0d},
	{"[",XKEY_LBRACE,'S','8','S','y',0x54},{"]",XKEY_RBRACE,'S','9','S','u',0x5b},
	{"`",XKEY_TILDA,'C','S','S','x',0x0e},
	{"\\",XKEY_SLASH,'C','S','S','d',0x5d},
	{"PGDN",XKEY_PGUP,'C','3','m'|0x80,0,0x7de0},{"PGUP",XKEY_PGDN,'C','4','n'|0x80,0,0x7ae0},
	{"DEL",XKEY_DEL,'C','9','p'|0x80,0,0x71e0},{"INS",XKEY_INS,'S','w','o'|0x80,0,0x70e0},
	{"HOME",XKEY_HOME,'S','q','k'|0x80,0,0x6ce0},{"END",XKEY_END,'S','e','l'|0x80,0,0x69e0},
	{";",XKEY_DOTCOM,'S','o','S','o',0x4c},{"\"",XKEY_QUOTE,'S','p','S','p',0x52},
	{"-",XKEY_MINUS,'S','j','S','j',0x4e},{"+",XKEY_PLUS,'S','k','S','k',0x00},
	{",",XKEY_PERIOD,'S','n','S','n',0x41},{".",XKEY_COMMA,'S','m','S','m',0x49},{"/",XKEY_BSLASH,'S','c','S','c',0x4a},

	{"ESC",XKEY_ESC,0,0,'C','1',0x76},
	{"F1",XKEY_F1,0,0,'a'|0x80,0,0x05},{"F2",XKEY_F2,0,0,'b'|0x80,0,0x06},{"F3",XKEY_F3,0,0,'c'|0x80,0,0x04},{"F4",XKEY_F4,0,0,'d'|0x80,0,0x0C},
	{"F5",XKEY_F5,0,0,'e'|0x80,0,0x03},{"F6",XKEY_F6,0,0,'f'|0x80,0,0x0B},{"F7",XKEY_F7,0,0,'g'|0x80,0,0x83},{"F8",XKEY_F8,0,0,'h'|0x80,0,0x0A},
	{"F9",XKEY_F9,0,0,'i'|0x80,0,0x01},{"F10",XKEY_F10,0,0,'j'|0x80,0,0x09},{"F11",XKEY_F11,0,0,'S','q',0x78},

	{"LA",XKEY_LALT,0,0,0,0,0x11},{"RA",XKEY_RALT,0,0,0,0,0x11e0},

	{"",ENDKEY,0,0,0,0,0x00}
};

keyEntry getKeyEntry(signed int qkey) {
	int idx = 0;
	while (1) {
		if (keyMap[idx].key == ENDKEY) break;
		if (keyMap[idx].key == qkey) break;
		idx++;
	}
	return keyMap[idx];
}

void setKey(const char* key,const char key1, const char key2) {
	int idx = 0;
	while (keyMap[idx].key != ENDKEY) {
		if (strcmp(key,keyMap[idx].name) == 0) {
			keyMap[idx].key1 = key1;
			keyMap[idx].key2 = key2;
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
