#include <string.h>

#include "hardware.h"

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

int bk_fdc_rd(Computer* comp, int adr) {
	comp->wdata = difIn(comp->dif, (adr & 2) >> 1, NULL, 0) & 0xffff;
	return 0;
}

void bk_fdc_wr(Computer* comp, int adr, int val) {
	// difOut(comp->dif, (adr & 2) ? 1 : 0, 0, comp->wdata);
}

// keyboard

int bk_kbf_rd(Computer* comp, int adr) {
	comp->wdata = comp->keyb->flag & 0xc0;
	return 0;
}

void bk_kbf_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->nod & 1) {
		comp->keyb->flag &= ~0x40;
		comp->keyb->flag |= (val & 0x40);
	}
}

// ffb2 (1777662)
int bk_kbd_rd(Computer* comp, int adr) {
	comp->wdata = (comp->reg[0xb2] << 8) | (comp->keyb->keycode & 0x7f);
//	comp->wdata = comp->keyb->keycode & 0x7f;
	comp->keyb->flag &= ~0x80;		// reset b7,flag
	return 0;
}

void bk11_kbd_wr(Computer* comp, int adr, int val) {
	comp->reg[0xb2] = (val >> 8) & 0xff;
// b9-12 = palette
	comp->vid->paln = (val >> 9) & 0x0f;
// b14: 0=enable 48.5Hz timer with interrupt 100
// b15: disable 'STOP' key interrupt
}

// scroller

int bk_scr_rd(Computer* comp, int adr) {
	comp->wdata = comp->vid->sc.y & 0x00ff;
	if (!comp->vid->cutscr)
		comp->wdata |= 0x200;
	return 0;
}

void bk_scr_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->nod & 1) {
		comp->vid->sc.y = val & 0xff;
	}
	if (comp->cpu->nod & 2) {
		comp->vid->cutscr = (val & 0x0200) ? 0 : 1;
	}
}

// pc/psw

int bk_str_rd(Computer* comp, int adr) {
	comp->wdata = (comp->iomap[adr & ~1] & 0xff) | ((comp->iomap[adr | 1] << 8) & 0xff00);
	return 0;
}

// timer

int bk_tiv_rd(Computer* comp, int adr) {
	comp->wdata = comp->cpu->timer.ival;
	return 0;
}

int bk_tva_rd(Computer* comp, int adr) {
	comp->wdata = comp->cpu->timer.val;
	return 0;
}

int bk_tfl_rd(Computer* comp, int adr) {
	comp->wdata = (comp->cpu->timer.flag & 0xff) | 0xff00;
	return 0;
}

void bk_tiv_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->nod & 1)
		comp->cpu->timer.ivl = val & 0xff;
	if (comp->cpu->nod & 2)
		comp->cpu->timer.ivh = (val >> 8) & 0xff;
}

void bk_tva_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->nod & 1)
		comp->cpu->timer.vl = val & 0xff;
	if (comp->cpu->nod & 2)
		comp->cpu->timer.vh = (val >> 8) & 0xff;
}

void bk_tfl_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->nod & 1) {
		comp->cpu->timer.flag = val & 0xff;
		comp->cpu->timer.per = 128;
		if (val & 0x40) comp->cpu->timer.per <<= 2;	// div 4
		if (val & 0x20) comp->cpu->timer.per <<= 4;	// div 16
		if (val & 0x12) comp->cpu->timer.val = comp->cpu->timer.ival;	// reload
	}
}

// external

int bk_fcc_rd(Computer* comp, int adr) {
	comp->wdata = 0;
	return 0;
}

