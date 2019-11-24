#include <assert.h>
#include <string.h>
#include <time.h>

#include "../spectrum.h"

#define F_LORAM		1
#define	F_HIRAM		(1<<1)
#define	F_CHAREN	(1<<2)

// vicII read byte
// bits 00..13:vic ADR bus
// bits 14..15:cia2 reg #00 bit 0,1 inverted

unsigned char c64_vic_mrd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	adr &= 0x3fff;
	adr |= ((comp->vid->vbank & 3) << 14);
	return comp->mem->ramData[adr & comp->mem->ramMask];
}

// rom
// writing to rom will write to ram under rom placed above ram...

unsigned char c64_rom_rd(unsigned short adr, void* data) {
	Computer* comp = (Computer*)data;
	unsigned char res = 0xff;
	if (adr & 0x4000) {		// e000..ffff:kernal (1110..1111)
		res = comp->mem->romData[0x2000 + (adr & 0x1fff)];	// kernal (8K page 1)
	} else {			// a000..bfff:basic (1010..1011)
		res = comp->mem->romData[adr & 0x1fff];			// basic (8K page 0)
	}
	return res;
}

void c64_rom_wr(unsigned short adr, unsigned char val, void* data) {
	Computer* comp = (Computer*)data;
	comp->mem->ramData[adr & 0xffff] = val;
}

// d000..d3ff	vicII (0x3f registers + mirrors)

unsigned char c64_vic_rd(unsigned short adr, void* data) {
	Computer* comp = (Computer*)data;
	adr &= 0x3f;
	unsigned char res = comp->vid->regs[adr];
	switch (adr) {
		case 0x11:
			res &= 0x7f;
			if (comp->vid->ray.y & 0x100)
				res |= 0x80;
			break;
		case 0x12:
			res = comp->vid->ray.y & 0xff;
			break;
		case 0x19:
			res = comp->vid->intrq & VIC_IRQ_ALL;
			if (res)
				res |= 0x80;
			break;
	}
	return res;
}

void c64_vic_set_mode(Computer* comp) {
	int mode = ((comp->vid->regs[0x11] & 0x60) | (comp->vid->regs[0x16] & 0x10));
//	printf("mode:%.2X\n",mode);
	switch (mode) {
		case 0x00: vidSetMode(comp->vid, VID_C64_TEXT); break;
		case 0x10: vidSetMode(comp->vid, VID_C64_TEXT_MC); break;
		case 0x20: vidSetMode(comp->vid, VID_C64_BITMAP); break;
		case 0x30: vidSetMode(comp->vid, VID_C64_BITMAP_MC); break;
		// and extend modes
		default: vidSetMode(comp->vid, VID_UNKNOWN); break;
	}
}

void c64_vic_wr(unsigned short adr, unsigned char val, void* data) {
//	printf("vic wr %.4X,%.2X\n",adr,val);
	Computer* comp = (Computer*)data;
	adr &= 0x3f;
	comp->vid->regs[adr] = val;
	switch (adr) {
		case 0x11:				// b7 = b8 of INT line
			comp->vid->intp.y &= 0xff;
			if (val & 0x80)
				comp->vid->intp.y |= 0x100;
			c64_vic_set_mode(comp);
			break;
		case 0x12:				// b0..7 of INT line
			comp->vid->intp.y &= 0x100;
			comp->vid->intp.y |= (val & 0xff);
			break;
		case 0x16:
			c64_vic_set_mode(comp);
			break;
		case 0x18:
			comp->vid->sbank = (val & 0x0e) >> 1;
			comp->vid->cbank = (val & 0xf0) >> 4;
			break;
		case 0x19:
			comp->vid->inten = val & 0x0f;	// b0:line int, b1:spr-bgr collision, b2:spr-spr collision, b3:light pen
			break;
	}
}

// d400..d7ff	sid

unsigned char c64_sid_rd(unsigned short adr, void* data) {
	return 0xff;
}

void c64_sid_wr(unsigned short adr, unsigned char val, void* data) {

}

// d800..dbff	palette (screen attributes 40x25 = 1000 bytes)
// vid->colram

unsigned char c64_pal_rd(unsigned short adr, void* data) {
	Computer* comp = (Computer*)data;
	return comp->vid->colram[adr & 0x3ff];
}

void c64_pal_wr(unsigned short adr, unsigned char val, void* data) {
	Computer* comp = (Computer*)data;
	comp->vid->colram[adr & 0x3ff] = val;
}

