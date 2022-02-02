// modes: https://osdev.fandom.com/ru/wiki/VGA_режимы

#include <stdio.h>
#include <string.h>

#include "vga.h"

//#define CGA_MODE 0

static const int ega_def_idx[16] = {0,1,2,3,4,5,20,7,56,57,58,59,60,61,62,63};

void vga_reset(Video* vid) {
	vidSetMode(vid, CGA_TXT_H);

	int i;
	xColor xcol;
	for (i = 0; i < 64; i++) {
		xcol.r = 0x55 * (((i >> 1) & 2) + ((i >> 5) & 1));
		xcol.g = 0x55 * ((i & 2) + ((i >> 4) & 1));
		xcol.b = 0x55 * (((i << 1) & 2) + ((i >> 3) & 1));
		vid->pal[i] = xcol;
	}
	vid->pal[6]=vid->pal[0x14];		// FIXME: ORLY?
	if (vid->vga.cga) {
		memcpy(vid->ram + 0x20000, vid->font, 0x2000);	// copy default font
		for (i = 0; i < 0x10; i++) {			// set default palette
			ATR_REG(i) = ega_def_idx[i];
		}
		vid_set_resolution(vid, 320, 200);
	}
}

static int vga_scr_height[4] = {200,350,400,480};
static int vga_scr_width[4] = {320,640,320,640};

void vga_upd_mode(Video* vid) {
	// reg[0x42] bit 2,3 = hres (00:320, 01:640), bit 6,7=vres(00:200,01:350,10:400,11:480 lines)
	// GRF_REG[0x06] bit0: 1:gfx, 0:text
	int mod = (vid->reg[0x42] & 0x0c) | (GRF_REG(6) & 1);
	printf("mod = %i\n",mod);
	switch(mod) {
		case 0: vidSetMode(vid, CGA_TXT_L); break;	// T40
		case 1: vidSetMode(vid, VGA_GRF_L); break;	// G320
		case 4: vidSetMode(vid, CGA_TXT_H); break;	// T80 (same T40)
		case 5: vidSetMode(vid, VGA_GRF_H); break;	// G640
		default:	// others is undefined
			break;
	}
	int ry = vga_scr_height[(vid->reg[0x42] >> 6) & 3];
	int rx = vga_scr_width[(vid->reg[0x42] >> 2) & 3];
	vid_set_resolution(vid, rx, ry);
}

// 3c2 (r42) b0 = 0:select 3bx, 1:select 3dx
int vga_rd(Video* vid, int port) {
	int res = -1;
	if (!vid->vga.cga && 0) {
		if (((port & 0x3f8) == 0x3b0) && (vid->reg[0x42] & 1)) return res;
		if ((port == 0x3ba) && (vid->reg[0x42] & 1)) return res;
		if (((port & 0x3f8) == 0x3d0) && !(vid->reg[0x42] & 1)) return res;
		if ((port == 0x3da) && !(vid->reg[0x42] & 1)) return res;
	}
	switch (port) {
		// ISR 0
		// b0-3 unused
		// b4: switch state. b2,3 of out 3c2 = switch nr.
		// b5,6: feature code
		// b7: CRT interrrupt (set @ start of vblank)
		case 0x3c2:
			res = 0;
			port = (vid->reg[0x42] >> 2) & 3;	// switch nr
			if (0b1001 & (1 << port)) res |= 0x10;	// switch sense
			// if (vid->intrq) res |= 0x80;
			break;
		case 0x3b5:
		case 0x3d5:		// CRT registers C.. may be readed
			if ((CRT_IDX >= 0x0c) && (CRT_IDX <= VGA_CRC))
				res = CRT_CUR_REG;
			break;
		// ISR 1
		// b0:videomem is free (vblank||hblank)
		// b1:[EGA]light pen trigger set (=0)
		// b2:[EGA]light pen switch open (=0)
		// b3:vertical retrace
		// b4,5: color bits outed to 3C0: ATR_REG(0x12) b5,4 = 00:BR, 01:rg, 10:bG, 11:--
		// b6,7: reserved
		case 0x3ba:
		case 0x3da:
			res = 0;
			if (vid->vblank)		// not vblank, btw
				res |= 8;
			if (vid->vbank || vid->hblank)
				res |= 1;
			vid->vga.atrig = 0;
			break;
	}
	return res;
}

