#include <string.h>

#include "hardware.h"

#define ENABLE_BK0011	1

// BK0010
// dot: 25.175MHz (~40 ns/dot)
// timer: cpu/128

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

int bk11_fdc_rd(Computer* comp, int adr) {
	comp->wdata = difIn(comp->dif, (adr & 2) >> 1, NULL, 0) & 0xffff;
	return 0;
}

void bk11_fdc_wr(Computer* comp, int adr, int val) {
	// difOut(comp->dif, (adr & 2) ? 1 : 0, 0, comp->wdata);
}

// keyboard

int bk11_kbf_rd(Computer* comp, int adr) {
	comp->wdata = comp->keyb->flag & 0xc0;
	return 0;
}

void bk11_kbf_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->nod & 1) {
		comp->keyb->flag &= ~0x40;
		comp->keyb->flag |= (val & 0x40);
	}
}

// ffb2 (1777662)
int bk11_kbd_rd(Computer* comp, int adr) {
	comp->wdata = (comp->reg[0xb2] << 8) | (comp->keyb->keycode & 0x7f);
//	comp->wdata = comp->keyb->keycode & 0x7f;
	comp->keyb->flag &= 0x7f;		// reset b7,flag
	return 0;
}

void bk11_kbd_wr(Computer* comp, int adr, int val) {
#if ENABLE_BK0011
	comp->reg[0xb2] = (val >> 8) & 0xff;
// b9-12 = palette
	comp->vid->paln = (val >> 9) & 0x0f;
// b14: 0=enable 48.5Hz timer with interrupt 100
// b15: 0=scr5,1=scr6
	comp->vid->curscr = (val & 0x8000) ? 0 : 1;
#endif
}

// scroller

int bk11_scr_rd(Computer* comp, int adr) {
	comp->wdata = comp->vid->sc.y & 0x00ff;
	if (!comp->vid->cutscr)
		comp->wdata |= 0x200;
	return 0;
}

void bk11_scr_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->nod & 1) {
		comp->vid->sc.y = val & 0xff;
	}
	if (comp->cpu->nod & 2) {
		comp->vid->cutscr = (val & 0x0200) ? 0 : 1;
	}
}

// pc/psw

int bk11_str_rd(Computer* comp, int adr) {
	comp->wdata = (comp->iomap[adr & ~1] & 0xff) | ((comp->iomap[adr | 1] << 8) & 0xff00);
	return 0;
}

// timer

int bk11_tiv_rd(Computer* comp, int adr) {
	comp->wdata = comp->cpu->timer.ival;
	return 0;
}

int bk11_tva_rd(Computer* comp, int adr) {
	comp->wdata = comp->cpu->timer.val;
	return 0;
}

int bk11_tfl_rd(Computer* comp, int adr) {
	comp->wdata = (comp->cpu->timer.flag & 0xff) | 0xff00;
	return 0;
}

void bk11_tiv_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->nod & 1)
		comp->cpu->timer.ivl = val & 0xff;
	if (comp->cpu->nod & 2)
		comp->cpu->timer.ivh = (val >> 8) & 0xff;
}

void bk11_tva_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->nod & 1)
		comp->cpu->timer.vl = val & 0xff;
	if (comp->cpu->nod & 2)
		comp->cpu->timer.vh = (val >> 8) & 0xff;
}

void bk11_tfl_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->nod & 1) {
		comp->cpu->timer.flag = val & 0xff;
		comp->cpu->timer.per = 128;
		if (val & 0x40) comp->cpu->timer.per <<= 2;	// div 4
		if (val & 0x20) comp->cpu->timer.per <<= 4;	// div 16
		if (val & 0x12) comp->cpu->timer.val = comp->cpu->timer.ival;	// reload
	}
}

// external

int bk11_fcc_rd(Computer* comp, int adr) {
	comp->wdata = 0;
	return 0;
}

