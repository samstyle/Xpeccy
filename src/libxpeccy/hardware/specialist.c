#include "hardware.h"

int spc_rd_io_a(Computer* comp) {
	int res;
	int msk;
	if (comp->ppi.a.rd) {	// A in
		msk =  comp->ppi.b.rd ? 0xff : comp->ppi.b.val;
		res = -1;
		for (int row = 2; row < 8; row++) {
			if (!(msk & (1 << row)))
				res &= comp->keyb->map[row];
		}
	} else {
		res = 0xff; // comp->ppi.a.val;
	}
	return res;
}

// technically, if A out, C.low out, then B in do keyboard scan too
int spc_rd_io_b(Computer* comp) {
	int res;
	int row;
	int mask;
	if (comp->ppi.b.rd) {
		res = ~1;
		// scan keyb.rows by bits A & C
		mask = ~(((comp->ppi.cl.val << 4) & 0xf00) | (comp->ppi.a.val & 0xff));	// bits set on scanned columns
		mask &= 0xfff;
		for (row = 2; row < 8; row++) {
			if ((comp->keyb->map[row] & mask) != mask) {
				res &= ~(1 << row);
			}
		}
		if (comp->keyb->map[1] != -1)	// HP key
			res ^= 2;
		if (comp->tape->volPlay & 0x80)
			res |= 1;
	} else {
		res = 0xff; // comp->ppi.b.val;
	}
	return res;
}

int spc_rd_io_c(Computer* comp) {
	int res = -1;
	// form full regC
	for (int row = 2; row < 8; row++) {
		if (!(comp->ppi.b.val & (1 << row)))
			res &= (comp->keyb->map[row] >> 8) & 0x0f;
	}
	// mask blocked halfes
	if (!comp->ppi.cl.rd) {
		res |= 0x0f;
	}
	if (!comp->ppi.ch.rd) {
		res |= 0xf0;
	}
	return res;
}

int spc_rd_io(int adr, void* p) {
	Computer* comp = (Computer*)p;
	int res = -1;
	switch (adr & 3) {
		case 0: res = spc_rd_io_a(comp);
			break;
		case 1: res = spc_rd_io_b(comp);
			break;
		case 2: res = spc_rd_io_c(comp);
			break;
	}
	//printf("rd %i = %.2X\n", adr & 3, res & 0xff);
	return res;
}

void spc_wr_io_a(Computer* comp, int val) {
	if (!comp->ppi.a.rd) {
		comp->ppi.a.val = val & 0xff;
	}
}

void spc_wr_io_b(Computer* comp, int val) {
	if (!comp->ppi.b.rd)
		comp->ppi.b.val = val & 0xff;
}

void spc_wr_io_c(Computer* comp, int val) {
	if (!comp->ppi.cl.rd) {
		comp->ppi.cl.val = (val & 0x0f);
	}
	if (!comp->ppi.ch.rd) {
		comp->ppi.ch.val = (val & 0xf0);
		spc_mem_map(comp);				// b4:memory map
		comp->beep->lev = (val & 0x20) ? 1 : 0;		// b5:beeper
		comp->tape->levRec = (val & 0x80) ? 1 : 0;	// b7:tape out
	}
}

void spc_wr_io(int adr, int val, void* p) {
	Computer* comp = (Computer*)p;
	comp->firstRun = 0;
	int mask;
	// printf("wr %i,%.2X\n", adr & 3, val);
	switch (adr & 3) {
		case 0: spc_wr_io_a(comp, val);
			break;
		case 1: spc_wr_io_b(comp, val);
			break;
		case 2: spc_wr_io_b(comp, val);
			break;
		case 3:			// ctrl
			if (val & 0x80) {
				comp->ppi.ctrl = val & 0xff;
				comp->ppi.a.rd = (val & 0x10) ? 1 : 0;
				comp->ppi.b.rd = (val & 0x02) ? 1 : 0;
				comp->ppi.cl.rd = (val & 0x01) ? 1 : 0;
				comp->ppi.ch.rd = (val & 0x08) ? 1 : 0;
				comp->ppi.a.mode = (val & 0x40) ? 2 : ((val & 0x20) ? 1 : 0);
				comp->ppi.ch.mode = comp->ppi.a.mode;
				comp->ppi.b.mode = (val & 4) ? 1 : 0;
				comp->ppi.cl.mode = comp->ppi.b.mode;
				comp->ppi.a.val = 0;
				comp->ppi.b.val = 0;
				comp->ppi.cl.val = 0;
			} else {
				mask = (1 << ((val & 0xe0) >> 1));	// bit mask
				if (val & 1) {		// set
					spc_wr_io_c(comp, (comp->ppi.cl.val | comp->ppi.ch.val) | mask);
				} else {
					spc_wr_io_c(comp, (comp->ppi.cl.val | comp->ppi.ch.val) & ~mask);
				}
			}
			spc_mem_map(comp);
			break;
	}
}

int spc_vid_rd(int adr, void* p) {
	Computer* comp = (Computer*)p;
	return memRd(comp->mem, 0x9000 + (adr & 0x3fff));
}

