#include <assert.h>
#include <string.h>
#include <time.h>

#include "../spectrum.h"
#include "../cpu/MOS6502/6502.h"

#define	regR00	reg[16]
#define regR01	reg[17]
#define regKROW	reg[18]

// Commodore
// dotClock	8.18MHz (ntsc) / 7.88MHz (pal)
// colorCLK	14.31818MHz (ntsc) / 17.734472MHz (pal)
// CPU clock	~1MHz

#define F_LORAM		1
#define	F_HIRAM		(1<<1)
#define	F_CHAREN	(1<<2)

// vicII read byte
// bits 00..13:vic ADR bus
// bits 14..15:cia2 reg #00 bit 0,1 inverted

int c64_vic_mrd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	adr &= 0x3fff;
	adr |= ((comp->vid->vbank & 3) << 14);
	return comp->mem->ramData[adr & comp->mem->ramMask];
}

// rom
// writing to rom will write to ram under rom placed above ram...

int c64_rom_rd(int adr, void* data) {
	Computer* comp = (Computer*)data;
	int res = -1;
	if (adr & 0x4000) {		// e000..ffff:kernal (1110..1111)
		res = comp->mem->romData[0x2000 + (adr & 0x1fff)];	// kernal (8K page 1)
	} else {			// a000..bfff:basic (1010..1011)
		res = comp->mem->romData[adr & 0x1fff];			// basic (8K page 0)
	}
	return res;
}

void c64_rom_wr(int adr, int val, void* data) {
	Computer* comp = (Computer*)data;
	comp->mem->ramData[adr & 0xffff] = val & 0xff;
}

// d000..d3ff	vicII (0x3f registers + mirrors)

unsigned char vic_rd(Video*, int);
void vic_wr(Video*, int, int);

int c64_vic_rd(int adr, void* data) {
	Computer* comp = (Computer*)data;
	return vic_rd(comp->vid, adr & 0x3f);
}

void c64_vic_wr(int adr, int val, void* data) {
	Computer* comp = (Computer*)data;
	vic_wr(comp->vid, adr & 0x3f, val);
}

// d400..d7ff	sid

int c64_sid_rd(int adr, void* data) {
	return 0xff;
}

void c64_sid_wr(int adr, int val, void* data) {

}

// d800..dbff	palette (screen attributes 40x25 = 1000 bytes)
// vid->colram

int c64_pal_rd(int adr, void* data) {
	Computer* comp = (Computer*)data;
	return comp->vid->colram[adr & 0x3ff];
}

void c64_pal_wr(int adr, int val, void* data) {
	Computer* comp = (Computer*)data;
	comp->vid->colram[adr & 0x3ff] = val;
}

// dc00/dd00 : cia 1 & cia 2 common ports
// all registers except 0,1 is the same (timer, tod, interrupts)
// Reading the ICR (reg D) will read the interrupt flags and automatically clear them.

extern unsigned char toBCD(unsigned char);

// dc00..dcff	cia1 (0x10 registers + mirrors)

// cia1 port A rd: joystick
int cia1_porta_rd(int adr, void* p) {
	return 0xff;
}

// cia1 port B rd: keyboard scan
int cia1_portb_rd(int adr, void* p) {
	Computer* comp = (Computer*)p;
	return kbd_rd(comp->keyb, comp->regKROW);
}

void cia1_porta_wr(int adr, int v, void* p) {
	((Computer*)p)->regKROW = v & 0xff;
}

int c64_cia1_rd(int adr, void* p) {
	return c64_cia_rd(((Computer*)p)->cia1, adr);
}

void c64_cia1_wr(int adr, int val, void* p) {
	c64_cia_wr(((Computer*)p)->cia1, adr, val);
}

// dd00..ddff	cia2 (0x10 registers + mirrors)

void cia2_porta_wr(int adr, int v, void* p) {
	((Computer*)p)->vid->vbank = ~v & 3;
}

int c64_cia2_rd(int adr, void* p) {
	return c64_cia_rd(((Computer*)p)->cia2, adr);
}

void c64_cia2_wr(int adr, int val, void* p) {
	c64_cia_wr(((Computer*)p)->cia2, adr, val);
}

// de00..deff	io1

int c64_io1_rd(int adr, void* data) {
	return -1;
}

void c64_io1_wr(int adr, int val, void* data) {

}

// df00..dfff	io2

int c64_io2_rd(int adr, void* data) {
	return -1;
}

void c64_io2_wr(int adr, int val, void* data) {

}

