#include <string.h>

#include "hardware.h"
#include "../cpu/1801vm1/1801vm1.h"

// BK0010
// dot: 25.175MHz (~40 ns/dot)
// timer: cpu/128

#define reg00	reg[0]
#define reg01	reg[1]
#define reg02	reg[2]
#define regCE	reg[0xce]
#define regB2	reg[0xb2]

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

// fdc

int bk_fdc_rd(Computer* comp, int adr) {
	return difIn(comp->dif, (adr & 2) >> 1, NULL, 0) & 0xffff;
	// return 0;
}

void bk_fdc_wr(Computer* comp, int adr, int val) {
	difOut(comp->dif, (adr & 2) >> 1, 0, val);
}

// keyboard

// 177660
int bk_kbf_rd(Computer* comp, int adr) {
	return (comp->keyb->drq << 7) | (!comp->keyb->inten << 6);
	// return 0;
}

void bk_kbf_wr(Computer* comp, int adr, int val) {
	comp->keyb->inten = !(val & 0x40);
}

// ffb2 (177662)
int bk_kbd_rd(Computer* comp, int adr) {
	return kbd_rd(comp->keyb, 0) & 0x7f; // comp->keyb->keycode & 0x7f;
//	comp->keyb->drq = 0;
	//return 0;
}

void bk11_kbd_wr(Computer* comp, int adr, int val) {
	comp->regB2 = (val >> 8) & 0xff;
// b8-11 = palette
	comp->vid->paln = ((val >> 6) & 0x3c);	// to bits 2..5
// b14: system timer off (vblank 50Hz ? ) = regB2.bit6
//	if (val & 0x4000) {
//		comp->cpu->inten &= ~PDP_INT_IRQ2;
//	} else {
//		comp->cpu->inten |= PDP_INT_IRQ2;
//	}
// b15: 0:scr.page5, 1:scr.page6
	comp->vid->curscr = !!(val & 0x8000);
}

// scroller

int bk_scr_rd(Computer* comp, int adr) {
	int r = comp->vid->sc.y & 0x00ff;
	if (!comp->vid->cutscr)
		r |= 0x200;
	return r;
}

void bk_scr_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->regNOD & 1) {
		comp->vid->sc.y = val & 0xff;
	}
	if (comp->cpu->regNOD & 2) {
		comp->vid->cutscr = !(val & 0x0200);
	}
}

// pc/psw

int bk_str_rd(Computer* comp, int adr) {
	return (adr & 2) ? comp->cpu->reg177676 : comp->cpu->reg177674;
	// comp->wdata = (comp->iomap[adr & ~1] & 0xff) | ((comp->iomap[adr | 1] << 8) & 0xff00);
	//return 0;
}

// timer

int bk_tiv_rd(Computer* comp, int adr) {
	return comp->cpu->timer.ival.w;
	//return 0;
}

int bk_tva_rd(Computer* comp, int adr) {
	return comp->cpu->timer.val.w;
	// return 0;
}

int bk_tfl_rd(Computer* comp, int adr) {
	return (comp->cpu->timer.flag & 0xff) | 0xff00;
	//return 0;
}

void bk_tiv_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->regNOD & 1)
		comp->cpu->timer.ival.l = val & 0xff;
	if (comp->cpu->regNOD & 2)
		comp->cpu->timer.ival.h = (val >> 8) & 0xff;
}

void bk_tva_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->regNOD & 1)
		comp->cpu->timer.val.l = val & 0xff;
	if (comp->cpu->regNOD & 2)
		comp->cpu->timer.val.h = (val >> 8) & 0xff;
}

void bk_tfl_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->regNOD & 1) {
		comp->cpu->timer.flag = val & 0xff;
		comp->cpu->timer.per = 128;
		if (val & 0x40) comp->cpu->timer.per <<= 2;	// div 4
		if (val & 0x20) comp->cpu->timer.per <<= 4;	// div 16
		if (val & 0x12) comp->cpu->timer.val = comp->cpu->timer.ival;	// reload
	}
}

// external

int bk_fcc_rd(Computer* comp, int adr) {
	return 0;
}

// 177776: system
int bk_sys_rd(Computer* comp, int adr) {
	int r = 0x8000;			// 8000 for 0010
	// r |= 0x80;		// TL ready
	if (comp->regCE) {		// b2: write to system port flag
		r |= 4;
		comp->regCE = 0;	// reset on reading
	}
	// b5: tape signal
	if (comp->tape->on && !comp->tape->rec && (comp->tape->volPlay & 0x80)) {
		r |= 0x20;
	}
	// b6: key pressed
	if (!comp->keyb->kpress) {
		r |= 0x40;		// = 0 if any key pressed, 1 if not
	}
	// b7: TL ready
	return r;
}

