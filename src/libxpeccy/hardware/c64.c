#include <assert.h>
#include <string.h>
#include <time.h>

#include "../spectrum.h"

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
	return vic_rd(comp->vid, adr);
}

void c64_vic_wr(int adr, int val, void* data) {
	Computer* comp = (Computer*)data;
	vic_wr(comp->vid, adr, val);
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
int cia1_porta_rd(int, void*) {
	return 0xff;
}

// cia1 port B rd: keyboard scan
int cia1_portb_rd(int, void* p) {
	Computer* comp = (Computer*)p;
	int row = comp->c64.keyrow;
	int res = 0xff;
	for (int idx = 0; idx < 8; idx++) {
		if ((row & 1) == 0) {
			res &= comp->keyb->msxMap[idx];
		}
		row >>= 1;
	}
	return res;
}

void cia1_porta_wr(int, int v, void* p) {
	((Computer*)p)->c64.keyrow = v & 0xff;
}

int c64_cia1_rd(int adr, void* p) {
	return c64_cia_rd(((Computer*)p)->c64.cia1, adr);
}

void c64_cia1_wr(int adr, int val, void* p) {
	c64_cia_wr(((Computer*)p)->c64.cia1, adr, val);
}

// dd00..ddff	cia2 (0x10 registers + mirrors)

void cia2_porta_wr(int, int v, void* p) {
	((Computer*)p)->vid->vbank = ~v & 3;
}

int c64_cia2_rd(int adr, void* p) {
	return c64_cia_rd(((Computer*)p)->c64.cia2, adr);
}

void c64_cia2_wr(int adr, int val, void* p) {
	c64_cia_wr(((Computer*)p)->c64.cia2, adr, val);
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
	return comp->vid->font[adr & 0x7ff];
}

void c64_maper(Computer* comp) {
	// low 32K allways RAM
	memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_32K, NULL, NULL, NULL);
	// 0x8000: RAM or cartrige rom (TODO)
	memSetBank(comp->mem, 0x80, MEM_RAM, 4, MEM_8K, NULL, NULL, NULL);
	// 0xa000: RAM or BASIC (8K): reg01 & 1
	if ((comp->c64.reg01 & (F_LORAM | F_HIRAM)) == (F_LORAM | F_HIRAM)) {			// 11:basic, others:ram
		memSetBank(comp->mem, 0xa0, MEM_ROM, 0, MEM_8K, c64_rom_rd, c64_rom_wr, comp);	// BASIC (A000..BFFF)
	} else {
		memSetBank(comp->mem, 0xa0, MEM_RAM, 5, MEM_8K, NULL, NULL, NULL);
	}
	// 0xc000: RAM (4K)
	memSetBank(comp->mem, 0xc0, MEM_RAM, 12, MEM_4K, NULL, NULL, NULL);
	// 0xd000: RAM, CHARROM or IO
	if ((comp->c64.reg01 & (F_LORAM | F_HIRAM)) == 0) {	// x00:RAM
		memSetBank(comp->mem, 0xd0, MEM_RAM, 13, MEM_4K, NULL, NULL, NULL);
	} else if (comp->c64.reg01 & F_CHAREN) {		// 1xx:IO
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
	if (comp->c64.reg01 & F_HIRAM) {			// x1x:KERNAL
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
	comp->c64.reg00 = 0x2f;
	comp->c64.reg01 = 0x37;
	int i;
	xColor xcol;
	for (i = 0; i < 16; i++) {
		comp->c64.cia1->reg[i] = 0x00;
		comp->c64.cia2->reg[i] = 0x00;
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
			res = comp->c64.reg00;
			break;
		case 0x0001:
			res = comp->c64.reg01 & 0xef;
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
			comp->c64.reg00 = val & 0xff;
			break;
		case 0x0001:
			comp->c64.reg01 &= ~comp->c64.reg00;			// reset output bits in R0
			val &= comp->c64.reg00;					// reset ro bits in value
			comp->c64.reg01 |= val;					// set new output bits in R0
			c64_maper(comp);
			comp->tape->levRec = (val & 8) ? 0xa0 : 0x60;		// datasette output
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
	vidUpdateTimings(comp->vid, comp->nsPerTick >> 3);
	fdc_set_hd(comp->dif->fdc, 0);
	comp->vid->mrd = c64_vic_mrd;
//	comp->tape->xen = 1;
	cia_set_port(comp->c64.cia1, 0, cia1_porta_rd, cia1_porta_wr);
	cia_set_port(comp->c64.cia1, 1, cia1_portb_rd, NULL);
	cia_set_port(comp->c64.cia2, 0, NULL, cia2_porta_wr);
	cia_set_port(comp->c64.cia2, 1, NULL, NULL);
}

void c64_irq(Computer* comp, int t) {
//	printf("c64irq\n");
	switch (t) {
		case IRQ_CIA1:
		case IRQ_CIA2:
			comp->cpu->intrq |= MOS6502_INT_IRQ;
			break;
		case IRQ_TAP_0:
			cia_irq(comp->c64.cia1, CIA_IRQ_FLAG);
			break;
		case IRQ_VID_INT:
			comp->cpu->intrq |= MOS6502_INT_IRQ;
			break;
	}
}

void c64_sync(Computer* comp, int ns) {
	if (comp->tape->on && !comp->tape->xen) {
		int vol = comp->tape->volPlay;
		tapSync(comp->tape, ns);
		if ((vol > 0x80) && (comp->tape->volPlay < 0x80)) {	// front 1->0
			cia_irq(comp->c64.cia1, CIA_IRQ_FLAG);
		}
	}
	cia_sync(comp->c64.cia1, ns, comp->nsPerTick);
	cia_sync(comp->c64.cia2, ns, comp->nsPerTick);
}

typedef struct {
	int code;
	int row;
	int mask;
} c64Key;

static c64Key c64matrix[] = {
	{XKEY_BSP,0,1},{XKEY_ENTER,0,2},{XKEY_RIGHT,0,4},{XKEY_F7,0,8}, {XKEY_F1,0,16},{XKEY_F3,0,32},{XKEY_F5,0,64},{XKEY_DOWN,0,128},
	{XKEY_3,1,1},{XKEY_W,1,2},{XKEY_A,1,4},{XKEY_4,1,8},{XKEY_Z,1,16},{XKEY_S,1,32},{XKEY_E,1,64},{XKEY_LSHIFT,1,128},
	{XKEY_5,2,1},{XKEY_R,2,2},{XKEY_D,2,4},{XKEY_6,2,8},{XKEY_C,2,16},{XKEY_F,2,32},{XKEY_T,2,64},{XKEY_X,2,128},
	{XKEY_7,3,1},{XKEY_Y,3,2},{XKEY_G,3,4},{XKEY_8,3,8},{XKEY_B,3,16},{XKEY_H,3,32},{XKEY_U,3,64},{XKEY_V,3,128},
	{XKEY_9,4,1},{XKEY_I,4,2},{XKEY_J,4,4},{XKEY_0,4,8},{XKEY_M,4,16},{XKEY_K,4,32},{XKEY_O,4,64},{XKEY_N,4,128},
	{XKEY_EQUAL,5,1},{XKEY_P,5,2},{XKEY_L,5,4},{XKEY_MINUS,5,8},{XKEY_PERIOD,5,16},{XKEY_APOS,5,32},{XKEY_TILDA,5,64},{XKEY_SLASH,5,128},
	{XKEY_RBRACK,6,1},{XKEY_COMMA,5,128},{XKEY_DOTCOM,6,4},{XKEY_HOME,6,8},{XKEY_RSHIFT,6,16},{XKEY_BSLASH,6,32},{XKEY_UP,6,64},{XKEY_BSLASH,6,128},
	{XKEY_1,7,1},{XKEY_LEFT,7,2},{XKEY_LCTRL,7,4},{XKEY_2,7,8},{XKEY_SPACE,7,16},{XKEY_RCTRL,7,32},{XKEY_Q,7,64},{XKEY_ESC,7,128},
	{0,0,0}
};

void c64_keyp(Computer* comp, keyEntry ent) {
	int idx = 0;
	while(c64matrix[idx].code > 0) {
		if (c64matrix[idx].code == ent.key) {
			comp->keyb->msxMap[c64matrix[idx].row] &= ~c64matrix[idx].mask;
		}
		idx++;
	}
}

void c64_keyr(Computer* comp, keyEntry ent) {
	int idx = 0;
	while(c64matrix[idx].code > 0) {
		if (c64matrix[idx].code == ent.key) {
			comp->keyb->msxMap[c64matrix[idx].row] |= c64matrix[idx].mask;
		}
		idx++;
	}
}