// char rom

int c64_chrd(int adr, void* data) {
	Computer* comp = (Computer*)data;
	return vid_fnt_rd(comp->vid, adr & 0x7ff);	// comp->vid->font[adr & 0x7ff];
}

void c64_maper(Computer* comp) {
	// low 32K allways RAM
	memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_32K, NULL, NULL, NULL);
	// 0x8000: RAM or cartrige rom (TODO)
	memSetBank(comp->mem, 0x80, MEM_RAM, 4, MEM_8K, NULL, NULL, NULL);
	// 0xa000: RAM or BASIC (8K): reg01 & 1
	if ((comp->regR01 & (F_LORAM | F_HIRAM)) == (F_LORAM | F_HIRAM)) {			// 11:basic, others:ram
		memSetBank(comp->mem, 0xa0, MEM_ROM, 0, MEM_8K, c64_rom_rd, c64_rom_wr, comp);	// BASIC (A000..BFFF)
	} else {
		memSetBank(comp->mem, 0xa0, MEM_RAM, 5, MEM_8K, NULL, NULL, NULL);
	}
	// 0xc000: RAM (4K)
	memSetBank(comp->mem, 0xc0, MEM_RAM, 12, MEM_4K, NULL, NULL, NULL);
	// 0xd000: RAM, CHARROM or IO
	if ((comp->regR01 & (F_LORAM | F_HIRAM)) == 0) {	// x00:RAM
		memSetBank(comp->mem, 0xd0, MEM_RAM, 13, MEM_4K, NULL, NULL, NULL);
	} else if (comp->regR01 & F_CHAREN) {		// 1xx:IO
		memSetBank(comp->mem, 0xd0, MEM_IO, 0, MEM_1K, c64_vic_rd, c64_vic_wr, comp);		// vicII
		memSetBank(comp->mem, 0xd4, MEM_IO, 0, MEM_1K, c64_sid_rd, c64_sid_wr, comp);		// sid
		memSetBank(comp->mem, 0xd8, MEM_IO, 0, MEM_1K, c64_pal_rd, c64_pal_wr, comp);		// palette
		memSetBank(comp->mem, 0xdc, MEM_IO, 0, MEM_256, c64_cia1_rd, c64_cia1_wr, comp);	// cia1
		memSetBank(comp->mem, 0xdd, MEM_IO, 0, MEM_256, c64_cia2_rd, c64_cia2_wr, comp);	// cia
		memSetBank(comp->mem, 0xde, MEM_IO, 0, MEM_256, c64_io1_rd, c64_io1_wr, comp);		// io1
		memSetBank(comp->mem, 0xdf, MEM_IO, 0, MEM_256, c64_io2_rd, c64_io2_wr, comp);		// io2
	} else {						// 0xx:CHARROM
		memSetBank(comp->mem, 0xd0, MEM_EXT, 3, MEM_4K, c64_chrd, NULL, comp);
	}
	// 0xe000: RAM or KERNAL
	if (comp->regR01 & F_HIRAM) {			// x1x:KERNAL
		memSetBank(comp->mem, 0xe0, MEM_ROM, 1, MEM_8K, c64_rom_rd, c64_rom_wr, comp);
	} else {						// x0x:RAM
		memSetBank(comp->mem, 0xe0, MEM_RAM, 7, MEM_8K, NULL, NULL, NULL);
	}
}

// colors taken from C64 wiki
// https://www.c64-wiki.com/wiki/Color

static xColor c64_palette[16] = {
	{0x00,0x00,0x00},{0xff,0xff,0xff},{0x88,0x00,0x00},{0xAA,0xFF,0xEE},
	{0xCC,0x44,0xCC},{0x00,0xCC,0x55},{0x00,0x00,0xAA},{0xEE,0xEE,0x77},
	{0xDD,0x88,0x55},{0x66,0x44,0x00},{0xFF,0x77,0x77},{0x33,0x33,0x33},
	{0x77,0x77,0x77},{0xAA,0xFF,0x66},{0x00,0x88,0xFF},{0xBB,0xBB,0xBB}
};

void c64_reset(Computer* comp) {
	comp->regR00 = 0x2f;
	comp->regR01 = 0x37;
	int i;
	xColor xcol;
	for (i = 0; i < 16; i++) {
		comp->cia1->reg[i] = 0x00;
		comp->cia2->reg[i] = 0x00;
		xcol = c64_palette[i];
		vid_set_col(comp->vid, i, xcol);
	}
	memset(comp->vid->reg, 0x00, 256);
	c64_maper(comp);
	vid_set_mode(comp->vid, VID_C64_TEXT);
}

