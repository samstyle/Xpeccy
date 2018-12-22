#include <string.h>

#include "hardware.h"

// 0000..7FFF	RAM
// 8000..FF7F	ROM
// FF80..FFFF	IO

// 0xffb0 (0177660)
// b6:rd/wr:disable keyboard INT
// b7:rd:keycode present in keyboard data reg

// 0xffb2 (0177662)
// b0..6:rd:key code
// rd:reset b7,ffb0

// hdd
// ffe0	: wr:com rd:status (7)
// ffe1 : #17
// ffe2 : master/slave select, head (6)
// ffe3 : rd: astate	(#16)
// ffe4 : trk (hi)	(5)
// ffe6 : trk (low)	(4)
// ffe8 : sec		(3)
// ffec	: rd:error code (1)
// ffee : data.low	(0)
// ffef : data.high	(#10 ? )
// ~bit1..3 = register
// bit 0 : alt.reg (#16, #17)
// data reg is 16-bit, others 8-bit

unsigned char bk_io_rd(unsigned short adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	comp->wdata = 0xffff;
	unsigned char res;
	switch (adr & 0xfffe) {
		// fdc
		case 0xfe58:
			if (!(adr & 1))
				comp->wreg = difIn(comp->dif, 0, NULL, 0) & 0xffff;
			comp->wdata = comp->wreg;
			break;
		case 0xfe5a:
			if (!(adr & 1))
				comp->wtmp = difIn(comp->dif, 1, NULL, 0) & 0xffff;
			comp->wdata = comp->wtmp;
			break;
		// keyboard
		case 0xffb0: comp->wdata = comp->keyb->map[7] & 0xc0; break;	// keyboard int control
		case 0xffb2:							// keyboard code
			comp->wdata = comp->keyb->map[0] & 0x7f;
			comp->keyb->map[7] &= 0x7f;
			break;
		// vertical scroll register
		case 0xffb4: comp->wdata = comp->vid->sc.y & 0x00ff;
			if (!comp->vid->cutscr) comp->wdata |= 0x200;
			break;
		// storage
		case 0xffbc: comp->wdata = comp->reg[0xbc] | ((comp->reg[0xbd] << 8) & 0xff00); break;
		case 0xffbe: comp->wdata = comp->reg[0xbe] | ((comp->reg[0xbf] << 8) & 0xff00); break;
		// timer
		case 0xffc6: comp->wdata = comp->cpu->timer.ival; break;		// timer:initial counter value
		case 0xffc8: comp->wdata = comp->cpu->timer.val; break;			// timer:current counter
		case 0xffca: comp->wdata = (comp->cpu->timer.flag & 0xff) | 0xff00; break;	// timer flags
		// system
		case 0xffcc: comp->wdata = 0xffff; break;							// external port
		case 0xffce:							// system port
			comp->wdata = 0xc080;
			if (~comp->keyb->map[7] & 0x20)
				comp->wdata |= 0x40;		// = 0 if any key pressed
			break;
		// hdd
		case 0xffe0:
		case 0xffe2:
		case 0xffe4:
		case 0xffe6:
		case 0xffe8:
		case 0xffea:
		case 0xffec:
		case 0xffee:
			ideIn(comp->ide, adr, &res, 0);
			comp->wdata = res & 0xff;
			adr &= ~1;
			break;
		default:
			if (!comp->debug) {
				printf("%.4X : rd %.4X\n",comp->cpu->pc, adr);
				assert(0);
			}
			break;
	}
	return  (adr & 1) ? (comp->wdata >> 8) & 0xff : comp->wdata & 0xff;
}

