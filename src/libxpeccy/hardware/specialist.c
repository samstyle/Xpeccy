#include "hardware.h"

int spc_rd_io_a(void* p) {
	Computer* comp = (Computer*)p;
	int res;
	int msk =  (comp->ppi->b.dir == PPI_OUT) ? comp->ppi->b.val : 0xff;
	res = -1;
	for (int row = 2; row < 8; row++) {
		if (!(msk & (1 << row)))
			res &= comp->keyb->map[row];
	}
	return res;
}

// technically, if A out, C.low out, then B in do keyboard scan too
int spc_rd_io_b(void* p) {
	Computer* comp = (Computer*)p;
	int res = ~1;
	int row;
	int mask;
	// scan keyb.rows by bits A & C
	mask = ((comp->ppi->cl.val << 8) & 0xf00) | (comp->ppi->a.val & 0x0ff);
	if (comp->ppi->cl.dir != PPI_OUT) mask |= 0xf00;
	if (comp->ppi->a.dir != PPI_OUT) mask |= 0xff;
	mask = ~mask;
	for (row = 2; row < 8; row++) {
		if ((comp->keyb->map[row] & mask) != mask) {
			res &= ~(1 << row);
		}
	}
	if (comp->keyb->map[1] != -1)	// HP key
		res ^= 2;
	if ((comp->tape->volPlay & 0x80) || !comp->tape->on)	// if tape stopped, signal must be 1
		res |= 1;
	return res;
}

int spc_rd_io_c(void* p) {
	Computer* comp = (Computer*)p;
	int res = -1;
	// form full regC
	int msk = (comp->ppi->b.dir == PPI_OUT) ? comp->ppi->b.val : 0xff;
	for (int row = 2; row < 8; row++) {
		if (!(msk & (1 << row)))
			res &= (comp->keyb->map[row] >> 8) & 0x0f;
	}
	return res;
}

int spc_rd_io(int adr, void* p) {
	Computer* comp = (Computer*)p;
	return ppi_rd(comp->ppi, adr & 3);
}

// reset -> set NP
// b4,C = 0 -> reset NP & never set again until comp reset (!)
// NP = 1: ROM/IO in every 16K
// NP = 0: RAM in 0x0000..0xbfff
// comp->rom = NP

void spc_wr_io_ch(int val, void* p) {
	Computer* comp = (Computer*)p;
	if (comp->rom && !(val & 0x10)) {		// b4: 0 will reset NP till next RESET
		comp->rom = 0;
		spc_mem_map(comp);
	}
	comp->beep->lev = (val & 0x20) ? 1 : 0;		// b5:beeper
	comp->tape->levRec = (val & 0x80) ? 1 : 0;	// b7:tape out
}

void spc_wr_io(int adr, int val, void* p) {
	Computer* comp = (Computer*)p;
	comp->rom = 0;
	ppi_wr(comp->ppi, adr & 3, val);
	spc_mem_map(comp);
}

int spc_vid_rd(int adr, void* p) {
	Computer* comp = (Computer*)p;
	return memRd(comp->mem, 0x9000 + (adr & 0x3fff));
}

void spc_init(Computer* comp) {
	vid_set_mode(comp->vid, VID_SPCLST);
	vid_upd_timings(comp->vid, comp->nsPerTick >> 1);			// CPU:2MHz, dots:8MHz
	comp->vid->mrd = spc_vid_rd;
	ppi_set_cb(comp->ppi, comp, spc_rd_io_a, NULL,\
				spc_rd_io_b, NULL,\
				spc_rd_io_c, spc_wr_io_ch,\
				spc_rd_io_c, NULL);
}

void spc_mem_map(Computer* comp) {
	if (comp->rom) {
		memSetBank(comp->mem, 0x00, MEM_ROM, 0, MEM_16K, NULL, NULL, NULL);		// 0x0000...0x37ff ROM
		memSetBank(comp->mem, 0x38, MEM_IO, 0, MEM_2K, spc_rd_io, spc_wr_io, comp);	// 0x3800...0x3fff IO
		memSetBank(comp->mem, 0x40, MEM_ROM, 0, MEM_16K, NULL, NULL, NULL);		// 0x4000...0x77ff ROM
		memSetBank(comp->mem, 0x78, MEM_IO, 0, MEM_2K, spc_rd_io, spc_wr_io, comp);	// 0x7800...0x7fff IO
		memSetBank(comp->mem, 0x80, MEM_ROM, 0, MEM_16K, NULL, NULL, NULL);		// 0x8000...0xb7ff ROM
		memSetBank(comp->mem, 0xb8, MEM_IO, 0, MEM_2K, spc_rd_io, spc_wr_io, comp);	// 0xb800...0xbfff IO
	} else {
		memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_64K, NULL, NULL, NULL);		// 0x0000...0xbfff RAM
	}
	memSetBank(comp->mem, 0xc0, MEM_ROM, 0, MEM_16K, NULL, NULL, NULL);		// 0xc000...0xf7ff ROM
	memSetBank(comp->mem, 0xf8, MEM_IO, 0, MEM_2K, spc_rd_io, spc_wr_io, comp);	// 0xf800...0xffff IO
}

void spc_reset(Computer* comp) {
	ppi_reset(comp->ppi);
	comp->vid->mrd = spc_vid_rd;
	vid_set_mode(comp->vid, VID_SPCLST);
	cpu_reset(comp->cpu);
	// kbdReleaseAll(comp->keyb);
	comp->rom = 1;
}

int spc_mrd(Computer* comp, int adr, int m1) {
	return memRd(comp->mem, adr) & 0xff;
}

void spc_mwr(Computer* comp, int adr, int val) {
	memWr(comp->mem, adr, val);
}

//static int pregz = 0;

void spc_sync(Computer* comp, int ns) {
	tapSync(comp->tape, ns);
	bcSync(comp->beep, ns);
#if 0
	if (pregz) {
		printf("PC: %.4X AF: %.4X BC: %.4X DE: %.4X HL: %.4X SP: %.4X\n", comp->cpu->pc, comp->cpu->af, comp->cpu->bc, comp->cpu->de, comp->cpu->hl, comp->cpu->sp);
	}
	if (comp->cpu->pc == 0xce5) {
		pregz = 1;
		printf("-----\n");
	} else if (comp->cpu->pc == 0xd34) {
		pregz = 0;
	}
#endif
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