void vga_wr(Video* vid, int port, int val) {
	if (!vid->vga.cga && 0) {
		if (((port & 0x3f8) == 0x3b0) && (vid->reg[0x42] & 1)) return;
		if ((port == 0x3ba) && (vid->reg[0x42] & 1)) return;
		if (((port & 0x3f8) == 0x3d0) && !(vid->reg[0x42] & 1)) return;
		if ((port == 0x3da) && !(vid->reg[0x42] & 1)) return;
	}
	switch (port) {
		// crt registers (3d4/3d5)
		case 0x3b4:
		case 0x3d4:
			CRT_IDX = val & 0xff;
			break;
		case 0x3b5:
		case 0x3d5:
			if (CRT_IDX <= VGA_CRC) {
				CRT_CUR_REG = val & 0xff;
				if (CRT_IDX == 0x11) {
					if (!(val & 0x10)) vid->intrq = 0;
					vid->inten = (val & 0x20) ? 0 : 1;
				}
			}
			vid->vga.cadr = ((CRT_REG(0x0e) << 8) | (CRT_REG(0x0f))) + ((CRT_REG(0x0b) >> 5) & 3);	// cursor address
			break;
		// sequencer registers (3c4/3c5)
		case 0x3c4:
			SEQ_IDX = val & 0xff;
			break;
		case 0x3c5:
			if (SEQ_IDX <= VGA_SRC) {
				SEQ_CUR_REG = val & 0xff;
			}
			break;
		// graphics registers (3ce/3cf)
		case 0x3ce:
			GRF_IDX = val & 0xff;
			break;
		case 0x3cf:
			if (GRF_IDX <= VGA_GRC) {
				GRF_CUR_REG = val & 0xff;
				if (GRF_IDX == 6)
					vga_upd_mode(vid);
			}
			break;
		// atribute registers (3c0)
		case 0x3c0:
			if (vid->vga.atrig) {	// data
				ATR_CUR_REG = val & 0xff;
			} else {		// index
				ATR_IDX = val & 0xff;
			}
			vid->vga.atrig = !vid->vga.atrig;
			break;
		case 0x3c2:
			// b0: 0 for 3b?, 1 for 3d? ports access
			// b1: enable ram access
			// b2,3: clock rate (00:320px, 01:640px, 10:external 11:not used) = switch num?
			// b4: [EGA] disable internal drivers
			// b5: page bit for odd/even mode
			// b6,7: lines 00:200, 01:350, 10:400, 11:480
			vid->reg[0x42] = val & 0xff;		// misc.output register
			vga_upd_mode(vid);
			break;
		// no such register in EGA/VGA, modes is in other registers, this is CGA register
		case 0x3d8:
			if (!vid->vga.cga) break;
			// cga	b0	0:text 40, 1:text 80
			//	b1	1:for grf 320 0:for others
			//	b2
			//	b3	enable display
			//	b4	1:for grf 640 0:for others
			//	b5	blink disabled (b7 = bg intensivity)
			vid->reg[0xff] = val & 0xff;
			switch (val & 0x13) {
				case 0x00: case 0x10:
					vidSetMode(vid, CGA_TXT_L);
					break;		// text 40x25
				case 0x01: case 0x11:
					vidSetMode(vid, CGA_TXT_H);
					break;		// text 80x25
				case 0x02: case 0x03:
					vidSetMode(vid, CGA_GRF_L);
					break;		// gfx 320x200
				case 0x12: case 0x13:
					vidSetMode(vid, CGA_GRF_H);
					break;		// gfx 640x200
			}
			if (val & 0x04) {}		// b2:monochrome
			if (val & 0x08) {}		// b3:video enabled
			vid->vga.blinken = (val & 0x20) ? 1 : 0;
			break;
		case 0x3d9: // cga palette register
			if (!vid->vga.cga) break;
			// b5: 0: blk,red,grn,brown; 1:blk,cyan,magenta,white
			// b4: grf:intense colors; txt:bgcolor colors text
			// b3: txt:intense border color; grf: intense bg in 320x200, intense fg in 640x200
			// b0..2: BGR color for... txt:border, grf320:background(blk), grf640:foreground
			vid->reg[0x59] = val & 0xff;
			break;
		case 0x3ba:
		case 0x3da:		// [EGA] feature control regiser
			// b0,1: fc0,fc1 to feature device
			vid->reg[0x5a] = val & 0xff;
			break;
	}
}