void bk_io_wr(unsigned short adr, unsigned char val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	comp->iomap[adr] = val;
	switch (adr) {
		// fdc
		case 0xfe58: comp->reg[0x58] = val; break;
		case 0xfe59:
			difOut(comp->dif, 0, 0x00, comp->reg[0x58] | (val << 8));
			break;
		case 0xfe5a: comp->reg[0x5a] = val; break;
		case 0xfe5b:
			difOut(comp->dif, 1, 0x00, comp->reg[0x5a] | (val << 8));
			break;
		// keyboard
		case 0xffb0:
			comp->keyb->map[7] &= ~0x40;
			comp->keyb->map[7] |= (val & 0x40);
			break;
		case 0xffb1: break;
		// pal/timer/scrbuf
		case 0xffb2: break;
		case 0xffb3:
			comp->reg[0xb3] = val;
			comp->vid->paln = (val << 2) & 0x3c;
			comp->vid->curscr = (val & 0x80) ? 1 : 0;
			break;
		// scroll
		case 0xffb4: comp->vid->sc.y = val;
			break;
		case 0xffb5:
			comp->vid->cutscr = (val & 0x02) ? 0 : 1;
			break;
		case 0xffbc:
		case 0xffbd:
		case 0xffbe:
		case 0xffbf:
			comp->reg[adr & 0xff] = val;
			break;
		// timer
		case 0xffc6: comp->cpu->timer.ivl = val; break;
		case 0xffc7: comp->cpu->timer.ivh = val; break;
		case 0xffc8: comp->cpu->timer.vl = val; break;
		case 0xffc9: comp->cpu->timer.vh = val; break;
		case 0xffca: comp->cpu->timer.flag = val & 0xff;
			comp->cpu->timer.per = 128;
			if (val & 0x40) comp->cpu->timer.per <<= 2;	// div 4
			if (val & 0x20) comp->cpu->timer.per <<= 4;	// div 16
			if (val & 0x12) comp->cpu->timer.val = comp->cpu->timer.ival; // reload value
			break;
		case 0xffcb: break;

		case 0xffcc:
		case 0xffcd: break;
		case 0xffce: comp->reg[0xce] = val; break;
		case 0xffcf:
			if (val & 8) {	// extend
				comp->reg[0] = (val >> 4) & 7;
				if (comp->reg[0xce] & 1) {
					comp->reg[1] = 0x80;
				} else if (comp->reg[0xce] & 2) {
					comp->reg[1] = 0x81;
				} else if (comp->reg[0xce] & 8) {
					comp->reg[1] = 0x82;
				} else if (comp->reg[0xce] & 0x10) {
					comp->reg[1] = 0x83;
				} else {
					comp->reg[1] = val & 7;
				}
				bk_mem_map(comp);
			} else {	// tape control

			}
			break;
		default:
			printf("%.4X : wr %.4X,%.2X\n",comp->cpu->pc,adr,val);
			assert(0);
			break;
	}
}

void bk_sync(Computer* comp, int ns) {
	if ((comp->vid->newFrame) && (comp->reg[0xb3] & 0x40)) {
		comp->cpu->intrq |= PDP_INT_IRQ2;
	}
	difSync(comp->dif, ns);
}

void bk_mem_map(Computer* comp) {
	memSetBank(comp->mem, 0x00, MEM_RAM, 6, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0x40, MEM_RAM, comp->reg[0] & 7, MEM_16K, NULL, NULL, NULL);
	if (comp->reg[1] & 0x80) {
		memSetBank(comp->mem, 0x80, MEM_ROM, comp->reg[1] & 3, MEM_16K, NULL, NULL, NULL);
	} else {
		memSetBank(comp->mem, 0x80, MEM_RAM, comp->reg[1] & 7, MEM_16K, NULL, NULL, NULL);
	}
	memSetBank(comp->mem, 0xc0, MEM_ROM, 4, MEM_8K, NULL, NULL, NULL);
	if (comp->dif->type == DIF_SMK512) {
		memSetBank(comp->mem, 0xe0, MEM_ROM, 5, MEM_8K, NULL, NULL, NULL);			// disk interface rom
		memSetBank(comp->mem, 0xf0, MEM_IO, 0xfe, MEM_4K, bk_io_rd, bk_io_wr, comp);		// 0170000..0177776 with disk interface
	} else {
		memSetBank(comp->mem, 0xe0, MEM_EXT, 7, MEM_8K, NULL, NULL, NULL);			// empty space
		memSetBank(comp->mem, 0xff, MEM_IO, 0xff, MEM_256, bk_io_rd, bk_io_wr, comp);		// 0177600..0177776 without disk interface
	}
}