// 177776: system
int bk_sys_rd(Computer* comp, int adr) {
	comp->wdata = 0x8000;		// 8000 for 0010
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
void bk_sys_wr(Computer* comp, int adr, int val) {
	if (comp->cpu->nod & 1) {
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

// system port for 0011

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
		// printf("rom bits: %i.%i.x.%i.%i\n",!!(val & 0x10),!!(val & 8),!!(val & 2), !!(val & 1));
		if (val & 0x1b) {			// b0,1,3,4: rom 0,1,2,3 @ #8000
			if (val & 0x01) comp->reg[1] = 0x80;
			if (val & 0x02) comp->reg[1] = 0x81;
			if (val & 0x08) comp->reg[1] = 0x82;
			if (val & 0x10) comp->reg[1] = 0x83;
		} else {
			comp->reg[1] = (val >> 8) & 7;		// ram b8,9,10 @ #8000 (reg[1].b7=0:ram)
		}
		comp->reg[2] = (val >> 12) & 7;			// ram b12,13,14 @ #4000
		comp->vid->curscr = !!(val & 0x04);
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

int bk_dbg_rd(Computer* comp, int adr) {
	comp->wdata = 0xffff;
	printf("%.4X : rd %.4X\n",comp->cpu->preg[7], adr);
//	assert(0);
	return -1;
}

void bk_dbg_wr(Computer* comp, int adr, int val) {
	printf("%.4X : wr %.4X, %.4X (nod = %i)\n",comp->cpu->preg[7], adr, val, comp->cpu->nod);
//	assert(0);
}

static xPort bk_io_tab[] = {
	{0xfffc, 0xfe58, 2, 2, 2, bk_fdc_rd, bk_fdc_wr},	// 177130..32:fdc
	{0xfffe, 0xffb0, 2, 2, 2, bk_kbf_rd, bk_kbf_wr},	// 177660: keyflag
	{0xfffe, 0xffb2, 2, 2, 2, bk_kbd_rd, NULL},		// 177662: keycode / video ctrl
	{0xfffe, 0xffb4, 2, 2, 2, bk_scr_rd, bk_scr_wr},	// 177664: scroller
	{0xfffc, 0xffbc, 2, 2, 2, bk_str_rd, NULL},		// 177704: storage (pc/psw)
	{0xfffe, 0xffc6, 2, 2, 2, bk_tiv_rd, bk_tiv_wr},	// 177706: timer
	{0xfffe, 0xffc8, 2, 2, 2, bk_tva_rd, bk_tva_wr},	// 177710
	{0xfffe, 0xffca, 2, 2, 2, bk_tfl_rd, bk_tfl_wr},	// 177712
	{0xfffe, 0xffcc, 2, 2, 2, bk_fcc_rd, NULL},		// 177714: ext (printer / ay / joystick)
	{0xfffe, 0xffce, 2, 2, 2, bk_sys_rd, bk_sys_wr},	// 177716: system
//	{0x0000, 0x0000, 2, 2, 2, bk_dbg_rd, bk_dbg_wr}
};

static xPort bk11_io_tab[] = {
	{0xfffc, 0xfe58, 2, 2, 2, bk_fdc_rd, bk_fdc_wr},	// 177130..32:fdc
	{0xfffe, 0xffb0, 2, 2, 2, bk_kbf_rd, bk_kbf_wr},	// 177660: keyflag
	{0xfffe, 0xffb2, 2, 2, 2, bk_kbd_rd, bk11_kbd_wr},	// 177662: keycode / video ctrl
	{0xfffe, 0xffb4, 2, 2, 2, bk_scr_rd, bk_scr_wr},	// 177664: scroller
	{0xfffc, 0xffbc, 2, 2, 2, bk_str_rd, NULL},		// 177704: storage (pc/psw)
	{0xfffe, 0xffc6, 2, 2, 2, bk_tiv_rd, bk_tiv_wr},	// 177706: timer
	{0xfffe, 0xffc8, 2, 2, 2, bk_tva_rd, bk_tva_wr},	// 177710
	{0xfffe, 0xffca, 2, 2, 2, bk_tfl_rd, bk_tfl_wr},	// 177712
	{0xfffe, 0xffcc, 2, 2, 2, bk_fcc_rd, NULL},		// 177714: ext (printer / ay / joystick)
	{0xfffe, 0xffce, 2, 2, 2, bk11_sys_rd, bk11_sys_wr},	// 177716: system
//	{0x0000, 0x0000, 2, 2, 2, bk_dbg_rd, bk_dbg_wr}
};

// cpu allways read whole word from even adr
int bk_io_rd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	adr &= ~1;
	hwIn(bk_io_tab, comp, adr);
	return comp->wdata;
}

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
void bk_io_wr(int adr, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	if (comp->cpu->nod & 1)		// LSB
		comp->iomap[(adr & ~1) & 0xffff] = val & 0xff;
	if (comp->cpu->nod & 2)		// MSB
		comp->iomap[(adr | 1) & 0xffff] = (val >> 8) & 0xff;
	hwOut(bk_io_tab, comp, adr & ~1, val, 1);
}

void bk11_io_wr(int adr, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	if (comp->cpu->nod & 1)		// LSB
		comp->iomap[(adr & ~1) & 0xffff] = val & 0xff;
	if (comp->cpu->nod & 2)		// MSB
		comp->iomap[(adr | 1) & 0xffff] = (val >> 8) & 0xff;
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
	if (comp->cpu->nod & 1)
		comp->mem->ramData[(fadr & ~1) & comp->mem->ramMask] = val & 0xff;
	if (comp->cpu->nod & 2)
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
	if ((comp->vid->newFrame) && (comp->iomap[0xffb3] & 0x40)) {
		comp->cpu->intrq |= PDP_INT_IRQ2;
	}
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
	memSetBank(comp->mem, 0x40, MEM_RAM, comp->reg[2] & 7, MEM_16K, bk_ram_rd, bk_ram_wr, comp);	// ram page @ 0x4000
	if (comp->reg[1] & 0x80) {									// ram/rom @ 0x8000
		memSetBank(comp->mem, 0x80, MEM_ROM, comp->reg[1] & 3, MEM_16K, bk_rom_rd, bk_rom_wr, comp);
	} else {
		memSetBank(comp->mem, 0x80, MEM_RAM, comp->reg[1] & 7, MEM_16K,  bk_ram_rd, bk_ram_wr, comp);
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
	comp->reg[0] = 1;
	comp->reg[1] = 0x80;
	comp->cpu->reset(comp->cpu);
	comp->vid->curscr = 0;
	comp->vid->paln = 0;
	vid_set_mode(comp->vid, VID_BK_BW);
	comp->keyb->flag = 0x40;
	comp->keyb->keycode = 0x00;
	bk_mem_map(comp);
}

void bk11_reset(Computer* comp) {
	memSetSize(comp->mem, MEM_128K, MEM_64K);
	for (int i = 0; i < 0x40; i++) {
		vid_set_col(comp->vid, i, bk_pal[i]);
	}
	comp->reg[0] = 1;
	comp->reg[1] = 0x80;
	comp->cpu->reset(comp->cpu);
//	comp->vid->curscr = 0;
	comp->vid->paln = 0;
	vid_set_mode(comp->vid, VID_BK_BW);
	comp->keyb->flag = 0x40;
	comp->keyb->keycode = 0x00;
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
			comp->keyb->flag = 0x00;
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
	vid_upd_timings(comp->vid, 302);
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
	{XKEY_LBRACK,0150},{XKEY_RBRACK,0177}, /*{XKEY_LBRACE,0150},{XKEY_RBRACE,0177},*/
	{XKEY_A,0146},{XKEY_S,0171},{XKEY_D,0167},{XKEY_F,0141},{XKEY_G,0160},
	{XKEY_H,0162},{XKEY_J,0157},{XKEY_K,0154},{XKEY_L,0144},{XKEY_DOTCOM,0166},{XKEY_APOS,0174},
	{XKEY_Z,0161},{XKEY_X,0176},{XKEY_C,0163},{XKEY_V,0155},{XKEY_B,0111},
	{XKEY_N,0164},{XKEY_M,0170},{XKEY_COMMA,0142},{XKEY_PERIOD,0140},
	{ENDKEY, 0}
};

static bkKeyCode bkey_small_rus[] = {
	{XKEY_Q,0112},{XKEY_W,0103},{XKEY_E,0125},{XKEY_R,0113},{XKEY_T,0105},
	{XKEY_Y,0116},{XKEY_U,0107},{XKEY_I,0133},{XKEY_O,0135},{XKEY_P,0132},
	{XKEY_LBRACK,0110},{XKEY_RBRACK,0137}, /*{XKEY_LBRACE,0110},{XKEY_RBRACE,0137},*/
	{XKEY_A,0106},{XKEY_S,0131},{XKEY_D,0127},{XKEY_F,0101},{XKEY_G,0120},
	{XKEY_H,0122},{XKEY_J,0117},{XKEY_K,0114},{XKEY_L,0104},{XKEY_DOTCOM,0126},{XKEY_APOS,0134},
	{XKEY_Z,0121},{XKEY_X,0136},{XKEY_C,0123},{XKEY_V,0115},{XKEY_B,0111},
	{XKEY_N,0124},{XKEY_M,0130},{XKEY_COMMA,0102},{XKEY_PERIOD,0100},
	{ENDKEY, 0}
};

static bkKeyCode bkey_shift[] = {
	{XKEY_1, '!'},{XKEY_2, '@'},{XKEY_3, '#'},{XKEY_4, '$'},{XKEY_5, '%'},
	{XKEY_6, '^'},{XKEY_7, '&'},{XKEY_8, '*'},{XKEY_9, '('},{XKEY_0, ')'},
	{XKEY_MINUS,'_'},{XKEY_EQUAL,'='},
	{ENDKEY, 0}
};

static bkKeyCode bkey_noshift[] = {
	{XKEY_1,'1'},{XKEY_2,'2'},{XKEY_3,'3'},{XKEY_4,'4'},{XKEY_5,'5'},
	{XKEY_6,'6'},{XKEY_7,'7'},{XKEY_8,'8'},{XKEY_9,'9'},{XKEY_0,'0'},
	{XKEY_MINUS,'-'},{XKEY_EQUAL,'+'},
	{ENDKEY, 0}
};

static bkKeyCode bkeyTab[] = {
	{XKEY_LBRACK,'('},{XKEY_RBRACK,')'},
	{XKEY_DOTCOM, ';'},{XKEY_APOS,'"'},
	{XKEY_COMMA, ','},{XKEY_PERIOD, '.'},{XKEY_BSLASH,'/'},{XKEY_SLASH,'\\'},
	{XKEY_SPACE,' '},{XKEY_ENTER,10},
	{XKEY_BSP,24},
	{XKEY_TAB,13},
	{XKEY_DOWN,27},{XKEY_LEFT,8},{XKEY_RIGHT,25},{XKEY_UP,26},
//	{XKEY_ESC,3},
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
static char bkvidcol[] = " color mode ";
static char bkvidbw[] = " b/w mode ";

void bk_press_keycode(Computer* comp, int code) {
	comp->keyb->flag |= 0x20;
	if (!(comp->keyb->flag & 0x80)) {
		comp->keyb->keycode = code & 0x7f;
		comp->keyb->flag |= 0x80;
		if (!(comp->keyb->flag & 0x40)) {		// keyboard interrupt enabled
			comp->cpu->intvec = (code & 0x80) ? 0274 : 060;
			comp->cpu->intrq |= PDP_INT_VIRQ;
//				printf("intrq %X\n", comp->cpu->intvec);
		}
	}
}

void bk_keyp(Computer* comp, keyEntry xkey) {
	int code = 0;
	switch(xkey.key) {
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
		case XKEY_LSHIFT:
			comp->keyb->shift = 1;
//			code = 0274;
			break;
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
	if (code == 0) {
		if (comp->keyb->shift) {
			code = bkey_code(bkey_shift, xkey.key);
		} else {
			code = bkey_code(bkey_noshift, xkey.key);
		}
	}
	if (code == 0)
		code = bkey_code(bkeyTab, xkey.key);
	if (code != 0) {
		bk_press_keycode(comp, code);
	}
}

void bk_keyr(Computer* comp, keyEntry xkey) {
	switch (xkey.key) {
		case XKEY_LSHIFT:
			comp->keyb->shift = 0;
//			bk_press_keycode(comp, 0273);
			break;
	}
	comp->keyb->flag &= ~0x20;	// 0x20 | 0x80
}