// b2,5,6 = tape signal (msb b6)
// b4: TL in/out
// b6: beeper
void bk_sys_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->regNOD & 1) {
		// b7: tape motor control (1:stop, 0:play)
		if (!(val & 0x80) && !comp->tape->on) {
			tapPlay(comp->tape);
		} else if ((val & 0x80) && comp->tape->on && !comp->tape->rec) {
			tapStop(comp->tape);
		}
		// b6 : beep
		comp->beep->lev = (val & 0x40) ? 1 : 0;
		// b6 : tape rec (main)
		comp->tape->levRec = (val & 0x40) ? 1 : 0;
		// b4: TL write
		comp->regCE = 1;	// write to system port
	}
}

// system port for 0011

// 177716: system
int bk11_sys_rd(Computer* comp, int adr) {
	int r = 0xc000;
	// comp->wdata |= 0x80;		// TL ready
	if (comp->regCE) {		// b2: write to system port flag
		r |= 4;
		comp->regCE = 0;	// reset on reading
	}
	// b5: tape signal
	if (comp->tape->on && !comp->tape->rec && (comp->tape->volPlay & 0x80)) {
		r |= 0x20;
	}
	// b6: key pressed
	if (!comp->keyb->kpress) {
		r |= 0x40;		// = 0 if any key pressed, 1 if not
	}
	// b7: TL ready
	return r;
}

// b2,5,6 = tape signal (msb b6)
// b4: TL in/out
// b6: beeper

void bk11_mem_map(Computer* comp);
void bk11_sys_wr(Computer* comp, int adr, int val) {
	if (val & (1 << 11)) {			// b11 set
		// printf("rom bits: %i.%i.x.%i.%i\n",!!(val & 0x10),!!(val & 8),!!(val & 2), !!(val & 1));
		if (val & 0x1b) {			// b0,1,3,4: rom 0,1,2,3 @ #8000
			if (val & 0x01) comp->reg01 = 0x80;
			if (val & 0x02) comp->reg01 = 0x81;
			if (val & 0x08) comp->reg01 = 0x82;
			if (val & 0x10) comp->reg01 = 0x83;
		} else {
			comp->reg01 = (val >> 8) & 7;		// ram b8,9,10 @ #8000 (reg01.b7=0:ram)
		}
		comp->reg02 = (val >> 12) & 7;			// ram b12,13,14 @ #4000
		bk11_mem_map(comp);
	} else if (comp->cpu->regNOD == 3) {
		// b7: tape motor control (1:stop, 0:play)
		if (!(val & 0x80) && !comp->tape->on) {
			tapPlay(comp->tape);
		} else if ((val & 0x80) && comp->tape->on && !comp->tape->rec) {
			tapStop(comp->tape);
		}
		// b6 : beep
		comp->beep->lev = (val & 0x40) ? 1 : 0;
		// b6 : tape rec (main)
		comp->tape->levRec = (val & 0x40) ? 1 : 0;
		// b4: TL write
		comp->regCE = 1;	// write to system port
	}
}

// * debug

int bk_dbg_rd(Computer* comp, int adr) {
	return 0xffff;
//	printf("%.4X : rd %.4X\n",comp->cpu->preg[7], adr);
//	assert(0);
	//return -1;
}

void bk_dbg_wr(Computer* comp, int adr, int val) {
//	printf("%.4X : wr %.4X, %.4X (nod = %i)\n",comp->cpu->preg[7], adr, val, comp->cpu->nod);
//	assert(0);
}

