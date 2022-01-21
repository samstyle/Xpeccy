// modes: https://osdev.fandom.com/ru/wiki/VGA_режимы

#include <stdio.h>
#include <string.h>

#include "vga.h"

int vga_rd(Video* vid, int port) {
	int res = -1;
	switch (port) {
		case VGA_CRTRD:
			if (CRT_IDX <= 0x18) {
				res = CRT_CUR_REG;
			}
		case VGA_STAT1:
			// b0:videomem is busy (!(vblank||hblank)
			// b3:vsync
			res = 4;
			if (vid->vblank)	// not vblank, btw
				res |= 8;
			if (!(vid->vbank || vid->hblank))
				res |= 1;
			vid->vga.atrig = 0;
			break;
		case VGA_MODE:
			res = vid->reg[0xff];
			break;
	}
	return res;
}

void vga_wr(Video* vid, int port, int val) {
	switch (port) {
		// crt registers (3d4/3d5)
		case VGA_CRTRN:
			CRT_IDX = val & 0xff;
			break;
		case VGA_CRTRD:
			// printf("VGA CRT: reg[%.2X] = %.2X\n", CRT_IDX, val & 0xff);
			if (CRT_IDX <= VGA_CRC) {
				CRT_CUR_REG = val & 0xff;
			}
			vid->vga.cadr = ((CRT_REG(0x0e) << 8) | (CRT_REG(0x0f))) + ((CRT_REG(0x0b) >> 5) & 3);	// cursor address
			break;
		// sequencer registers (3c4/3c5)
		case VGA_SEQRN:
			SEQ_IDX = val & 0xff;
			break;
		case VGA_SEQRD:
			printf("VGA SEQ: reg[%.2X] = %.2X\n", SEQ_IDX, val & 0xff);
			if (SEQ_IDX <= VGA_SRC) {
				SEQ_CUR_REG = val & 0xff;
			}
			break;
		// vga registers (3ce/3cf)
		case VGA_GRFRN:
			GRF_IDX = val & 0xff;
			break;
		case VGA_GRFRD:
			printf("VGA GRF: reg[%.2X] = %.2X\n", GRF_IDX, val & 0xff);
			if (GRF_IDX <= VGA_GRC) {
				GRF_CUR_REG = val & 0xff;
			}
			break;
		case VGA_MODE:
			vid->reg[0xff] = val & 0xff;
			// b4:gfx 640x200 or 320x200; b2:gfx(1) or text(0); b1:text 80x25
			// printf("VGA mode %.2X\n",val & 0x13);
			switch (val & 0x13) {
				case 0x00: case 0x10:
					vidSetMode(vid, VID_VGA_T40);
					break;		// text 40x25
				case 0x01: case 0x11:
					vidSetMode(vid, VID_VGA_T80);
					break;		// text 80x25
				case 0x02: case 0x03:
					vidSetMode(vid, VID_VGA_G320);
					break;		// gfx 320x200
				case 0x12: case 0x13:
					vidSetMode(vid, VID_VGA_G640);
					break;		// gfx 640x200
			}
			if (val & 0x04) {}		// b2:monochrome
			if (val & 0x08) {}		// b3:video enabled
			if (val & 0x20) {}		// b5:blink enabled
			break;
	}
}

// A0000..AFFFF : gfx mode plane (64K)
// B0000..B7FFF : monochrome mode plane (32K)
// B8000..BFFFF : color text mode plane (32K)

// VGA_R06h,bit2,3:This field specifies the range of host memory addresses that is decoded by the VGA hardware and mapped into display memory accesses
// 00:A0000..BFFFF (128K)
// 01:A0000..AFFFF (64K)
// 10:B0000..B7FFF (32K)
// 11:B8000..BFFFF (32K)

int vga_adr(Video* vid, int exadr) {
	int adr = -1;
	switch(GRF_REG(6) & 0x0c) {
		case 0: adr = exadr; break;
		case 1: if (exadr < MEM_64K) adr = exadr; break;
		case 2: if ((exadr >= 0xb0000) && (exadr < 0xb8000)) adr = exadr; break;
		case 3: if (exadr >= 0xb8000) adr = exadr; break;
	}
	return adr;
}

void vga_mwr(Video* vid, int adr, int val) {
	adr = vga_adr(vid, adr);
	if (adr < 0) return;
	if (adr < 0xb0000) {		// write to plane directly
		// TODO: check write mode / write planes
		printf("vga wr mem %.6X,%.2X\n",adr,val);
	} else if (adr & 1) {		// text mode: attribute (plane 1)
		vid->ram[0x10000 + ((adr >> 1) & 0x3fff)] = val & 0xff;
	} else {			// text mode: char (plane 0)
		vid->ram[((adr >> 1) & 0x3fff)] = val & 0xff;
	}
}

// VGA_R05h.bit3 = read mode
// read mode 0:
// VGA_R04h.bit0,1 = plane to read byte from
// read mode 1:

int vga_mrd(Video* vid, int adr) {
	int res = -1;
	adr = vga_adr(vid, adr);
	if (adr < 0) return -1;
	if (adr < 0xb0000) {		// write to plane directly
		for(res = 0; res < 4; res++)
			vid->vga.latch[res] = vid->ram[(adr & 0xffff) | (res << 16)];
		// TODO: check read mode & plane
		if (GRF_REG(0x05) & 8) {
			// color compare mode
		} else {
			res = vid->vga.latch[GRF_REG(0x04) & 3];
		}
	} else if (adr & 1) {		// text mode: attribute (plane 1)
		res = vid->ram[0x10000 + ((adr >> 1) & 0x3fff)];
	} else {			// text mode: char (plane 0)
		res = vid->ram[((adr >> 1) & 0x3fff)];
	}
	return res;
}

// T40
// Horiz: each 8(9) dots: take char,atr
// char addr = plane0 + (line * 40) + x
// char data addr = plane2 + (charset * 0x2000) + (char * 32) + in_char_line
// Vert: each line in_char_line++
// if in_char_line==CRT_R09h.bits0-4 {in_char_line=0; line++;}
// :: if line==CRT_R12h {HBlank=1}
// :: else if line==CRT_R06h {frame_end}

void vga_t40_frm(Video* vid) {
	vid->vga.line = 0;
	vid->vga.chline = 0;	// CRT_REG(0x08) & 0x1f;
}

void vga_t40_line(Video* vid) {
	int i,t;
	memset(vid->line, CRT_REG(0x11), 0x400);
	vid->xpos = 0;
	vid->vadr = (CRT_REG(0x0c) << 8) | (CRT_REG(0x0d));	// C,D: start address registers
	vid->vadr += vid->vga.line * CRT_REG(1);		// CRT_R1 chars in line
	for (t = 0; t < CRT_REG(1); t++) {
		// char / atr taken right way (checked)
		vid->idx = vid->ram[vid->vadr];			// char (plane 0)
		vid->atrbyte = vid->ram[vid->vadr + 0x10000];	// attr	(plane 1)
		vid->tadr = vid->idx * 32;			// offset of 1st char byte in plane 2 (allways 32 bytes/char in font plane)
		vid->tadr += vid->vga.chline;			// +line in char
		vid->idx = vid->ram[0x20000 + vid->tadr];	// pixels
		if ((vid->vadr == vid->vga.cadr) && !(CRT_REG(0x0a) & 0x20)) {		// cursor position, cursor enabled
			if ((vid->vga.chline > (CRT_REG(0x0a) & 0x1f)) \
				&& (vid->vga.chline <= (CRT_REG(0x0b) & 0x1f))) {	// cursor start/end
				vid->idx ^= 0xff;
			}
		}
		if ((vid->atrbyte & 0x80) && vid->flash)	// blinking (b7 attribute)
			vid->idx ^= 0xff;
		for (i = 0; i < 8; i++) {
			if (vid->idx & 0x80) {
				vid->line[vid->xpos++] = vid->atrbyte & 0x0f;		// b0..3:foreground
			} else {
				vid->line[vid->xpos++] = (vid->atrbyte >> 4) & 0x07;	// b4..6:background
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
}

void vga_t40_dot(Video* vid) {
	vid_dot_full(vid, vid->line[vid->ray.x]);		// 40x8=320
}

// T80

void vga_t80_dot(Video* vid) {
	vid_dot_half(vid, vid->line[vid->ray.x << 1]);		// 80x8=640
	vid_dot_half(vid, vid->line[(vid->ray.x << 1) + 1]);
}

// res = HResolution (320 or 640)
void vga_4bpp(Video* vid, int res) {
	memset(vid->line, CRT_REG(0x11), 0x400);
	int pos = 0;
	int i,k,mask;
	unsigned char col,b0,b1,b2,b3;
	vid->vadr = (vid->ray.y >> 1) * 0x50;
	if (vid->ray.y & 1)
		vid->vadr |= 0x2000;
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
			vid->line[pos++] = col;
			mask >>= 1;
		}
	}
}

// G320

void vga320_2bpp_line(Video* vid) {
	memset(vid->line, CRT_REG(0x11), 0x400);
	int pos = 0;
	int i;
	unsigned char bt;
	vid->vadr = (vid->ray.y >> 1) * 0x50;
	if (vid->ray.y & 1)
		vid->vadr |= 0x2000;
	for (i = 0; i < 320/4; i++) {
		bt = vid->ram[vid->vadr++];	// 4 pix/byte
		vid->line[pos++] = (bt >> 6) & 3;
		vid->line[pos++] = (bt >> 4) & 3;
		vid->line[pos++] = (bt >> 2) & 3;
		vid->line[pos++] = bt & 3;
	}
}

void vga320_4bpp_line(Video* vid) {
	vga_4bpp(vid, 320);
}

// G640

void vga640_1bpp_line(Video* vid) {
	memset(vid->line, CRT_REG(0x11), 0x400);
	int pos = 0;
	int i,k;
	unsigned char bt;
	vid->vadr = (vid->ray.y >> 1) * 0x50;
	if (vid->ray.y & 1)
		vid->vadr |= 0x2000;
	for (i = 0; i < 640/8; i++) {
		bt = vid->ram[vid->vadr++];
		for (k = 0; k < 8; k++) {
			vid->line[pos++] = (bt & 0x80) ? 0x0f : 0x00;
			bt <<= 1;
		}
	}
}

void vga640_4bpp_line(Video* vid) {
	vga_4bpp(vid, 640);
}