#define BK_BLK	{0,0,0}
#define BK_BLU	{0,0,255}
#define BK_RED	{255,0,0}
#define BK_MAG	{255,0,255}
#define BK_GRN	{0,255,0}
#define BK_CYA	{0,255,255}
#define BK_YEL	{255,255,0}
#define BK_WHT	{255,255,255}

static xColor bk_pal[0x40] = {
	BK_BLK,BK_BLU,BK_GRN,BK_RED,	// 0: standard
	BK_BLK,BK_YEL,BK_MAG,BK_RED,
	BK_BLK,BK_CYA,BK_BLU,BK_MAG,
	BK_BLK,BK_GRN,BK_CYA,BK_YEL,
	BK_BLK,BK_MAG,BK_CYA,BK_WHT,
	BK_BLK,BK_WHT,BK_WHT,BK_WHT,	// 5: for b/w mode
	BK_BLK,BK_RED,BK_RED,BK_RED,
	BK_BLK,BK_GRN,BK_GRN,BK_GRN,
	BK_BLK,BK_MAG,BK_MAG,BK_MAG,
	BK_BLK,BK_GRN,BK_MAG,BK_RED,
	BK_BLK,BK_GRN,BK_MAG,BK_RED,
	BK_BLK,BK_CYA,BK_YEL,BK_RED,
	BK_BLK,BK_RED,BK_GRN,BK_CYA,
	BK_BLK,BK_CYA,BK_YEL,BK_WHT,
	BK_BLK,BK_YEL,BK_GRN,BK_WHT,
	BK_BLK,BK_CYA,BK_GRN,BK_WHT
};

void bk_reset(Computer* comp) {
	memSetSize(comp->mem, MEM_128K, MEM_64K);
	memset(comp->mem->ramData, 0x00, MEM_256);
	for (int i = 0; i < 0x40; i++) {
		comp->vid->pal[i] = bk_pal[i];
	}
	comp->reg[0] = 1;
	comp->reg[1] = 0x80;
	comp->cpu->reset(comp->cpu);
	comp->vid->curscr = 0;
	comp->vid->paln = 0;
	vidSetMode(comp->vid, VID_BK_BW);
	comp->keyb->map[7] = 0x40;
	comp->keyb->map[0] = 0;
	bk_mem_map(comp);
}

void bk_mwr(Computer* comp, unsigned short adr, unsigned char val) {
	memWr(comp->mem, adr, val);
}

unsigned char bk_mrd(Computer* comp, unsigned short adr, int m1) {
	return memRd(comp->mem, adr);
}

sndPair bk_vol(Computer* comp, sndVolume* vol) {
	sndPair res;
	res.left = 0;
	res.right = 0;
	return res;
}

// keys

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
	{XKEY_LBRACK,0150},{XKEY_RBRACK,0177},{XKEY_LBRACE,0150},{XKEY_RBRACE,0177},
	{XKEY_A,0146},{XKEY_S,0171},{XKEY_D,0167},{XKEY_F,0141},{XKEY_G,0160},
	{XKEY_H,0162},{XKEY_J,0157},{XKEY_K,0154},{XKEY_L,0144},{XKEY_DOTCOM,0166},{XKEY_APOS,0174},
	{XKEY_Z,0161},{XKEY_X,0176},{XKEY_C,0163},{XKEY_V,0155},{XKEY_B,0111},
	{XKEY_N,0164},{XKEY_M,0170},{XKEY_COMMA,0142},{XKEY_PERIOD,0140},
	{ENDKEY, 0}
};

static bkKeyCode bkey_small_rus[] = {
	{XKEY_Q,0112},{XKEY_W,0103},{XKEY_E,0125},{XKEY_R,0113},{XKEY_T,0105},
	{XKEY_Y,0116},{XKEY_U,0107},{XKEY_I,0133},{XKEY_O,0135},{XKEY_P,0132},
	{XKEY_LBRACK,0110},{XKEY_RBRACK,0137},{XKEY_LBRACE,0110},{XKEY_RBRACE,0137},
	{XKEY_A,0106},{XKEY_S,0131},{XKEY_D,0127},{XKEY_F,0101},{XKEY_G,0120},
	{XKEY_H,0122},{XKEY_J,0117},{XKEY_K,0114},{XKEY_L,0104},{XKEY_DOTCOM,0126},{XKEY_APOS,0134},
	{XKEY_Z,0121},{XKEY_X,0136},{XKEY_C,0123},{XKEY_V,0115},{XKEY_B,0111},
	{XKEY_N,0124},{XKEY_M,0130},{XKEY_COMMA,0102},{XKEY_PERIOD,0100},
	{ENDKEY, 0}
};