static xPort bk_io_tab[] = {
	{0xfffc, 0xfe58, 2, 2, 2, bk_fdc_rd, bk_fdc_wr},	// 177130..32:fdc
	{0xfffe, 0xffb0, 2, 2, 2, bk_kbf_rd, bk_kbf_wr},	// 177660: keyflag
	{0xfffe, 0xffb2, 2, 2, 2, bk_kbd_rd, NULL},		// 177662: keycode / video ctrl
	{0xfffe, 0xffb4, 2, 2, 2, bk_scr_rd, bk_scr_wr},	// 177664: scroller
	{0xfffc, 0xffbc, 2, 2, 2, bk_str_rd, NULL},		// 177674/6: storage (pc/psw)
	{0xfffe, 0xffc6, 2, 2, 2, bk_tiv_rd, bk_tiv_wr},	// 177706: timer
	{0xfffe, 0xffc8, 2, 2, 2, bk_tva_rd, bk_tva_wr},	// 177710
	{0xfffe, 0xffca, 2, 2, 2, bk_tfl_rd, bk_tfl_wr},	// 177712
	{0xfffe, 0xffcc, 2, 2, 2, bk_fcc_rd, NULL},		// 177714: ext (printer / ay / joystick)
	{0xfffe, 0xffce, 2, 2, 2, bk_sys_rd, bk_sys_wr},	// 177716: system
	{0x0000, 0x0000, 2, 2, 2, bk_dbg_rd, bk_dbg_wr}
};

static xPort bk11_io_tab[] = {
	{0xfffc, 0xfe58, 2, 2, 2, bk_fdc_rd, bk_fdc_wr},	// 177130..32:fdc
	{0xfffe, 0xffb0, 2, 2, 2, bk_kbf_rd, bk_kbf_wr},	// 177660: keyflag
	{0xfffe, 0xffb2, 2, 2, 2, bk_kbd_rd, bk11_kbd_wr},	// 177662: keycode / video ctrl
	{0xfffe, 0xffb4, 2, 2, 2, bk_scr_rd, bk_scr_wr},	// 177664: scroller
	{0xfffc, 0xffbc, 2, 2, 2, bk_str_rd, NULL},		// 177674/6: storage (pc/psw)
	{0xfffe, 0xffc6, 2, 2, 2, bk_tiv_rd, bk_tiv_wr},	// 177706: timer
	{0xfffe, 0xffc8, 2, 2, 2, bk_tva_rd, bk_tva_wr},	// 177710
	{0xfffe, 0xffca, 2, 2, 2, bk_tfl_rd, bk_tfl_wr},	// 177712
	{0xfffe, 0xffcc, 2, 2, 2, bk_fcc_rd, NULL},		// 177714: ext (printer / ay / joystick)
	{0xfffe, 0xffce, 2, 2, 2, bk11_sys_rd, bk11_sys_wr},	// 177716: system
	{0x0000, 0x0000, 2, 2, 2, bk_dbg_rd, bk_dbg_wr}
};

// cpu allways read whole word from even adr
int bk_io_rd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	adr &= ~1;
	return hwIn(bk_io_tab, comp, adr);
	// return comp->wdata;
}

int bk11_io_rd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	adr &= ~1;
	return hwIn(bk11_io_tab, comp, adr);
	// return comp->wdata;
}

void bk_io_wr(int adr, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
//	if (comp->cpu->regNOD & 1)		// LSB
//		comp->iomap[(adr & ~1) & 0xffff] = val & 0xff;
//	if (comp->cpu->regNOD & 2)		// MSB
//		comp->iomap[(adr | 1) & 0xffff] = (val >> 8) & 0xff;
	hwOut(bk_io_tab, comp, adr & ~1, val, 1);
}

void bk11_io_wr(int adr, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
//	if (comp->cpu->regNOD & 1)		// LSB
//		comp->iomap[(adr & ~1) & 0xffff] = val & 0xff;
//	if (comp->cpu->regNOD & 2)		// MSB
//		comp->iomap[(adr | 1) & 0xffff] = (val >> 8) & 0xff;
	hwOut(bk11_io_tab, comp, adr & ~1, val, 1);
}

int bk_ram_rd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	int res;
	adr &= ~1;
	int fadr = mem_get_phys_adr(comp->mem, adr);	// = (comp->mem->map[(adr >> 8) & 0xff].num << 8) | (adr & 0xff);
	comp->cpu->t += 3;
	res = comp->mem->ramData[fadr & comp->mem->ramMask] & 0xff;
	fadr++;
	res |= (comp->mem->ramData[fadr & comp->mem->ramMask] << 8) & 0xff00;
	return res;
}

void bk_ram_wr(int adr, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	comp->cpu->t += 2;
	int fadr = mem_get_phys_adr(comp->mem, adr);	// = comp->mem->map[(adr >> 8) & 0xff].num << 8) | (adr & 0xff);
	if (comp->cpu->regNOD & 1)
		comp->mem->ramData[(fadr & ~1) & comp->mem->ramMask] = val & 0xff;
	if (comp->cpu->regNOD & 2)
		comp->mem->ramData[(fadr | 1) & comp->mem->ramMask] = (val >> 8) & 0xff;
}

