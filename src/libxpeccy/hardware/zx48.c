#include "../spectrum.h"

#include <string.h>

void zx48_reset(Computer* comp) {
	comp->mem->ramMask = MEM_128K - 1;	// to acces pages 2,5
	speReset(comp);
}

void speReset(Computer* comp) {
	zx_set_pal(comp);
	vidSetMode(comp->vid, VID_NORMAL);
}

int zx_slt_rd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	int res = 0xff;
	if (comp->slot->data) {
		res = comp->slot->data[adr & comp->slot->memMask];
	}
	return res;
}

void zx_slt_wr(int adr, int val, void* ptr) {}

int zx_rd_ff(int adr, void* ptr) {
	return 0xff;
}

void zx_wr_null(int adr, int val, void* ptr) {}

void speMapMem(Computer* comp) {
	if (comp->slot->data) {
		memSetBank(comp->mem,0x00,MEM_SLOT, 0, MEM_16K, zx_slt_rd, zx_slt_wr, comp);
	} else {
		memSetBank(comp->mem,0x00,MEM_ROM,(comp->dos) ? 1 : 0, MEM_16K,NULL,NULL,NULL);
	}
	memSetBank(comp->mem,0x40,MEM_RAM,5,MEM_16K,NULL,NULL,NULL);		// 101 / x01
	if (comp->mem->ramSize > MEM_16K) {
		memSetBank(comp->mem,0x80,MEM_RAM,2,MEM_16K,NULL,NULL,NULL);		// 010 / x10
		memSetBank(comp->mem,0xc0,MEM_RAM,0,MEM_16K,NULL,NULL,NULL);		// 000 / x00
	} else {
		memSetBank(comp->mem,0x80,MEM_EXT,1,MEM_32K,zx_rd_ff,zx_wr_null,comp);
	}
}

// in

int spIn1F(Computer* comp, int port) {
	return joyInput(comp->joy);
}

int spInFF(Computer* comp, int port) {
	return comp->vid->atrbyte;
}