// A0000..AFFFF : gfx mode plane (64K)
// B0000..B7FFF : monochrome mode plane (32K)
// B8000..BFFFF : color text mode plane (32K)

// VGA_R06h,bit2,3:This field specifies the range of host memory addresses that is decoded by the VGA hardware and mapped into display memory accesses
// 00:A0000..BFFFF (128K) - no b000/b800 mapings
// 01:A0000..AFFFF (64K)
// 10:B0000..B7FFF (32K)
// 11:B8000..BFFFF (32K)

#define VGA_DIRECT (1<<24)

int vga_adr(Video* vid, int exadr) {
	int adr = -1;
	if (vid->vga.cga) {
		if ((exadr >= 0xa0000) && (exadr < 0xb0000)) {
			adr = exadr & 0xffff;
		} else if (exadr < 0xbffff) {
			adr = ((exadr & 0x7fff) >> 1) | VGA_DIRECT;
			if (exadr & 1)
				adr += MEM_64K;
		}
	} else {
#if 0
		switch(GRF_REG(6) & 0x0c) {
			case 0x00: adr = exadr & 0x1ffff; break;
			case 0x04: if (exadr < MEM_64K) adr = exadr; break;
			case 0x08:
				if (exadr < 0xb0000) break;
				if (exadr > 0xb7fff) break;
				adr = ((exadr & 0x7fff) >> 1) | VGA_DIRECT;
				if (exadr & 1)
					adr += MEM_64K;
				break;
			case 0x0c: if (exadr < 0xb8000) break;
				adr = ((exadr & 0x7fff) >> 1) | VGA_DIRECT;
				if (exadr & 1)
					adr += MEM_64K;
				break;
		}
#else
		if (exadr < 0xb0000) {
			adr = exadr & 0x1ffff;
		} else {
			adr = ((exadr & 0x7fff) >> 1) | VGA_DIRECT;
			if (exadr & 1)
				adr += MEM_64K;
		}
	}
#endif
	return adr;
}

// GRF_R05.b0,1 = write mode
// write mode 00:
// SEQ_R02.b0-3: 1-enable plane to write, 0-disable
// GRF_R08.b0-7: 1-write bit from cpu, 0-doesn't change or set/reset (see below)
// GRF_R01.b0-3: 1-enable set/reset for plane, 0-disable
// GRF_R00.b0-3: 1-set plane, 0-reset plane
// write mode 01:
// write latches to video mem
// write mode 02:
// cpu data b0-3 = color (0..15) = bits for planes 0-3
// GRF_R08.b0..7: 1=write color bits to planes at this bit-position, 0=skip

