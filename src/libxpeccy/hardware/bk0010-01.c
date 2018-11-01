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
			if (!comp->vid->curscr) comp->wdata |= 0x200;
			break;	// vertical scroll register
		case 0xffc6: comp->wdata = comp->timer.ival & 0xffff; break;	// timer:initial counter value
		case 0xffc8: comp->wdata = comp->timer.val & 0xffff; break;	// timer:current counter
		case 0xffca: comp->wdata = (comp->timer.flags & 0xff) | 0xff00;
			break;	// timer flags
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
		// scroll
		case 0xffb4: comp->vid->sc.y = val;
			break;
		case 0xffb5:
			comp->vid->curscr = (val & 0x02) ? 0 : 1;
			break;
		// timer
		case 0xffc6: comp->timer.ival &= 0xff00;
			comp->timer.ival |= (val & 0xff);
			break;
		case 0xffc7: comp->timer.ival &= 0xff;
			comp->timer.ival |= (val << 8) & 0xff00;
			break;
		case 0xffc8: comp->timer.val &= 0xff00;
			comp->timer.val |= (val & 0xff);
			break;
		case 0xffc9: comp->timer.val &= 0xff;
			comp->timer.val |= (val << 8) & 0xff00;
			break;
		case 0xffca: comp->timer.flags = val & 0xff;
			comp->timer.per = comp->timer.bper;
			if (val & 0x40) comp->timer.per *= 4;	// div 4
			if (val & 0x20) comp->timer.per *= 16;	// div 16
			if (val & 0x12) comp->timer.val = comp->timer.ival; // reload value
			break;
		case 0xffcb: break;
		// register
		case 0xffcc:
		case 0xffcd: break;
		case 0xffce:
		case 0xffcf: break;
		default:
			printf("%.4X : wr %.4X,%.2X\n",comp->cpu->pc,adr,val);
			comp->brk = 1;
			break;
	}
}

void bk_mem_map(Computer* comp) {
	memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_32K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0x80, MEM_ROM, 0, MEM_32K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0xff, MEM_IO, 0, MEM_256, bk_io_rd, bk_io_wr, comp);
}

static xColor bk_pal[8] = {
	{0,0,0},{255,255,255},{255,255,255},{255,255,255},
	{0,0,0},{0,0,255},{0,255,0},{255,0,0}
};

void bk_reset(Computer* comp) {
	memset(comp->mem->ramData, 0x00, MEM_256);
	for (int i = 0; i < 8; i++) {
		comp->vid->pal[i] = bk_pal[i];
	}
	comp->cpu->reset(comp->cpu);
	vidSetMode(comp->vid, VID_BK_BW);
	comp->keyb->map[7] = 0x40;
	comp->keyb->map[0] = 0;
	comp->timer.flags = 0x01;
}

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
						comp->cpu->intrq |= PDP_INT_TIMER;
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
