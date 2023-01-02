// modes: https://osdev.fandom.com/ru/wiki/VGA_режимы

#include <stdio.h>
#include <string.h>

#include "vga.h"

//#define CGA_MODE 0

// stretch x2
// when SEQ(1).b3=1
void cga_lores_dot(Video* vid) {
	vid_dot_full(vid, vid->line[vid->ray.x]);
	vid_dot_full(vid, vid->line[vid->ray.x]);
}

// normal mode
// when SEQ(1).b3=0
void cga_t40_dot(Video* vid) {
	vid_dot_full(vid, vid->line[vid->ray.x]);
}

// CGA hires (640) modes, showing in 320 columns (compressed x2)
void ega_hires_dot(Video* vid) {
	vid_dot_half(vid, vid->line[vid->ray.x << 1]);
	vid_dot_half(vid, vid->line[(vid->ray.x << 1) | 1]);
}

static int vga_scr_height[4] = {200,350,400,480};

void vga_upd_mode(Video* vid) {
	// 3C2.bit 2,3 = clock select (00:14MHz, 01:16MHz, 10:from ft.connector, 11:not used
	// 3C2.bit 6,7 = vres(00:200,01:350,10:400,11:480 lines)
	// SEQ(1).b3: 1=divide dot clock by 2 (T40 or G320)
	// GRF_REG[0x06] bit0: 1:gfx, 0:text
	int mod = (SEQ_REG(1) & 8) | (GRF_REG(6) & 1);
	printf("ega mode = %i\n",mod);
	switch(mod) {
		case 0: vidSetMode(vid, CGA_TXT_H); break;	// T80
		case 1: vidSetMode(vid, VGA_GRF_H); break;	// G640
		case 8: vidSetMode(vid, CGA_TXT_L); break;	// T40
		case 9: vidSetMode(vid, VGA_GRF_L); break;	// G320
		default:	// others is undefined
			break;
	}
	int rx = 640;	// (SEQ_REG(1) & 8) ? 320 : 640;
	int ry = vga_scr_height[(vid->reg[EGA_3C2] >> 6) & 3];
	vid->linedbl = (vid->vga.cga || (ry == 200));
	if (vid->linedbl) ry <<= 1;
	vid->cbDot = (SEQ_REG(1) & 8) ? cga_lores_dot : cga_t40_dot;
	vid_set_resolution(vid, rx, ry);
}

static const int ega_def_idx[16] = {0,1,2,3,4,5,20,7,56,57,58,59,60,61,62,63};

void vga_reset(Video* vid) {
	int i;
	xColor xcol;
	for (i = 0; i < 64; i++) {
		xcol.r = 0x55 * (((i >> 1) & 2) + ((i >> 5) & 1));
		xcol.g = 0x55 * ((i & 2) + ((i >> 4) & 1));
		xcol.b = 0x55 * (((i << 1) & 2) + ((i >> 3) & 1));
		vid_set_col(vid, i, xcol);
	}
	vid->pal[6]=vid->pal[0x14];		// FIXME: ORLY?
	if (vid->vga.cga) {
		memcpy(vid->ram + 0x20000, vid->font, 0x2000);	// copy default font
		for (i = 0; i < 0x10; i++) {			// set default palette
			ATR_REG(i) = ega_def_idx[i];
		}
		vid_set_resolution(vid, 320, 200);
	}
	SEQ_REG(1) = 0;
	GRF_REG(6) = 0;
	vga_upd_mode(vid);
}

/* switches (initial video mode) (?)
0100	mda
1001	cga 40x25
1000	cga 80x25
0111	ega normal (8x8 chars)
0110	ega enchanced (8x14 chars)
*/

// 3c2 (r42) b0 = 0:select 3bx, 1:select 3dx
int vga_rd(Video* vid, int port) {
	int res = -1;
	if (!vid->vga.cga && 0) {
		if (((port & 0x3f8) == 0x3b0) && (vid->reg[EGA_3C2] & 1)) return res;
		if ((port == 0x3ba) && (vid->reg[EGA_3C2] & 1)) return res;
		if (((port & 0x3f8) == 0x3d0) && !(vid->reg[EGA_3C2] & 1)) return res;
		if ((port == 0x3da) && !(vid->reg[EGA_3C2] & 1)) return res;
	}
	switch (port) {
		// ISR 0
		// b0-3 unused
		// b4: switch state. b2,3 of out 3c2 = switch nr.
		// b5,6: feature code
		// b7: CRT interrrupt (set @ start of vblank)
		case 0x3c2:
			res = 0;
			port = (vid->reg[EGA_3C2] >> 2) & 3;	// switch nr
			if (0b1001 & (1 << port)) res |= 0x10;	// switch sense (0110/0111 ?)
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
			if (vid->vblank || vid->hblank)
				res |= 1;
			vid->vga.atrig = 0;
			break;
	}
	return res;
}