void vga_mwr(Video* vid, int adr, int val) {
	if (!(vid->reg[0x42] & 2) && !vid->vga.cga) return;	// vmem disabled
	adr = vga_adr(vid, adr);
	if (adr < 0) return;
	if (adr & VGA_DIRECT) {			// b000/b800 buffers
		vid->ram[adr ^ VGA_DIRECT] = val & 0xff;
	} else if (vid->vga.cga) {				// write to plane
		vid->ram[adr] = val & 0xff;
	} else {
		int bt,lay;
		if (GRF_REG(3) & 7) {		// rotation
			val &= 0xff;
			val |= (val << 8);
			val >>= (GRF_REG(3) & 7);
		}
		for (lay = 0; lay < 4; lay++) {
			switch ((GRF_REG(3) & 0x18) >> 3) {	// logical operation with latches
				case 0: break;					// 00 no changes
				case 1: val &= vid->vga.latch[lay]; break;	// 01 AND
				case 2: val |= vid->vga.latch[lay]; break;	// 10 OR
				case 3: val ^= vid->vga.latch[lay]; break;	// 11 XOR
			}
			switch(GRF_REG(5) & 3) {		// write mode
				case 0:
					if (SEQ_REG(2) & (1 << lay)) {			// map mask bit = 1, changing enabled
						if (GRF_REG(1) & (1 << lay)) {		// set-reset enabled for this layer
							bt = (GRF_REG(0) & (1 << lay)) ? 0xff : 0x00;	// set or reset
						} else {
							bt = vid->ram[adr];		// else - byte from layer
						}
						bt &= ~GRF_REG(8);			// reset bits alowed from cpu
						bt |= (val & GRF_REG(8));		// write bits alowed from cpu
						vid->ram[adr] = bt;			// write modified byte to layer
					}
					break;
				case 1:
					vid->ram[adr] = vid->vga.latch[lay];
					break;
				case 2:
					bt = (val & (1 << lay)) ? 0xff : 0x00;
					vid->ram[adr] &= ~GRF_REG(8);
					vid->ram[adr] |= (val & GRF_REG(8));
					break;
				case 3:					// VGA only
					break;
			}
			adr += MEM_64K;		// move to next layer
		}
	}
}

// GRF_R05.bit3 = read mode
// read mode 0:
// GRF_R04.bit0,1 = plane to read byte from
// read mode 1:
// GRF_R02.b0-3 = color to compare (0..15)
// GRF_R07.b0-3 = 0: don't care about this plane for color compare

int vga_mrd(Video* vid, int adr) {
	int res = -1;
	if (!(vid->reg[0x42] & 2) && !vid->vga.cga) return res;
	int col;
	int msk;
	adr = vga_adr(vid, adr);
	if (adr < 0) return -1;
	if (adr & VGA_DIRECT) {		// b000/b800
		res = vid->ram[adr ^ VGA_DIRECT];
	} else if (vid->vga.cga) {
		res = vid->ram[adr];
	} else {			// a000:planes
		for(res = 0; res < 4; res++)		// store latches
			vid->vga.latch[res] = vid->ram[(adr & 0xffff) | (res << 16)];
		if (GRF_REG(0x05) & 8) {		// color compare mode
			res = 0;
			msk = 1;
			do {
				col = 0;
				if (vid->vga.latch[0] & msk) col |= 1;		// get color from latches
				if (vid->vga.latch[1] & msk) col |= 2;
				if (vid->vga.latch[2] & msk) col |= 4;
				if (vid->vga.latch[3] & msk) col |= 8;
				if (!((GRF_REG(2) ^ col) & GRF_REG(7) & 0x0f))	// check if color matched
					res |= msk;
				msk <<= 1;
			} while (msk > 0x100);
		} else {
			res = vid->vga.latch[GRF_REG(0x04) & 3];
		}
	}
	return res;
}

// CGA/EGA/VGA drawings
// T40
// Horiz: each 8(9) dots: take char,atr
// char addr = plane0 + (line * 40) + x
// char data addr = plane2 + (charset * 0x2000) + (char * 32) + in_char_line
// Vert: each line in_char_line++
// if in_char_line==CRT_R09h.bits0-4 {in_char_line=0; line++;}
// :: if line==CRT_R12h {HBlank=1}
// :: else if line==CRT_R06h {frame_end}

// SEQ_REG(4).b2 - odd/even mode for cpu addresses; 0:odd addresses to odd plane, even-to even; 1:normal mode
// GRF_REG(5).b4 - odd/even mode; 1:on 0:off

