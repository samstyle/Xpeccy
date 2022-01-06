// modes: https://osdev.fandom.com/ru/wiki/VGA_режимы

#include <stdio.h>
#include <string.h>

#include "vga.h"

#define CRT_IDX		vid->vga.crt_idx
#define CRT_REG(_n)	vid->reg[VGA_CRB + (_n)]
#define CRT_CUR_REG	CRT_REG(CRT_IDX)

#define SEQ_IDX		vid->vga.seq_idx
#define SEQ_REG(_n)	vid->reg[VGA_SRB + (_n)]
#define SEQ_CUR_REG	SEQ_REG(SEQ_IDX)

#define GRF_IDX		vid->vga.grf_idx
#define GRF_REG(_n)	vid->reg[VGA_GRB + (_n)]
#define GRF_CUR_REG	GRF_REG(GRF_IDX)

int vga_rd(Video* vid, int port) {
	int res = -1;
	switch (port) {
		case VGA_CRTRD:
			if (CRT_IDX <= 0x18) {
				res = CRT_CUR_REG;
			}
		case VGA_STAT1:
			// b0:videomem is busy
			// b2:light pen off
			// b3:vblank
			res = 4;
			if (vid->vblank)
				res |= 8;
			if (vid->vga.trg) res |= 1;	// just triggering, TODO: make it more real
			vid->vga.trg ^= 1;
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
			vid->vga.cadr = ((CRT_REG(0x0e) << 15) | (CRT_REG(0x0f))) + ((CRT_REG(0x0b) >> 5) & 3);	// cursor address
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
			printf("VGA mode %.2X\n",val & 0x13);
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
		// TODO: check read mode & plane
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
	vid->vga.chline = 0;
}

void vga_t40_line(Video* vid) {
	int i,t;
	memset(vid->line, 0, 0x400);
	vid->xpos = 0;
	vid->vadr = 0;					// b8000
	vid->vadr += vid->vga.line * CRT_REG(1);	// CRT_R1 chars in line
	for (t = 0; t < CRT_REG(1); t++) {
		// char / atr taken right way (checked)
		vid->idx = vid->ram[vid->vadr];			// char (plane 0)
		vid->atrbyte = vid->ram[vid->vadr + 0x10000];	// attr	(plane 1)
		vid->tadr = vid->idx * 16;			// offset of 1st char byte in plane 2 (allways 32 bytes/char in font plane)
		vid->tadr += vid->vga.chline;			// +line in char
		vid->idx = vid->ram[0x20000 + vid->tadr];	// pixels
		if ((vid->vadr == vid->vga.cadr) && !(CRT_REG(0x0a) & 0x20)) {		// cursor position, cursor enabled
			if ((vid->vga.chline >= (CRT_REG(0x0a) & 0x1f)) \
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
	if (vid->vga.chline > CRT_REG(9)) { //CRT_REG(9)) {		// char height register
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

// G320

// G640