// dc00/dd00 : cia 1 & cia 2 common ports
// all registers except 0,1 is the same (timer, tod, interrupts)

extern unsigned char toBCD(unsigned char);

unsigned char c64_cia_rd(c64cia* cia, unsigned char adr) {
	unsigned char res = 0xff;
	switch (adr & 0x0f) {
		case 0x02: res = cia->portA_mask; break;
		case 0x03: res = cia->portB_mask; break;
		case 0x04: res = cia->timerA.vall; break;
		case 0x05: res = cia->timerA.valh; break;
		case 0x06: res = cia->timerB.vall; break;
		case 0x07: res = cia->timerB.valh; break;
		case 0x08: res = toBCD(cia->time.tenth); break;
		case 0x09: res = toBCD(cia->time.sec); break;
		case 0x0a: res = toBCD(cia->time.min); break;
		case 0x0b: if (cia->time.hour < 12) {
				res = toBCD(cia->time.hour);
			} else {
				res = toBCD(cia->time.hour - 12) | 0x80;
			}
			break;
		case 0x0c: res = cia->ssr; break;
		case 0x0d:
			res = cia->state & cia->imask;
			res &= 0x7f;
			if (res)
				res |= 0x80;
			break;		// cia1:INT; cia2:NMI bits
		case 0x0e: res = cia->timerA.flags; break;
		case 0x0f: res = cia->timerB.flags; break;
	}
	return res;
}

void c64_cia_wr(c64cia* cia, unsigned char adr, unsigned char val) {
	switch (adr & 0x0f) {
		case 0x02: cia->portA_mask = val; break;
		case 0x03: cia->portB_mask = val; break;
		case 0x04: cia->timerA.inil = val; break;
		case 0x05: cia->timerA.inih = val; break;
		case 0x06: cia->timerB.inil = val; break;
		case 0x07: cia->timerB.inih = val; break;
		// NOTE: wait... wut?
		case 0x08: if (cia->timerB.flags & 0x80) {cia->alarm.tenth = val;} else {cia->time.tenth = val;} break;
		case 0x09: if (cia->timerB.flags & 0x80) {cia->alarm.sec = val;} else {cia->time.sec = val;} break;
		case 0x0a: if (cia->timerB.flags & 0x80) {cia->alarm.min = val;} else {cia->time.min = val;} break;
		case 0x0b: if (cia->timerB.flags & 0x80) {cia->alarm.hour = val;} else {cia->time.hour = val;} break;
		case 0x0c: cia->ssr = val; break;
		case 0x0d:
			if (val & 0x80)
				cia->imask |= (val & 0x7f);		// 1 - set state bits 0..6 where val bits is 1
			else
				cia->imask &= (~val & 0x7f);		// 0 - same but reset bits 0..6
			break;
		case 0x0e: cia->timerA.flags = val; break;
		case 0x0f: cia->timerB.flags = val; break;
	}
}

// dc00..dcff	cia1 (0x10 registers + mirrors)

unsigned char c64_cia1_rd(unsigned short adr, void* data) {
	Computer* comp = (Computer*)data;
	unsigned char res = 0xff;
	int idx;
	int row;
	adr &= 0x0f;
	switch (adr) {
		case 0x00:
			res = 0xff;		// joystick 1
			break;
		case 0x01:
			row = comp->c64.keyrow;
			res = 0xff;
			for (idx = 0; idx < 8; idx++) {
				if ((row & 1) == 0) {
					res &= comp->keyb->msxMap[idx];
				}
				row >>= 1;
			}
			break;
		default:
			res = c64_cia_rd(&comp->c64.cia1, adr & 0x0f);
			break;

	}
	//printf("cia1 rd %.2X = %.2X\n",adr,res);
	return res;
}

void c64_cia1_wr(unsigned short adr, unsigned char val, void* data) {
	Computer* comp = (Computer*)data;
	adr &= 0x0f;
	comp->c64.cia1.reg[adr] = val;
	//printf("cia1 wr %.2X,%.2X\n",adr,val);
	switch (adr) {
		case 0x00: comp->c64.keyrow = val; break;
		case 0x01:			// no write to port b?
			break;
		default:
			c64_cia_wr(&comp->c64.cia1, adr & 0x0f, val);
			break;
	}
}

// dd00..ddff	cia2 (0x10 registers + mirrors)