void vga_check_vsync(Video* vid) {
	int vs_start;
	if (vid->vga.cga) {
		vs_start = CRT_REG(7);
	} else {
		vs_start = (CRT_REG(0x10) | ((CRT_REG(7) & 4) << 6));
	}
	int vs_end = vs_start + (CRT_REG(0x11) & 0x0f);
	if (vid->ray.y == vs_start) vid->vsync = 1;
	else if (vid->ray.y == vs_end) vid->vsync = 0;
}

void cga_t40_frm(Video* vid) {
	vid->vga.line = 0;
	vid->vga.chline = 0;	// CRT_REG(0x08) & 0x1f;
}

void cga_t40_line(Video* vid) {
	memset(vid->line, CRT_REG(0x11), 0x400);
	int i,t;
	vid->xpos = 0;
	vid->vadr = (CRT_REG(0x0c) << 8) | (CRT_REG(0x0d));	// C,D: start address registers
	if (vid->vga.cga) {
		vid->vadr += vid->vga.line * CRT_REG(1);		// CRT_R1 chars in line
	} else {
		vid->vadr += vid->vga.line * (CRT_REG(1)+1);
	}
	for (t = 0; t <= CRT_REG(1); t++) {
		// char / atr taken right way (checked)
		vid->idx = vid->ram[vid->vadr];			// char (plane 0)
		vid->atrbyte = vid->ram[vid->vadr + MEM_64K];	// attr	(plane 1)
		vid->tadr = vid->idx * 32;			// offset of 1st char byte in plane 2 (allways 32 bytes/char in font plane)
		vid->tadr += vid->vga.chline;			// +line in char
		vid->idx = vid->ram[0x20000 + vid->tadr];	// pixels
		if ((vid->vadr == vid->vga.cadr) && !(CRT_REG(0x0a) & 0x20)) {		// cursor position, cursor enabled
			if ((vid->vga.chline > (CRT_REG(0x0a) & 0x1f)) \
				&& (vid->vga.chline <= (CRT_REG(0x0b) & 0x1f))) {	// cursor start/end
				vid->idx ^= 0xff;
			}
		}
		if ((vid->atrbyte & 0x80) && vid->flash && vid->vga.blinken)	// blinking (b7 attribute)
			vid->idx ^= 0xff;
		for (i = 0; i < 8; i++) {
			if (vid->idx & 0x80) {
				vid->line[vid->xpos++] = ATR_REG(vid->atrbyte & 0x0f);		// b0..3:foreground
			} else {
				vid->line[vid->xpos++] = ATR_REG((vid->atrbyte >> 4) & 0x07);	// b4..6:background
			}
			vid->idx <<= 1;
		}
		vid->vadr++;
	}
	vid->vga.chline++;
	if (vid->vga.chline > CRT_REG(9)) {			// char height register
		vid->vga.line++;
		vid->vga.chline = 0;
	}
	if ((vid->ray.y == vid->blank.y) && (vid->inten & 1))
		vid->intrq = 1;
	vga_check_vsync(vid);
}

void cga_t40_dot(Video* vid) {
	vid_dot_full(vid, vid->line[vid->ray.x]);		// 40x8=320
}

// T80

void cga_t80_dot(Video* vid) {
	if (vid->vga.cga) {
		vid_dot_half(vid, vid->line[vid->ray.x << 1]);		// 80x8=640
		vid_dot_half(vid, vid->line[(vid->ray.x << 1) + 1]);
	} else {
		vid_dot_full(vid, vid->line[vid->ray.x]);
	}
}