int bk_rom_rd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	int res;
	adr &= ~1;
	int fadr = mem_get_phys_adr(comp->mem, adr);	// = comp->mem->map[(adr >> 8) & 0xff].num << 8) | (adr & 0xff);
	comp->cpu->t += 2;
	res = comp->mem->romData[fadr & comp->mem->romMask] & 0xff;
	fadr++;
	res |= (comp->mem->romData[fadr & comp->mem->romMask] << 8) & 0xff00;
	return res;
}

void bk_rom_wr(int adr, int val, void* ptr) {
	// nothing to do
}

void bk_sync(Computer* comp, int ns) {
//	if ((comp->vid->newFrame) && (comp->regB2 & 0x40)) {	// b14,177662 = 50Hz int
//		comp->cpu->intrq |= PDP_INT_IRQ2;
//	}
	tapSync(comp->tape, ns);
	bcSync(comp->beep, ns);
	difSync(comp->dif, ns);
}

// bk0010
// 0000: ram
// 4000: ram screen
// 8000: basic rom
// c000: system rom
// fe00: io

void bk_mem_map(Computer* comp) {
	memSetBank(comp->mem, 0x00, MEM_RAM, 6, MEM_16K, bk_ram_rd, bk_ram_wr, comp);		// page 6 (0)
	memSetBank(comp->mem, 0x40, MEM_RAM, 1, MEM_16K, bk_ram_rd, bk_ram_wr, comp);		// page 1 : scr 0
	memSetBank(comp->mem, 0x80, MEM_ROM, 0, MEM_32K, bk_rom_rd, bk_rom_wr, comp);
	memSetBank(comp->mem, 0xff, MEM_IO, 0xff, MEM_256, bk_io_rd, bk_io_wr, comp);
}

// bk0011
// 0000: ram
// 4000: ram window 0
// 8000: ram window 1 / rom
// c000: system rom (bos)
// e000: extend rom (fdd)
// fe00: io

// port 177716, wr with b11=1
// b12-14: ram window 0 (4000, 16K)
// b08-10: ram window 1 (8000, 16K)
// b0,1[,3,4,5,6]: rom page (8000, 16K)