unsigned char c64_cia2_rd(unsigned short adr, void* data) {
	Computer* comp = (Computer*)data;
	unsigned char res = 0xff;
	adr &= 0x0f;
	switch (adr) {
		case 0x00:
			res = 0xff;	// rs232 line 1
			break;
		case 0x01:
			res = 0xff;	// rs232 line 2
			break;
		default:
			res = c64_cia_rd(&comp->c64.cia2, adr & 0x0f);
			break;

	}
	//printf("cia2 rd %.2X = %.2X\n",adr,res);
	return res;
}

void c64_cia2_wr(unsigned short adr, unsigned char val, void* data) {
	Computer* comp = (Computer*)data;
	adr &= 0x0f;
	comp->c64.cia2.reg[adr] = val;
	//printf("cia2 wr %.2X,%.2X\n",adr,val);
	switch (adr) {
		case 0x00:
			comp->vid->vbank = ~val & 3;
			// rs232 bits
			break;
		case 0x01:
			// rs232 bits
			break;
		default:
			c64_cia_wr(&comp->c64.cia2, adr & 0x0f, val);
			break;
	}
}

// de00..deff	io1

unsigned char c64_io1_rd(unsigned short adr, void* data) {
	return 0xff;
}

void c64_io1_wr(unsigned short adr, unsigned char val, void* data) {

}

// df00..dfff	io2

unsigned char c64_io2_rd(unsigned short adr, void* data) {
	return 0xff;
}

void c64_io2_wr(unsigned short adr, unsigned char val, void* data) {

}

// char rom

unsigned char c64_chrd(unsigned short adr, void* data) {
	Computer* comp = (Computer*)data;
	return comp->vid->font[adr & 0x7ff];
}

void c64_maper(Computer* comp) {
	// low 32K allways RAM
	memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_32K, NULL, NULL, NULL);
	// 0x8000: RAM or cartrige rom (TODO)
	memSetBank(comp->mem, 0x80, MEM_RAM, 4, MEM_8K, NULL, NULL, NULL);
	// 0xa000: RAM or BASIC (8K)
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
	for (i = 0; i < 16; i++) {
		comp->c64.cia1.reg[i] = 0x00;
		comp->c64.cia2.reg[i] = 0x00;
		comp->vid->pal[i] = c64_palette[i];
	}
	memset(comp->vid->regs, 0x00, 256);
//	comp->vid->regs[0x11] = 0x1b;
//	comp->vid->regs[0x18] = 0xc8;
	c64_maper(comp);
	vidSetMode(comp->vid, VID_C64_TEXT);
}

unsigned char c64_mrd(Computer* comp, unsigned short adr, int m1) {
	unsigned char res = 0xff;
	switch (adr) {
		case 0x0000:
			res = comp->c64.reg00;
			break;
		case 0x0001:
			res = comp->c64.reg01;
			break;
		default:
			res = memRd(comp->mem, adr);
	}
	return res;
}

void c64_mwr(Computer* comp, unsigned short adr, unsigned char val) {
	switch (adr) {
		case 0x0000:
			comp->c64.reg00 = val;
			break;
		case 0x0001:
			comp->c64.reg01 &= ~comp->c64.reg00;			// reset output bits in R0
			val &= comp->c64.reg00;					// reset ro bits in value
			comp->c64.reg01 |= val;					// set new output bits in R0
			c64_maper(comp);
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

void c64_sync_time(ciaTime* xt, int ns) {
	xt->ns += ns;
	while (xt->ns >= 1e8) {		// 1/10 sec
		xt->ns -= 1e9;
		xt->tenth++;
		if (xt->tenth > 9) {
			xt->tenth = 0;
			xt->sec++;
			if (xt->sec > 59) {
				xt->sec = 0;
				xt->min++;
				if (xt->min > 59) {
					xt->min = 0;
					xt->hour++;
					if (xt->hour > 23) {
						xt->hour = 0;
					}
				}
			}
		}
	}
}

void c64_sync(Computer* comp, int ns) {
	if (comp->vid->intrq & VIC_IRQ_ALL) {
		comp->cpu->intrq = (comp->cpu->inten & MOS6502_INT_IRQ);
		comp->vid->intrq = 0;
	}
	c64_sync_time(&comp->c64.cia1.time, ns);
	c64_sync_time(&comp->c64.cia2.time, ns);
	// if tape signal goes hi->low, set b4,0D@CIA1
	if (comp->tape->on) {
		int vol = comp->tape->volPlay;
		tapSync(comp->tape, ns);
		if ((vol > 0x7f) && (comp->tape->volPlay < 0x80)) {		// front 1->0
			comp->c64.cia1.state |= 0x10;
			comp->c64.cia1.state &= comp->c64.cia1.imask;
		}
	}
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