// res = HResolution (320 or 640)
void vga_4bpp(Video* vid, int res) {
	memset(vid->line, CRT_REG(0x11), 0x400);
	int pos = 0;
	int i,k,mask;
	unsigned char col,b0,b1,b2,b3;
	vid->vadr = (vid->ray.y) * (res / 8);
	if (!(CRT_REG(0x17) & 1)) {		// odd lines at +8Kb
		vid->vadr &= ~(1 << 13);
		if (vid->ray.y & 1) vid->vadr |= (1 << 13);
	}
	for (i = 0; i < res/8; i++) {
		b0 = vid->ram[vid->vadr];
		b1 = vid->ram[vid->vadr + 0x10000];
		b2 = vid->ram[vid->vadr + 0x20000];
		b3 = vid->ram[vid->vadr + 0x30000];
		vid->vadr++;
		mask = 0x80;
		for (k = 0; k < 8; k++) {
			col = 0;
			if (b3 & mask) col |= 8;
			if (b2 & mask) col |= 4;
			if (b1 & mask) col |= 2;
			if (b0 & mask) col |= 1;
			vid->line[pos++] = ATR_REG(col);
			mask >>= 1;
		}
	}
}

// G320

static unsigned char cga_set_0[4] = {0, 4, 2, 20};
static unsigned char cga_set_1[4] = {0, 3, 5, 7};

unsigned char cga_to_ega(Video* vid, unsigned char c) {
	c &= 3;
	c = (vid->reg[0x59] & 0x20) ? cga_set_1[c] : cga_set_0[c];
	if (c == 0) {
		c = vid->reg[0x59] & 7;		// background color (320x200)
		if ((vid->reg[0x59] & 8) && c)
			c |= 8;
	}
	if ((vid->reg[0x59] & 0x10) && c)
		c |= 8;
	return ATR_REG(c);
}

// cga mode
void cga320_2bpp_line(Video* vid) {
	memset(vid->line, CRT_REG(0x11), 0x400);
	int pos = 0;
	int i;
	unsigned char bt;
	vid->vadr = (vid->ray.y >> 1) * CRT_REG(1) * 2;		// crt_reg(1) * 8 / 4 = line width (bytes)
	vid->vadr += (CRT_REG(0x0c) << 8) | (CRT_REG(0x0d));		// start address
	if (vid->ray.y & 1)
		vid->vadr |= 0x2000;
	for (i = 0; i <= CRT_REG(1) * 2; i++) {
		if (vid->vadr & 1) {
			bt = vid->ram[(vid->vadr >> 1) + 0x10000];
		} else {
			bt = vid->ram[vid->vadr >> 1];
		}
		vid->vadr++;
		vid->line[pos++] = cga_to_ega(vid, bt >> 6);
		vid->line[pos++] = cga_to_ega(vid, bt >> 4);
		vid->line[pos++] = cga_to_ega(vid, bt >> 2);
		vid->line[pos++] = cga_to_ega(vid, bt);
	}
	vga_check_vsync(vid);
}

void vga320_4bpp_line(Video* vid) {
	vga_4bpp(vid, 320);
	vga_check_vsync(vid);
}

// G640

void cga640_1bpp_line(Video* vid) {
	memset(vid->line, CRT_REG(0x11), 0x400);
	int pos = 0;
	int i,k;
	unsigned char bt;
	vid->vadr = (vid->ray.y >> 1) * CRT_REG(1) * 2;		// crt_reg(1) * 8 / 4 = line width (bytes)
	vid->vadr += (CRT_REG(0x0c) << 8) | (CRT_REG(0x0d));	// start address
	//if (vid->ray.y & 1)
	//	vid->vadr |= 0x2000;
	for (i = 0; i <= CRT_REG(1) * 2; i++) {
#if 1
		bt = vid->ram[vid->vadr & (MEM_256K-1)];
#else
		if (vid->vadr & 1) {
			bt = vid->ram[(vid->vadr >> 1) + 0x10000];
		} else {
			bt = vid->ram[vid->vadr >> 1];
		}
#endif
		vid->vadr++;
		for (k = 0; k < 8; k++) {
			vid->line[pos++] = (bt & 0x80) ? 7 : 0;
			bt <<= 1;
		}
	}
	vga_check_vsync(vid);
}

void vga640_4bpp_line(Video* vid) {
	vga_4bpp(vid, 640);
	vga_check_vsync(vid);
}