void bk11_mem_map(Computer* comp) {
	memSetBank(comp->mem, 0x00, MEM_RAM, 6, MEM_16K, bk_ram_rd, bk_ram_wr, comp);		// page 6 (0) @ 0x0000
	memSetBank(comp->mem, 0x40, MEM_RAM, comp->reg02 & 7, MEM_16K, bk_ram_rd, bk_ram_wr, comp);	// ram page @ 0x4000
	if (comp->reg01 & 0x80) {									// ram/rom @ 0x8000
		memSetBank(comp->mem, 0x80, MEM_ROM, comp->reg01 & 3, MEM_16K, bk_rom_rd, bk_rom_wr, comp);
	} else {
		memSetBank(comp->mem, 0x80, MEM_RAM, comp->reg01 & 7, MEM_16K,  bk_ram_rd, bk_ram_wr, comp);
	}
	memSetBank(comp->mem, 0xc0, MEM_ROM, 4, MEM_8K,  bk_rom_rd, bk_rom_wr, comp);		// monitor
	if (comp->dif->type == DIF_SMK512) {
		memSetBank(comp->mem, 0xe0, MEM_ROM, 6, MEM_8K,  bk_rom_rd, bk_rom_wr, comp);			// disk interface rom (page 3, 8K)
		memSetBank(comp->mem, 0xfe, MEM_IO, 0xfe, MEM_512, bk11_io_rd, bk11_io_wr, comp);		// 0170000..0177776 with disk interface
	} else {
		memSetBank(comp->mem, 0xe0, MEM_EXT, 7, MEM_8K,  NULL, NULL, NULL);				// empty space
		memSetBank(comp->mem, 0xff, MEM_IO, 0xff, MEM_256, bk11_io_rd, bk11_io_wr, comp);		// 0177600..0177776 without disk interface
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
	memSetSize(comp->mem, MEM_32K, MEM_32K);
	for (int i = 0; i < 0x40; i++) {
		vid_set_col(comp->vid, i, bk_pal[i]);
	}
	comp->reg00 = 1;
	comp->reg01 = 0x80;
	comp->regB2 = 0x40;
	cpu_reset(comp->cpu);
	comp->vid->curscr = 0;
	comp->vid->paln = 0;
	vid_set_mode(comp->vid, VID_BK_BW);
	kbd_reset(comp->keyb);
	bk_mem_map(comp);
}

void bk11_reset(Computer* comp) {
	memSetSize(comp->mem, MEM_128K, MEM_64K);
	for (int i = 0; i < 0x40; i++) {
		vid_set_col(comp->vid, i, bk_pal[i]);
	}
	comp->reg00 = 1;
	comp->reg01 = 0x80;
	cpu_reset(comp->cpu);
//	comp->vid->curscr = 0;
	comp->vid->paln = 0;
	vid_set_mode(comp->vid, VID_BK_BW);
	kbd_reset(comp->keyb);
//	comp->keyb->kpress = 0;
//	comp->keyb->inten = 0;
//	comp->keyb->drq = 0;
//	comp->keyb->keycode = 0x00;
	bk11_mem_map(comp);
}

void bk_mwr(Computer* comp, int adr, int val) {
	memWr(comp->mem, adr, val);
}

int bk_mrd(Computer* comp, int adr, int m1) {
	return memRd(comp->mem, adr);
}

// only for sending control signals (like INIT)
void bk_irq(Computer* comp, int val) {
	switch (val) {
		case PDP11_INIT:
			// comp->keyb->inten = 0;
			comp->keyb->drq = 0;
			break;
		case IRQ_VID_FRAME:
			if (!(comp->regB2 & 0x40)) {
				comp->cpu->intrq |= PDP_INT_IRQ2;
			}
			break;
		case IRQ_KBD_DATA:		// key was pressed & kbd interrupt enabled
			comp->cpu->intvec = comp->keyb->ar2 ? 0274 : 060;
			comp->cpu->intrq |= PDP_INT_VIRQ;
			break;
	}
}

sndPair bk_vol(Computer* comp, sndVolume* sv) {
	sndPair vol;
	int lev = comp->beep->val * sv->beep / 6;
	if (comp->tape->rec) {
		lev += comp->tape->levRec ? 0x1000 * sv->tape / 100 : 0;
	} else {
		lev += (comp->tape->volPlay << 8) * sv->tape / 1600;
	}
	vol.left = lev;
	vol.right = lev;
	return vol;
}

void bk_init(Computer* comp) {
	fdc_set_hd(comp->dif->fdc, 0);
	vid_upd_timings(comp->vid, 200);	// 302
	kbd_set_type(comp->keyb, KBD_BK);
}

// keys


static char bkcapson[] = " caps on ";
static char bkcapsoff[] = " caps off ";
static char bkkbdrus[] = " rus ";
static char bkkbdlat[] = " lat ";
static char bkvidcol[] = " color mode ";
static char bkvidbw[] = " b/w mode ";

void bk_keyp(Computer* comp, keyEntry* xkey) {
	int code = 0;
	switch(xkey->key) {
		case XKEY_PGDN:
			switch(comp->vid->vmode) {
				case VID_BK_BW:
					comp->msg = bkvidcol;
					vid_set_mode(comp->vid, VID_BK_COL);
					break;
				case VID_BK_COL:
					comp->msg = bkvidbw;
					vid_set_mode(comp->vid, VID_BK_BW);
					break;
			}
			break;
		case XKEY_CAPS:
			comp->msg = !comp->keyb->caps ? bkcapson : bkcapsoff;
			break;
		case XKEY_LCTRL:
			comp->msg = !comp->keyb->lang ? bkkbdrus : bkkbdlat;
			break;
	}
	comp->keyb->arg = code;
	kbd_press(comp->keyb, xkey);
}

void bk_keyr(Computer* comp, keyEntry* xkey) {
	kbd_release(comp->keyb, xkey);
}

static vLayout bkLay = {{256+96,256+24},{0,0},{96,24},{256,256},{0,0},0};

HardWare b10_hw_core = {HW_BK0010,HWG_BK,"BK0010","BK0010",8,MEM_32K,(double)29/23,&bkLay,16,NULL,
			bk_init,bk_mem_map,NULL,NULL,bk_mrd,bk_mwr,bk_irq,NULL,bk_reset,bk_sync,bk_keyp,bk_keyr,bk_vol};
HardWare b11_hw_core = {HW_BK0011M,HWG_BK,"BK0011M","BK0011M",8,MEM_128K,(double)29/23,&bkLay,16,NULL,
			bk_init,bk11_mem_map,NULL,NULL,bk_mrd,bk_mwr,bk_irq,NULL,bk11_reset,bk_sync,bk_keyp,bk_keyr,bk_vol};