int c64_mrd(Computer* comp, int adr, int m1) {
	int res = -1;
	switch (adr) {
		case 0x0000:
			res = comp->regR00;
			break;
		case 0x0001:
			res = comp->regR01 & 0xef;
			if (!comp->tape->on) res |= 0x10;	// 1:no buttons pressed, 0:button pressed
			break;
		default:
			res = memRd(comp->mem, adr);
			break;
	}
	return res;
}

// 0001
// b0..2	memory configuration
// b3		datasette output level (rec)
// b4		0:one of datasette buttons pressed (play,rec,rew,ffwd), 1:no buttons
// b5		wr:datasette motor control (1:off, 0:on)

void c64_mwr(Computer* comp, int adr, int val) {
	switch (adr) {
		case 0x0000:
			comp->regR00 = val & 0xff;
			break;
		case 0x0001:
			comp->regR01 &= ~comp->regR00;			// reset output bits in R0
			val &= comp->regR00;					// reset ro bits in value
			comp->regR01 |= val;					// set new output bits in R0
			c64_maper(comp);
			comp->tape->levRec = (val & 8) ? 0xb0 : 0x50;		// datasette output
			if (val & 0x20) {
				comp->tape->on = 0;
				comp->tape->rec = 0;
			} else {
				comp->tape->on = 1;
			}
			break;
		default:
			memWr(comp->mem, adr, val);
			break;
	}
}

sndPair c64_vol(Computer* comp, sndVolume* sv) {
	sndPair res;
	int lev = 0;
	// 1:tape sound
	if (comp->tape->on) {
		if (comp->tape->rec) {
			lev = comp->tape->levRec ? 0x1000 * sv->tape / 100 : 0;
		} else {
			lev = (comp->tape->volPlay << 8) * sv->tape / 1600;
		}
	}
	res.left = lev;
	res.right = lev;
	return res;
}

void c64_init(Computer* comp) {
	vid_upd_timings(comp->vid, comp->nsPerTick >> 3);
	fdc_set_hd(comp->dif->fdc, 0);
	comp->vid->mrd = c64_vic_mrd;
//	comp->tape->xen = 1;
	cia_set_port(comp->cia1, 0, cia1_porta_rd, cia1_porta_wr);
	cia_set_port(comp->cia1, 1, cia1_portb_rd, NULL);
	cia_set_port(comp->cia2, 0, NULL, cia2_porta_wr);
	cia_set_port(comp->cia2, 1, NULL, NULL);
	kbd_set_type(comp->keyb, KBD_C64);
}

void c64_irq(Computer* comp, int t) {
//	printf("c64irq\n");
	switch (t) {
		case IRQ_CIA1:
		case IRQ_CIA2:
			comp->cpu->intrq |= MOS6502_INT_IRQ;
			break;
		case IRQ_TAP_0:
			cia_irq(comp->cia1, CIA_IRQ_FLAG);
			break;
		case IRQ_VIC:
			printf("vic int\n");
			comp->cpu->intrq |= MOS6502_INT_IRQ;
			break;
	}
}

void c64_sync(Computer* comp, int ns) {
	if (comp->tape->on && !comp->tape->xen) {
		int vol = comp->tape->volPlay;
		tapSync(comp->tape, ns);
		if ((vol > 0x80) && (comp->tape->volPlay < 0x80)) {	// front 1->0
			cia_irq(comp->cia1, CIA_IRQ_FLAG);
		}
	}
	cia_sync(comp->cia1, ns, comp->nsPerTick);
	cia_sync(comp->cia2, ns, comp->nsPerTick);
}

void c64_keyp(Computer* comp, keyEntry* ent) {
	kbd_press(comp->keyb, ent);
}

void c64_keyr(Computer* comp, keyEntry* ent) {
	kbd_release(comp->keyb, ent);
}

static vLayout cmdrLay = {{512,312},{24,30},{144,44},{320,200},{0,0},64};

HardWare c64_hw_core = {HW_C64,HWG_COMMODORE,"Commodore64","Commodore64",16,MEM_128K,1.0,&cmdrLay,16,NULL,
			c64_init,c64_maper,NULL,NULL,c64_mrd,c64_mwr,c64_irq,NULL,c64_reset,c64_sync,c64_keyp,c64_keyr,c64_vol};