// 177776: system
int bk11_sys_rd(Computer* comp, int adr) {
	comp->wdata = 0xc000;
	// comp->wdata |= 0x80;		// TL ready
	if (comp->reg[0xce]) {		// b2: write to system port flag
		comp->wdata |= 4;
		comp->reg[0xce] = 0;	// reset on reading
	}
	// b5: tape signal
	if (comp->tape->on && !comp->tape->rec && (comp->tape->volPlay & 0x80)) {
		comp->wdata |= 0x20;
	}
	// b6: key pressed
	if (!(comp->keyb->flag & 0x20)) {
		comp->wdata |= 0x40;		// = 0 if any key pressed, 1 if not
	}
	// b7: TL ready
	return 0;
}

// b2,5,6 = tape signal (msb b6)
// b4: TL in/out
// b6: beeper
void bk11_sys_wr(Computer* comp, int adr, int val) {
	if (val & 0x800) {			// b11 set
		if (val & 0x1b) {			// b0,1,3,4: rom 0,1,2,3 @ #8000
			if (val & 0x01) comp->reg[1] = 0x80;
			if (val & 0x02) comp->reg[1] = 0x81;
			if (val & 0x08) comp->reg[1] = 0x82;
			if (val & 0x10) comp->reg[1] = 0x83;
		} else {
			comp->reg[1] = (val >> 8) & 7;		// ram b8,9,10 @ #8000 (reg[1].b7=0:ram)
		}
		comp->reg[2] = (val >> 12) & 7;			// ram b12,13,14 @ #4000
		bk11_mem_map(comp);
	} else if (comp->cpu->nod & 1) {
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
		comp->reg[0xce] = 1;	// write to system port
	}
}

// * debug

int bk11_dbg_rd(Computer* comp, int adr) {
	comp->wdata = 0xffff;
	printf("%.4X : rd %.4X\n",comp->cpu->preg[7], adr);
//	assert(0);
	return -1;
}

void bk11_dbg_wr(Computer* comp, int adr, int val) {
	printf("%.4X : wr %.4X, %.4X (nod = %i)\n",comp->cpu->preg[7], adr, val, comp->cpu->nod);
//	assert(0);
}

static xPort bk11_io_tab[] = {
	{0xfffc, 0xfe58, 2, 2, 2, bk11_fdc_rd, bk11_fdc_wr},	// 177130..32:fdc
	{0xfffe, 0xffb0, 2, 2, 2, bk11_kbf_rd, bk11_kbf_wr},	// 177660: keyflag
	{0xfffe, 0xffb2, 2, 2, 2, bk11_kbd_rd, bk11_kbd_wr},	// 177662: keycode / video ctrl
	{0xfffe, 0xffb4, 2, 2, 2, bk11_scr_rd, bk11_scr_wr},	// 177664: scroller
	{0xfffc, 0xffbc, 2, 2, 2, bk11_str_rd, NULL},		// 177704: storage (pc/psw)
	{0xfffe, 0xffc6, 2, 2, 2, bk11_tiv_rd, bk11_tiv_wr},	// 177706: timer
	{0xfffe, 0xffc8, 2, 2, 2, bk11_tva_rd, bk11_tva_wr},	// 177710
	{0xfffe, 0xffca, 2, 2, 2, bk11_tfl_rd, bk11_tfl_wr},	// 177712
	{0xfffe, 0xffcc, 2, 2, 2, bk11_fcc_rd, NULL},		// 177714: ext (printer / ay / joystick)
	{0xfffe, 0xffce, 2, 2, 2, bk11_sys_rd, bk11_sys_wr},	// 177716: system
//	{0x0000, 0x0000, 2, 2, 2, bk11_dbg_rd, bk11_dbg_wr}
};

// cpu allways read whole word from even adr
int bk11_io_rd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	adr &= ~1;
	hwIn(bk11_io_tab, comp, adr);
	return comp->wdata;
}

// if cpu->nod = 1, write 1 byte immediately
// if cpu->nod = 0:
//	even adr : store low byte in wdata
//	odd adr : is high byte, add stored low byte, write whole word
void bk11_io_wr(int adr, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	if (comp->cpu->nod & 1)		// LSB
		comp->iomap[(adr & ~1) & 0xffff] = val & 0xff;
	if (comp->cpu->nod & 2)		// MSB
		comp->iomap[(adr | 1) & 0xffff] = (val >> 8) & 0xff;
	hwOut(bk11_io_tab, comp, adr & ~1, val, 1);
}