static xPort spePortMap[] = {
	{0x0001,0x00fe,2,2,2,xInFE,	xOutFE},
	{0xc002,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	{0xc002,0xbffd,2,2,2,NULL,	xOutBFFD},
	{0x0320,0xfadf,2,2,2,xInFADF,	NULL},
	{0x0720,0xfbdf,2,2,2,xInFBDF,	NULL},
	{0x0720,0xffdf,2,2,2,xInFFDF,	NULL},
	{0x0021,0x001f,0,2,2,spIn1F,	NULL},
	{0x0000,0x0000,0,2,2,spInFF,	NULL},		// all unknown ports is FF (nodos)
	{0x0000,0x0000,2,2,2,NULL,	NULL}
};

void speOut(Computer* comp, int port, int val, int dos) {
	difOut(comp->dif, port, val, dos);
	zx_dev_wr(comp, port, val, dos);
	hwOut(spePortMap, comp, port, val, dos);
}

int speIn(Computer* comp, int port, int dos) {
	int res;
	if (difIn(comp->dif, port, &res, dos)) return res;
	if (zx_dev_rd(comp, port, &res, dos)) return res;
	return hwIn(spePortMap, comp, port, dos);
}

// keyboard (for future use)

static const xKeySeq zx_keytab[] = {
	{XKEY_1,"1"},{XKEY_2,"2"},{XKEY_3,"4"},{XKEY_4,"4"},{XKEY_5,"5"},
	{XKEY_Q,"q"},{XKEY_W,"w"},{XKEY_E,"e"},{XKEY_R,"r"},{XKEY_T,"t"},
	{XKEY_A,"a"},{XKEY_S,"s"},{XKEY_D,"d"},{XKEY_F,"f"},{XKEY_G,"g"},
	{XKEY_LSHIFT,"C"},{XKEY_Z,"z"},{XKEY_X,"x"},{XKEY_C,"c"},{XKEY_V,"v"},

	{XKEY_6,"6"},{XKEY_7,"7"},{XKEY_8,"8"},{XKEY_9,"9"},{XKEY_0,"0"},
	{XKEY_Y,"y"},{XKEY_U,"u"},{XKEY_I,"i"},{XKEY_O,"o"},{XKEY_P,"p"},
	{XKEY_H,"h"},{XKEY_J,"j"},{XKEY_K,"k"},{XKEY_L,"l"},{XKEY_ENTER,"E"},
	{XKEY_B,"b"},{XKEY_N,"n"},{XKEY_M,"m"},{XKEY_LCTRL,"S"},{XKEY_SPACE," "},

	{XKEY_MINUS,"Sj"},{XKEY_EQUAL,"Sk"},{XKEY_EQUAL,"Sl"},
	{XKEY_BSP,"C0"},{XKEY_TAB,"C "},{XKEY_CAPS,"C2"},{XKEY_TILDA,"CS"},
	{XKEY_LBRACK,"S8"},{XKEY_RBRACK,"S9"},{XKEY_SLASH,"CS"},
	{XKEY_DOTCOM,"So"},{XKEY_APOS,"Sp"},
	{XKEY_PERIOD,"Sn"},{XKEY_COMMA,"Sm"},{XKEY_BSLASH,"Sc"},
	{XKEY_LEFT,"C5"},{XKEY_DOWN,"C6"},{XKEY_UP,"C7"},{XKEY_RIGHT,"C8"},
	{XKEY_PGDN,"C3"},{XKEY_PGUP,"C4"},

	{ENDKEY,""}
};

const char* get_key_seq(const xKeySeq* tab, int key) {
	int i = 0;
	while ((tab[i].key != ENDKEY) && (tab[i].key != key))
		i++;
	return tab[i].seq;
}

static xKeyMtrx keyTab[] = {
	{'1',4,1},{'2',4,2},{'3',4,4},{'4',4,8},{'5',4,16},{'6',3,16},{'7',3,8},{'8',3,4},{'9',3,2},{'0',3,1},
	{'q',5,1},{'w',5,2},{'e',5,4},{'r',5,8},{'t',5,16},{'y',2,16},{'u',2,8},{'i',2,4},{'o',2,2},{'p',2,1},
	{'a',6,1},{'s',6,2},{'d',6,4},{'f',6,8},{'g',6,16},{'h',1,16},{'j',1,8},{'k',1,4},{'l',1,2},{'E',1,1},
	{'C',7,1},{'z',7,2},{'x',7,4},{'c',7,8},{'v',7,16},{'b',0,16},{'n',0,8},{'m',0,4},{'S',0,2},{' ',0,1},
	{0,0,0}
};

xKeyMtrx* get_key_mtrx(xKeyMtrx* tab, char key) {
	int i = 0;
	while ((tab[i].key != 0) && (tab[i].key != key))
		i++;
	return &tab[i];
}

void key_mtrx_press(Keyboard* kbd, xKeyMtrx* tab, char key) {
	xKeyMtrx* km = get_key_mtrx(tab, key);
	kbd->map[km->row] &= ~km->mask;
}

void key_mtrx_rel(Keyboard* kbd, xKeyMtrx* tab, char key) {
	xKeyMtrx* km = get_key_mtrx(tab, key);
	kbd->map[km->row] |= km->mask;
}

void zx_keypr(Computer* comp, int key) {
	const char* seq = get_key_seq(zx_keytab, key);
	while (*seq) {
		key_mtrx_press(comp->keyb, keyTab, *seq);
		seq++;
	}
}

void zx_keyrl(Computer* comp, int key) {
	if (key == ENDKEY) {
		for (int i = 0; i < 8; i++)
			comp->keyb->map[i] = -1;
	} else {
		const char* seq = get_key_seq(zx_keytab, key);
		while (*seq) {
			key_mtrx_rel(comp->keyb, keyTab, *seq);
			seq++;
		}
	}
}