void vga_wr(Video* vid, int port, int val) {
	if (!vid->vga.cga && 0) {
		if (((port & 0x3f8) == 0x3b0) && (vid->reg[EGA_3C2] & 1)) return;
		if ((port == 0x3ba) && (vid->reg[EGA_3C2] & 1)) return;
		if (((port & 0x3f8) == 0x3d0) && !(vid->reg[EGA_3C2] & 1)) return;
		if ((port == 0x3da) && !(vid->reg[EGA_3C2] & 1)) return;
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
			vid->vga.blinken = (ATR_REG(0x10) & 8) ? 1 : 0;
			break;
		case 0x3c2:
			// b0: 0 for 3b?, 1 for 3d? ports access
			// b1: enable ram access
			// b2,3: clock rate (00:320px, 01:640px, 10:external 11:not used) = switch num?
			// b4: [EGA] disable internal drivers
			// b5: page bit for odd/even mode
			// b6,7: lines 00:200, 01:350, 10:400, 11:480
			vid->reg[EGA_3C2] = val & 0xff;		// misc.output register
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
			vid->reg[CGA_3D8] = val & 0xff;
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
			vid->reg[CGA_3D9] = val & 0xff;
			break;
		case 0x3ba:
		case 0x3da:		// [EGA] feature control regiser
			// b0,1: fc0,fc1 to feature device
			vid->reg[CGA_3DA] = val & 0xff;
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

// GRF_R05.b4: odd/even mode
// GRF_R06.b1: chain odd/even (2=0, 3=1)

void vga_mwr(Video* vid, int adr, int val) {
	adr &= (adr < 0xb0000) ? 0xffff : 0x7fff;
	if (vid->vga.cga) {
		if (adr & 1) adr |= 0x20000;
		adr >>= 1;
		vid->ram[adr] = val;
	} else if ((vid->reg[EGA_3C2] & 2)) {		// vmem enabled
		unsigned char wmsk = SEQ_REG(2) & 0x0f;	// mapmask (layers write-enable)
		unsigned char bmsk = GRF_REG(8);	// bitmask
		if (GRF_REG(5) & 0x10) {		// odd/even mode
			if (adr & 1) wmsk &= 0x0a; else wmsk &= 0x05;
			//wmsk &= 0x05;			// 0,2
			//if (adr & 1) wmsk <<= 1;	// 1,3
			adr >>= 1;
		}
		int bt,lay,lm;
		for (lay = 0; lay < 4; lay++) {
			lm = (1 << lay);
			if (wmsk & lm) {
				switch (GRF_REG(5) & 3) {
					case 0:
						if (GRF_REG(1) & lm) {		// set-reset enabled for this layer
							bt = (GRF_REG(0) & lm) ? 0xff : 0x00;	// set or reset
						} else {
							bt = val;
						}
						if (GRF_REG(3) & 7) {		// rotation, affect only mode 0
							bt &= 0xff;
							bt |= (bt << 8);
							bt >>= (GRF_REG(3) & 7);
							bt &= 0xff;
						}
						break;
					case 1:
						bt = vid->vga.latch[lay];	// mode 1: write latch, don't apply alu
						break;
					case 2:
						bt = (val & lm) ? 0xff : 0x00;	// mode 2: write color to pixels enabled by bitmask register
						break;
					case 3:
						bt = GRF_REG(0) & bmsk;	// mode 3: write set/reset reg & bitmask
						break;
				}
				if (!(GRF_REG(5) & 1)) {		// modes 1 and 3 doesn't affected by alu/bitmask
					switch ((GRF_REG(3) & 0x18) >> 3) {			// logical operation with latches
						case 1: bt &= vid->vga.latch[lay]; break;	// 01 AND
						case 2: bt |= vid->vga.latch[lay]; break;	// 10 OR
						case 3: bt ^= vid->vga.latch[lay]; break;	// 11 XOR
					}
					bt &= bmsk;					// apply bitmask
					bt |= (vid->vga.latch[lay] & ~bmsk); //(vid->ram[adr] & ~bmsk);
				}
				vid->ram[adr] = bt;
				// if ((adr == 6) && (bt == 0x7c)) vid->xirq(IRQ_BRK, vid->data);	// maniac mansion, upper text
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
	adr &= (adr < 0xb0000) ? 0xffff : 0x7fff;
	if (vid->vga.cga) {
		if (adr & 1) adr |= 0x20000;
		adr >>= 1;
		res = vid->ram[adr] & 0xff;
	} else if (vid->reg[EGA_3C2] & 2) {
		int col;
		int msk;
		int rp = GRF_REG(4) & 3;
		if (GRF_REG(5) & 0x10) {
			rp = (rp & 2) | (adr & 1);
			adr = (adr >> 1) | ((adr & 1) ? MEM_64K : 0);
		}

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
			res = vid->vga.latch[rp];
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

// ATR_REG(10).b0 : 1:gfx, 0:txt
//	gfx: if GRF_REG(5).bit5=1:cga4; else if GRF_REG(6).b2,3=11:cga2; else ega;
//	txt: txt

void vga_check_vsync(Video* vid) {
	int vs_start;
	if (vid->vga.cga) {
		vs_start = CRT_REG(7);
	} else {
		vs_start = (CRT_REG(0x10) | ((CRT_REG(7) & 4) << 6));
	}
	int vs_end = vs_start + (CRT_REG(0x11) & 0x0f);
	if (vid->ray.y == vs_start) {
		vid->vsync = 1;
	} else if (vid->ray.y == vs_end) {
		vid->vsync = 0;
	}
}

void cga_t40_ini(Video* vid) {
	vid->vga.cpl = 40;
}

void cga_t80_ini(Video* vid) {
	vid->vga.cpl = 80;
}

void cga_t40_frm(Video* vid) {
	vid->vga.line = 0;
	vid->vga.chline = 0;	// CRT_REG(0x08) & 0x1f;
	vid->bgadr = (CRT_REG(0x0c) << 8) | (CRT_REG(0x0d));	// screen start address
	vid->vadr = vid->bgadr;
	vid->fadr = 0;		// fetches counter (index) for ega graphics modes
}

void cga_t40_line(Video* vid) {
	memset(vid->line, CRT_REG(0x11), 0x400);
//	if (vid->vga.line >= CRT_REG(0x12)) return;
	int i,t;
	vid->xpos = 0;
	vid->tadr = vid->vadr;					// current line adr
	for (t = vid->vga.cga ? 1 : 0; t <= CRT_REG(1); t++) {
		vid->idx = vid->ram[vid->vadr];			// char (plane 0)
		vid->atrbyte = vid->ram[vid->vadr + MEM_64K];	// attr	(plane 1)
		vid->fadr = vid->idx * 32;			// offset of 1st char byte in plane 2 (allways 32 bytes/char in font plane)
		vid->fadr += vid->vga.chline;			// +line in char
		if (!vid->vga.cga && (!vid->vga.blinken || !(vid->atrbyte & 0x08))) {	// font select (each 8K)
			vid->fadr += (SEQ_REG(3) & 0x03) << 13;
		} else {
			vid->fadr += (SEQ_REG(3) & 0x0c) << 11;
		}
		vid->idx = vid->ram[0x20000 + vid->fadr];	// pixels
		if ((vid->vadr == vid->vga.cadr) && !(CRT_REG(0x0a) & 0x20)) {		// cursor position, cursor enabled
			if ((vid->vga.chline > (CRT_REG(0x0a) & 0x1f)) \
				&& (vid->vga.chline <= (CRT_REG(0x0b) & 0x1f))) {	// cursor start/end
				vid->idx ^= 0xff;
			}
		}
		if (vid->vga.blinken) {
			if ((vid->atrbyte & 0x80) && vid->flash)
				vid->idx ^= 0xff;
			vid->atrbyte &= 0x7f;
		}
		for (i = 0; i < 8; i++) {
			if (vid->idx & 0x80) {
				vid->line[vid->xpos++] = ATR_REG(vid->atrbyte & 0x0f);		// b0..3:foreground
			} else {
				vid->line[vid->xpos++] = ATR_REG((vid->atrbyte >> 4) & 0x0f);	// b4..7:background (b7=0 if blink enabled)
			}
			vid->idx <<= 1;
		}
		vid->vadr++;
	}
	vid->vga.chline++;
	if (vid->vga.chline > CRT_REG(9)) {			// char height register
		vid->vga.line++;
		vid->vga.chline = 0;
//		vid->vadr = vid->tadr;				// vadr to next line
	} else {
		vid->vadr = vid->tadr;				// return to current line address
	}
	if ((vid->ray.y == vid->blank.y) && (vid->inten & 1))
		vid->intrq = 1;
	vga_check_vsync(vid);
}

int vga_seq_adr(Video* vid, int idx) {
	int adr = idx;
	if (!(CRT_REG(0x17) & 0x40)) {
		adr <<= 1;
		if (CRT_REG(0x17) & 0x20) {
			adr |= (idx & 0x8000) >> 15;
		} else {
			adr |= (idx & 0x2000) >> 13;
		}
	}
	if (!(CRT_REG(0x17) & 1)) {		// A13 = V0 (ray.y.bit0)
		adr &= ~0x2000;
		adr |= ((vid->ray.y & 1) << 13);
	}
	if (!(CRT_REG(0x17) & 2)) {		// A14 = V1
		adr &= ~0x4000;
		adr |= ((vid->ray.y & 2) << 13);
	}
	if (!(SEQ_REG(4) & 4)) {		// odd/even
		adr = (adr >> 1) | ((adr & 1) << 16);
	}
	adr += vid->bgadr;
	return adr;
}

// res = HResolution (320 or 640)
// SEQ(1).bit2: if 1, data from 2 planes as 16-bit stream, result - 2 streams by 16 bits; 0 - separate 4 streams by 8 bits (all planes)
// GRF(5).bit5: if 1, 2 bits from stream forms one color/cga; 0 - 1 bit per color/ega
// GRF(17h).bit0: 0:vadr.b13=ray.y.bit0
// GRF(17h).bit1: 0:vadr.b14=ray.y.bit1
// GRF(17h).bit2: 1:inc line counter each 2nd line, 0:each line
// GRF(17h).bit5: if (bit6=0) {0:vadr.b0=vadr.b13, 1:vadr.b0=vadr.b15;}
// GRF(17h).bit6: 0:word mode (vadr<<=1), 1:byte mode
// GRF(17h).bit7: 0:disable Hsync/Vsync
void vga_4bpp(Video* vid, int res) {
	memset(vid->line, CRT_REG(0x11), 0x500);
	int pos = 0;
	res = vid->vga.cpl;
	vid->tadr = vid->fadr;
	vid->vadr = vid->fadr;
	// TODO:fadr = line idx (starts from 0, increment every fetch)
	//	vadr = calculated from fadr due bits below + screen start adr, pick byte(s) to b0..3
	int k,msk;
	int col,b0,b1,b2,b3;
	switch ((SEQ_REG(1) & 4) | (GRF_REG(5) & 0x20)) {
		case 0x00:			// common
			while (pos < res) {
				vid->vadr = vga_seq_adr(vid, vid->fadr);
				b0 = vid->ram[vid->vadr] & 0xff;
				b1 = vid->ram[vid->vadr + 0x10000] & 0xff;
				b2 = vid->ram[vid->vadr + 0x20000] & 0xff;
				b3 = vid->ram[vid->vadr + 0x30000] & 0xff;
				vid->fadr++;
				msk = 0x80;
				do {
					col = 0;
					if (b3 & msk) col |= 8;
					if (b2 & msk) col |= 4;
					if (b1 & msk) col |= 2;
					if (b0 & msk) col |= 1;
					msk >>= 1;
					col &= ATR_REG(0x12);
					vid->line[pos++] = ATR_REG(col & 0x0f);
				} while (msk);
			}
			break;
		case 0x04:			// iterleaved
			while (pos < res) {
				vid->vadr = vga_seq_adr(vid, vid->fadr);
				b0 = ((vid->ram[vid->vadr] & 0xff) << 8) | ((vid->ram[vid->vadr + 0x10000]) & 0xff);
				b1 = ((vid->ram[vid->vadr+0x20000] & 0xff) << 8) | ((vid->ram[vid->vadr + 0x30000]) & 0xff);
				vid->fadr++;
				for (k = 0; k < 8; k++) {
					col = ((b0 & 0x8000) >> 14) | ((b1 & 0x8000) >> 15);
					b0 <<= 1;
					b1 <<= 1;
					col &= ATR_REG(0x12);
					vid->line[pos++] = ATR_REG(col & 0x0f);
				}
			}
			break;
		case 0x20:			// cga (byte = 4 pix, 2bpp)
			while (pos < res) {
				vid->vadr = vga_seq_adr(vid, vid->fadr);
				b0 = (vid->ram[vid->vadr] << 8) | (vid->ram[vid->vadr + 0x10000] & 0xff);
				vid->fadr++;
				for (k = 0; k < 8; k++) {
					col = (b0 >> 14) & 3;
					b0 <<= 2;
					col &= ATR_REG(0x12);
					vid->line[pos++] = ATR_REG(col & 0x0f);
				}
			}
			break;
		case 0x24:			// cga interleaved
			while (pos < res) {
				vid->vadr = vga_seq_adr(vid, vid->fadr);
				b0 = ((vid->ram[vid->vadr] & 0xff) << 8) | (vid->ram[vid->vadr + 0x10000] & 0xff);
				vid->fadr++;
				for (k = 0; k < 8; k++) {
					col = (b0 >> 14) & 3;
					b0 <<= 2;
					col &= ATR_REG(0x12);
					vid->line[pos++] = ATR_REG(col & 0x0f);
				}
			}
			break;
	}
	switch (CRT_REG(0x17) & 3) {
		case 0: if ((vid->ray.y & 3) != 3) vid->fadr = vid->tadr; break;
		case 1:
		case 2: if (!(vid->ray.y & 1)) vid->fadr = vid->tadr; break;
		case 3: break;
	}
	if (CRT_REG(17) & 4) {
		vid->vga.line += (vid->ray.y & 1);
	} else {
		vid->vga.line++;
	}
	// correct colors for cga mode, but not for all games...
	if (GRF_REG(5) & 0x10) {
		for(pos = 0; pos < res; pos++) {
			if (vid->line[pos] & 0x20) {		// cga modes: r,b = g
				vid->line[pos] |= 0x38;
			} else {
				vid->line[pos] &= ~0x38;
			}
		}
	}
}

void vga_glo_ini(Video* vid) {
	vid->vga.cpl = 320;		// dots/line
}

void vga_ghi_ini(Video* vid) {
	vid->vga.cpl = 640;
}

// G320

static unsigned char cga_set_0[8] = {0, 4, 2, 20, 0, 0x3c, 0x3a, 0x3e};
static unsigned char cga_set_1[8] = {0, 3, 5, 7, 0, 0x3b, 0x3d, 0x3f};

unsigned char cga_to_ega(Video* vid, unsigned char c) {
	c &= 3;
	if (!c) {
		c = vid->reg[CGA_3D9] & 7;		// background color (320x200)
		if ((vid->reg[CGA_3D9] & 8) && c)	// bright background
			c |= 8;
		c = ega_def_idx[c];
	} else {
		if ((vid->reg[CGA_3D9] & 0x10))	// bright
			c |= 4;
		c = (vid->reg[CGA_3D9] & 0x20) ? cga_set_1[c] : cga_set_0[c];
	}
	return c;
}

// cga mode
void cga320_2bpp_line(Video* vid) {
	memset(vid->line, CRT_REG(0x11), 0x400);
	int pos = 0;
	int i;
	unsigned char bt;
	vid->vadr = (vid->ray.y >> 1) * CRT_REG(1) * 2;			// crt_reg(1) * 8 / 4 = line width (bytes)
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
	int bt;
	vid->vadr = (vid->ray.y >> 1) * CRT_REG(1) * 2;		// crt_reg(1) * 8 / 4 = line width (bytes)
	vid->vadr += (CRT_REG(0x0c) << 8) | (CRT_REG(0x0d));	// start address
	if (vid->ray.y & 1)
		vid->vadr |= 0x2000;
	for (i = 0; i <= CRT_REG(1) * 2; i++) {
		if (vid->vadr & 1) {
			bt = vid->ram[(vid->vadr >> 1) + 0x10000];
		} else {
			bt = vid->ram[vid->vadr >> 1];
		}
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