void spc_mem_map(Computer* comp) {
	if ((!(comp->ppi.ch.val & 0x10) && !comp->ppi.ch.rd) || comp->firstRun) {
		memSetBank(comp->mem, 0x00, MEM_ROM, 0, MEM_16K, NULL, NULL, NULL);		// 0xc000...0xf7ff ROM
		memSetBank(comp->mem, 0x38, MEM_IO, 0, MEM_2K, spc_rd_io, spc_wr_io, comp);	// 0xf800...0xffff IO
		memSetBank(comp->mem, 0x40, MEM_ROM, 0, MEM_16K, NULL, NULL, NULL);		// 0xc000...0xf7ff ROM
		memSetBank(comp->mem, 0x78, MEM_IO, 0, MEM_2K, spc_rd_io, spc_wr_io, comp);	// 0xf800...0xffff IO
		memSetBank(comp->mem, 0x80, MEM_ROM, 0, MEM_16K, NULL, NULL, NULL);		// 0xc000...0xf7ff ROM
		memSetBank(comp->mem, 0xb8, MEM_IO, 0, MEM_2K, spc_rd_io, spc_wr_io, comp);	// 0xf800...0xffff IO
	} else {
		memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_64K, NULL, NULL, NULL);		// 0x0000...0xbfff RAM
	}
	memSetBank(comp->mem, 0xc0, MEM_ROM, 0, MEM_16K, NULL, NULL, NULL);		// 0xc000...0xf7ff ROM
	memSetBank(comp->mem, 0xf8, MEM_IO, 0, MEM_2K, spc_rd_io, spc_wr_io, comp);	// 0xf800...0xffff IO
}

// reset: all channel IN mode

void spc_reset(Computer* comp) {
	comp->ppi.a.rd = 1;
	comp->ppi.a.val = 0xff;
	comp->ppi.b.rd = 1;
	comp->ppi.b.val = 0xff;
	comp->ppi.cl.rd = 1;
	comp->ppi.cl.val = 0x0f;
	comp->ppi.ch.rd = 1;
	comp->ppi.ch.val = 0xf0;
	comp->ppi.ctrl = 0xff;
	comp->vid->mrd = spc_vid_rd;
	vidSetMode(comp->vid, VID_SPCLST);
	comp->cpu->reset(comp->cpu);
	// kbdReleaseAll(comp->keyb);
	comp->firstRun = 1;
}

int spc_mrd(Computer* comp, int adr, int m1) {
	return memRd(comp->mem, adr) & 0xff;
}

void spc_mwr(Computer* comp, int adr, int val) {
	memWr(comp->mem, adr, val);
}

void spc_sync(Computer* comp, int ns) {
	tapSync(comp->tape, ns);
	bcSync(comp->beep, ns);
}

static keyScan spc_keys[] = {
	//f1		f2		f3		f4	f5		f6		f7	f8		f9	f10		lock		clear
	{'!',7,0x800},{'@',7,0x400},{'#',7,0x200},{'$',7,0x100},{'%',7,0x80},{'^',7,0x40},{'&',7,0x20},{'*',7,0x10},{'(',7,0x8},{')',7,0x04},{'O',7,0x02},{'C',7,0x01},
	{';',6,0x800},{'1',6,0x400},{'2',6,0x200},{'3',6,0x100},{'4',6,0x80},{'5',6,0x40},{'6',6,0x20},{'7',6,0x10},{'8',6,0x8},{'9',6,0x04},{'0',6,0x02},{'-',6,0x01},
	{'j',5,0x800},{'c',5,0x400},{'u',5,0x200},{'k',5,0x100},{'e',5,0x80},{'n',5,0x40},{'g',5,0x20},{'[',5,0x10},{']',5,0x8},{'z',5,0x04},{'h',5,0x02},{':',5,0x01},
	{'f',4,0x800},{'y',4,0x400},{'w',4,0x200},{'a',4,0x100},{'p',4,0x80},{'r',4,0x40},{'o',4,0x20},{'l',4,0x10},{'d',4,0x8},{'v',4,0x04},{'/',4,0x02},{'>',4,0x01},
	{'q',3,0x800},{'|',3,0x400},{'s',3,0x200},{'m',3,0x100},{'i',3,0x80},{'t',3,0x40},{'x',3,0x20},{'b',3,0x10},{'A',3,0x8},{'<',3,0x04},{'?',3,0x02},{'B',3,0x01},
	{'X',2,0x800},{'H',2,0x400},{'U',2,0x200},{'D',2,0x100},{'T',2,0x80},{'_',2,0x40},{0x20,2,0x20},{'L',2,0x10},{'V',2,0x8},{'R',2,0x04},{'S',2,0x02},{'E',2,0x01},
	{'P',1,0x800},	/* HP button */
	{0, 0, 0}
};

void spc_keyp(Computer* comp, keyEntry kent) {
	// printf("press: %s, %c %c (%i)\n", kent.name, kent.zxKey.key1, kent.zxKey.key2, kent.key);
	kbd_press(comp->keyb, spc_keys, comp->keyb->map, kent.zxKey);
	// printf("kbd: "); for (int i = 7; i > 1; i--) {printf("%.3X ", comp->keyb->map[i] & 0xfff);} printf("\n");
}

void spc_keyr(Computer* comp, keyEntry kent) {
	kbd_release(comp->keyb, spc_keys, comp->keyb->map, kent.zxKey);
}

sndPair spc_vol(Computer* comp, sndVolume* v) {
	sndPair vol;
	vol.left = comp->beep->val * v->beep / 6;
	vol.left += (comp->tape->volPlay << 8) * v->tape / 1600;
	vol.right = vol.left;
	return vol;
}
