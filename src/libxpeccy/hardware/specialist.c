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
//	int row;
	int mask;
	// scan keyb.rows by bits A & C
	mask = ((comp->ppi->cl.val << 8) & 0xf00) | (comp->ppi->a.val & 0x0ff);
	if (comp->ppi->cl.dir != PPI_OUT) mask |= 0xf00;
	if (comp->ppi->a.dir != PPI_OUT) mask |= 0xff;
	mask = ~mask;

	res &= kbd_rd(comp->keyb, mask);
#if 0
	for (row = 2; row < 8; row++) {
		if ((comp->keyb->map[row] & mask) != mask) {
			res &= ~(1 << row);
		}
	}
	if (comp->keyb->map[1] != -1)	// HP key
		res ^= 2;
#endif
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
	kbd_set_type(comp->keyb, KBD_SPCLST);
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
}

void spc_keyp(Computer* comp, keyEntry* kent) {
	kbd_press(comp->keyb, kent);
}

void spc_keyr(Computer* comp, keyEntry* kent) {
	kbd_release(comp->keyb, kent);
}

sndPair spc_vol(Computer* comp, sndVolume* v) {
	sndPair vol;
	vol.left = comp->beep->val * v->beep / 6;
	vol.left += (comp->tape->volPlay << 8) * v->tape / 1600;
	vol.right = vol.left;
	return vol;
}

static vLayout spclstLay = {{384+16,256+8},{0,0},{16,8},{384,256},{0,0},0};

HardWare spc_hw_core = {HW_SPCLST,HWG_SPCLST,"Specialist","Specialist",16,MEM_64K,1.0,&spclstLay,16,NULL,
			spc_init,spc_mem_map,NULL,NULL,spc_mrd,spc_mwr,NULL,NULL,spc_reset,spc_sync,spc_keyp,spc_keyr,spc_vol};