int bk11_ram_rd(int adr, void* ptr) {
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

void bk11_ram_wr(int adr, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	comp->cpu->t += 2;
	int fadr = mem_get_phys_adr(comp->mem, adr);	// = comp->mem->map[(adr >> 8) & 0xff].num << 8) | (adr & 0xff);
	if (comp->cpu->nod & 1)
		comp->mem->ramData[(fadr & ~1) & comp->mem->ramMask] = val & 0xff;
	if (comp->cpu->nod & 2)
		comp->mem->ramData[(fadr | 1) & comp->mem->ramMask] = (val >> 8) & 0xff;
}

int bk11_rom_rd(int adr, void* ptr) {
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

void bk11_rom_wr(int adr, int val, void* ptr) {
	// nothing to do
}

void bk11_sync(Computer* comp, int ns) {
	if ((comp->vid->newFrame) && (comp->iomap[0xffb3] & 0x40)) {
		comp->cpu->intrq |= PDP_INT_IRQ2;
	}
	tapSync(comp->tape, ns);
	bcSync(comp->beep, ns);
	difSync(comp->dif, ns);
}

// 0000: ram
// 4000: ram window 0
// 8000: ram window 1 / rom
// c000: system rom
// fe00: io

// port 177716, wr with b11=1
// b12-14: ram window 0 (4000, 16K)
// b08-10: ram window 1 (8000, 16K)
// b0,1[,3,4,5,6]: rom page (8000, 16K)

void bk11_mem_map(Computer* comp) {
	memSetBank(comp->mem, 0x00, MEM_RAM, 6, MEM_16K, bk11_ram_rd, bk11_ram_wr, comp);		// page 6 (0)
// for 11
	memSetBank(comp->mem, 0x40, MEM_RAM, comp->reg[2] & 7, MEM_16K, bk11_ram_rd, bk11_ram_wr, comp);	// ram @ 0x4000
	if (comp->reg[1] & 0x80) {
		memSetBank(comp->mem, 0x80, MEM_ROM, comp->reg[1] & 3, MEM_16K, bk11_rom_rd, bk11_rom_wr, comp);
	} else {
		memSetBank(comp->mem, 0x80, MEM_RAM, comp->reg[1] & 7, MEM_16K,  bk11_ram_rd, bk11_ram_wr, comp);
	}
	memSetBank(comp->mem, 0xc0, MEM_ROM, 4, MEM_8K,  bk11_rom_rd, bk11_rom_wr, comp);
	if (comp->dif->type == DIF_SMK512) {
		memSetBank(comp->mem, 0xe0, MEM_ROM, 5, MEM_8K,  bk11_rom_rd, bk11_rom_wr, comp);			// disk interface rom
		memSetBank(comp->mem, 0xfe, MEM_IO, 0xfe, MEM_512, bk11_io_rd, bk11_io_wr, comp);		// 0170000..0177776 with disk interface
	} else {
		memSetBank(comp->mem, 0xe0, MEM_EXT, 7, MEM_8K,  NULL, NULL, NULL);			// empty space
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

static xColor bk11_pal[0x40] = {
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

void bk11_reset(Computer* comp) {
	memSetSize(comp->mem, MEM_128K, MEM_64K);
	for (int i = 0; i < 0x40; i++) {
		vid_set_col(comp->vid, i, bk11_pal[i]);
	}
	comp->reg[0] = 1;
	comp->reg[1] = 0x80;
	comp->cpu->reset(comp->cpu);
	comp->vid->curscr = 0;
	comp->vid->paln = 0;
	vid_set_mode(comp->vid, VID_BK_BW);
	comp->keyb->flag = 0x00;
	comp->keyb->keycode = 0;
	bk11_mem_map(comp);
}

void bk11_mwr(Computer* comp, int adr, int val) {
	memWr(comp->mem, adr, val);
}

int bk11_mrd(Computer* comp, int adr, int m1) {
	return memRd(comp->mem, adr);
}

// only for sending control signals (like INIT)
void bk11_iowr(Computer* comp, int adr, int val) {
	switch (val) {
		case PDP11_INIT:
			comp->keyb->flag = 0;
			break;
	}
}

void bk11_init(Computer* comp) {
	fdc_set_hd(comp->dif->fdc, 0);
	vid_upd_timings(comp->vid, 302);
}
