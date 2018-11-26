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

unsigned char bk_io_rd(unsigned short adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	comp->wdata = 0xffff;
	switch (adr & 0xfffe) {
		case 0xffb0: comp->wdata = comp->keyb->map[7] & 0xc0; break;	// keyboard int control
		case 0xffb2:							// keyboard code
			comp->wdata = comp->keyb->map[0] & 0x7f;
			comp->keyb->map[7] &= 0x7f;
			break;
		case 0xffb4: comp->wdata = comp->vid->sc.y & 0x00ff;
			if (!comp->vid->cutscr) comp->wdata |= 0x200;
			break;	// vertical scroll register
		case 0xffcc: comp->wdata = 0xffff; break;							// external port
		case 0xffce:							// system port
			comp->wdata = 0x8080;
			if (~comp->keyb->map[7] & 0x20)
				comp->wdata |= 0x40;		// = 0 if any key pressed
			break;
		default:
			if (!comp->debug) {
				printf("%.4X : rd %.4X\n",comp->cpu->pc, adr);
				comp->brk = 1;
			}
			break;
	}
	return  (adr & 1) ? (comp->wdata >> 8) & 0xff : comp->wdata & 0xff;
}

void bk_io_wr(unsigned short adr, unsigned char val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	switch (adr) {
		// keyboard
		case 0xffb0:
			comp->keyb->map[7] &= ~0x40;
			comp->keyb->map[7] |= (val & 0x40);
			break;
		case 0xffb1: break;
		// pal/timer/scrbuf
		case 0xffb2: break;
		case 0xffb3: comp->vid->paln = (val << 2) & 0x3c;
			comp->cpu->timer.on = (val & 0x40) ? 1 : 0;
			comp->vid->curscr = (val & 0x80) ? 1 : 0;
			break;
		// scroll
		case 0xffb4: comp->vid->sc.y = val;
			break;
		case 0xffb5:
			comp->vid->cutscr = (val & 0x02) ? 0 : 1;
			break;
		case 0xffcc:
		case 0xffcd: break;
		case 0xffce: comp->reg[0xce] = val; break;
		case 0xffcf:
			if (val & 8) {	// extend
				comp->reg[0] = (val >> 4) & 7;
				if (val & 1) {
					comp->reg[1] = 0x80;
				} else if (val & 2) {
					comp->reg[1] = 0x81;
				} else {
					comp->reg[1] = val & 7;
				}
				bk_mem_map(comp);
			} else {	// tape control

			}
			break;
		default:
			printf("%.4X : wr %.4X,%.2X\n",comp->cpu->pc,adr,val);
			comp->brk = 1;
			break;
	}
}

void bk_mem_map(Computer* comp) {
	memSetBank(comp->mem, 0x00, MEM_RAM, 6, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0x40, MEM_RAM, comp->reg[0] & 7, MEM_16K, NULL, NULL, NULL);
	if (comp->reg[1] & 0x80) {
		memSetBank(comp->mem, 0x80, MEM_ROM, comp->reg[1] & 3, MEM_16K, NULL, NULL, NULL);
	} else {
		memSetBank(comp->mem, 0x80, MEM_RAM, comp->reg[1] & 7, MEM_16K, NULL, NULL, NULL);
	}
	memSetBank(comp->mem, 0xc0, MEM_ROM, 2, MEM_8K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0xe0, MEM_EXT, 3, MEM_8K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0xff, MEM_IO, 0, MEM_256, bk_io_rd, bk_io_wr, comp);
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
	memSetSize(comp->mem, MEM_128K, MEM_32K);
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

/*
void bk_sync(Computer* comp, int ns) {
	if (!(comp->timer.flags & 1)) {
		comp->timer.cnt -= ns;
		while (comp->timer.cnt < 0) {
			comp->timer.cnt += comp->timer.per;
			if (comp->timer.flags & 0x10) {
				comp->timer.val--;
				if (comp->timer.val < 0) {
					if (comp->timer.flags & 0x04) {		// signal
						comp->timer.flags |= 0x80;
						// comp->cpu->intrq |= PDP_INT_TIMER;
					}
					if (comp->timer.flags & 2) {		// wraparound
						comp->timer.val = 0xffff;
					} else if (comp->timer.flags & 0x08) {	// one-shot (stop counter)
						comp->timer.flags &= ~0x10;
					} else {				// common
						comp->timer.val = comp->timer.ival;
					}
				}
			}
		}
	}
}
*/

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

static bkKeyCode bkeyTab[] = {
	{XKEY_1,'1'},{XKEY_2,'2'},{XKEY_3,'3'},{XKEY_4,'4'},{XKEY_5,'5'},
	{XKEY_6,'6'},{XKEY_7,'7'},{XKEY_8,'8'},{XKEY_9,'9'},{XKEY_0,'0'},
	{XKEY_Q,'q'},{XKEY_W,'w'},{XKEY_E,'e'},{XKEY_R,'r'},{XKEY_T,'t'},
	{XKEY_Y,'y'},{XKEY_U,'u'},{XKEY_I,'i'},{XKEY_O,'o'},{XKEY_P,'p'},
	{XKEY_A,'a'},{XKEY_S,'s'},{XKEY_D,'d'},{XKEY_F,'f'},{XKEY_G,'g'},
	{XKEY_H,'h'},{XKEY_J,'j'},{XKEY_K,'k'},{XKEY_L,'l'},{XKEY_ENTER,10},
	{XKEY_Z,'z'},{XKEY_X,'x'},{XKEY_C,'c'},{XKEY_V,'v'},{XKEY_B,'b'},
	{XKEY_N,'n'},{XKEY_M,'m'},{XKEY_SPACE,' '},
	{XKEY_BSP,24},
	{XKEY_TAB,13},
	{XKEY_DOWN,27},{XKEY_LEFT,8},{XKEY_RIGHT,25},{XKEY_UP,26},
	{ENDKEY, 0}
};

void bk_keyp(Computer* comp, keyEntry xkey) {
	int idx = 0;
	switch(xkey.key) {
		case XKEY_PGUP: vidSetMode(comp->vid, VID_BK_COL); break;
		case XKEY_PGDN: vidSetMode(comp->vid, VID_BK_BW); break;
	}
	while ((bkeyTab[idx].xkey != ENDKEY) && (bkeyTab[idx].xkey != xkey.key))
		idx++;
	if (bkeyTab[idx].xkey != ENDKEY) {
		comp->keyb->map[7] |= 0x20;
		if (!(comp->keyb->map[7] & 0x80)) {
			comp->keyb->map[0] = bkeyTab[idx].code;
			comp->keyb->map[7] |= 0x80;
//			printf("code %.2X\n",comp->keyb->map[0]);
			if (!(comp->keyb->map[7] & 0x40)) {		// keyboard interrupt enabled
				comp->cpu->intvec = 0x30;		// 060
				comp->cpu->intrq |= PDP_INT_VIRQ;
//				printf("intrq %X\n", comp->cpu->intvec);
			}
		}
	}
}

void bk_keyr(Computer* comp, keyEntry xkey) {
	comp->keyb->map[7] &= ~0x20;	// 0x20 | 0x80
}