static bkKeyCode bkeyTab[] = {
	{XKEY_1,'1'},{XKEY_2,'2'},{XKEY_3,'3'},{XKEY_4,'4'},{XKEY_5,'5'},
	{XKEY_6,'6'},{XKEY_7,'7'},{XKEY_8,'8'},{XKEY_9,'9'},{XKEY_0,'0'},
	{XKEY_MINUS,'-'},{XKEY_PLUS,'+'},
	{XKEY_LBRACK,'('},{XKEY_RBRACK,')'},
	{XKEY_DOTCOM, ';'},{XKEY_APOS,'"'},
	{XKEY_COMMA, ','},{XKEY_PERIOD, '.'},{XKEY_QUEST,'/'},
	{XKEY_SPACE,' '},{XKEY_ENTER,10},
	{XKEY_BSP,24},
	{XKEY_TAB,13},
	{XKEY_DOWN,27},{XKEY_LEFT,8},{XKEY_RIGHT,25},{XKEY_UP,26},
	{ENDKEY, 0}
};

int bkey_code(bkKeyCode* tab, int xkey) {
	int idx = 0;
	while ((tab[idx].xkey != ENDKEY) && (tab[idx].xkey != xkey))
		idx++;
	return tab[idx].code;		// 0 if ENDKEY
}

static char bkcapson[] = " caps on ";
static char bkcapsoff[] = " caps off ";
static char bkkbdrus[] = " rus ";
static char bkkbdlat[] = " lat ";

void bk_keyp(Computer* comp, keyEntry xkey) {
	int code = 0;
	switch(xkey.key) {
		case XKEY_PGUP: vidSetMode(comp->vid, VID_BK_COL); break;
		case XKEY_PGDN: vidSetMode(comp->vid, VID_BK_BW); break;
		case XKEY_LSHIFT: comp->keyb->shift = 1; break;
		case XKEY_CAPS:
			comp->keyb->caps ^= 1;
			code = comp->keyb->caps ? 0274 : 0273;
			comp->msg = comp->keyb->caps ? bkcapson : bkcapsoff;
			break;
		case XKEY_LCTRL:
			comp->keyb->lang ^= 1;
			code = comp->keyb->lang ? 016 : 017;
			comp->msg = comp->keyb->lang ? bkkbdrus : bkkbdlat;
			break;
	}
	if (code == 0) {
		if (comp->keyb->caps ^ comp->keyb->shift) {
			code = bkey_code(comp->keyb->lang ? bkey_big_rus : bkey_big_lat, xkey.key);
		} else {
			code = bkey_code(comp->keyb->lang ? bkey_small_rus : bkey_small_lat, xkey.key);
		}
	}
	if (code == 0)
		code = bkey_code(bkeyTab, xkey.key);
	if (code != 0) {
		comp->keyb->map[7] |= 0x20;
		if (!(comp->keyb->map[7] & 0x80)) {
			comp->keyb->map[0] = code & 0x7f;
			comp->keyb->map[7] |= 0x80;
//			printf("code %.2X\n",comp->keyb->map[0]);
			if (!(comp->keyb->map[7] & 0x40)) {		// keyboard interrupt enabled
				comp->cpu->intvec = (code & 0x80) ? 0274 : 060;
				comp->cpu->intrq |= PDP_INT_VIRQ;
//				printf("intrq %X\n", comp->cpu->intvec);
			}
		}
	}
}

void bk_keyr(Computer* comp, keyEntry xkey) {
	switch (xkey.key) {
		case XKEY_LSHIFT: comp->keyb->shift = 0; break;
	}
	comp->keyb->map[7] &= ~0x20;	// 0x20 | 0x80
}
