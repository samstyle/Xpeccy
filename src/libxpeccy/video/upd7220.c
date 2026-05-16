#include <string.h>
#include <stdio.h>

#include "upd7220.h"

// WHATIF: upd7220 writes up to 16 params into fifo and reading it internally 1 byte in *some time*.
//	'fifo full' and 'fifo empty' status bits will indicate correct state in this case (maybe)

upd7220* upd7220_create() {
	upd7220* upd = (upd7220*)malloc(sizeof(upd7220));
	if (upd) {
		memset(upd, 0x00, sizeof(upd7220));
	}
	return upd;
}

void upd7220_destroy(upd7220* upd) {
	if (upd) {
		free(upd);
	}
}

// a0: 0 - rd:status, wr:param fifo
// a0: 1 - rd:fifo rd, wr:command fifo

// status:
// b7: light pen detect
// b6: hblank
// b5: vsync
// b4: dma execute
// b3: drawing in process
// b2: fifo empty
// b1: fifo full
// b0: data ready

// execute command. reg[0] = com, reg[1..15] = params
// commands,parameters
// 00000000	0	reset
// 0000111x	8	sync
// 0110111m	0	select master/slave chip
// 01101011	0	start
// 00001101	0	start
// 00001100	0	stop
// 01000110	1	zoom
// 0111rrrr	16	scroll
// 01001011	4	csrform
// 01000111	1	pitch
// 11000000	3?	lpen
// 01001100	11	vectw
// 01101100	0	vecte
// 01111rrr	8	textw
// 01101000	0	texte
// 01001001	2/3	csrw
// 11100000	5	csrr
// 01001010	2	mask
// 001ww0mm	1	write
// 101ww0mm	0	read
// 001ww1mm	0	dmaw
// 101ww1mm	0	dmar

void upd7220_reset_buf(x7220buf* buf) {
	buf->pos = 0;
	buf->cnt = 0;
}

void upd7220_reset(Video* vid, upd7220* upd) {
	upd7220_reset_buf(&upd->inbuf);
	upd7220_reset_buf(&upd->outbuf);
	upd->off = 1;
	vid->regCharBegin = 0;
	vid->regCharEnd = 15;
	vid->regCharHeight = 16;
}

void upd7220_start(Video* vid, upd7220* upd) {
	upd->off = 0;
}

void upd7220_stop(Video* vid, upd7220* upd) {
	upd->off = 1;
}

void upd7220_master(Video* vid, upd7220* upd) {
	upd->master = !!(upd->inbuf.data[0] & 1);
}

// set CRT geometry
// b0,com	DE		display enabled
// b5..0,p1	CHR.F.I.G.D.S	CHR.G = 00:mix txt/grf, 01:grf, 10:txt, 11:undef, F:!flash, I.S = 00:nointerlace, 01:undef?, 10:interlace, 11:interlace+repeat field, D:refresh enabled
// p2		C/R		characters/row - 2 (80:4Eh, 40:26h)
// b4..0,p3	HS		HSync width - 1 (symbols)
// b7..5,p3	VSL		VSync width - 1 (symbol rows)
// b1..0,p4	VSH		^^
// b7..2,p4	HFP		Horizontal front porch width
// b5..0,p5	HBP		Horizontal back porch width
// b5..0,p6	VFP		Vertical front porch width
// p7		LFL		lines/frame (0=1024, normal 400)
// b1..0,p8	LFH		^^
// b7..2,p8	VBP		Vertical back porch width
void upd7220_sync(Video* vid, upd7220* upd) {

}

// com = 4B
// p1:4-0	lines per char row (char height)
// p1:7		1:display cursor
// p2:4-0	cursor top line
// p2:5		0:blinking, 1:steady
// p2:7-6	BRL (cursor blinking rate)
// p3:2-0	BRH
// p3:7-3	cursor bottom line
void upd7220_cchar(Video* vid, upd7220* upd) {
	vid->regCharHeight = upd->inbuf.data[1] & 0x1f;
	vid->flgShowCrs = !!(upd->inbuf.data[1] & 0x80);
	vid->regCrsStart = upd->inbuf.data[2] & 0x1f;
	vid->flgCrsBlink = !!(upd->inbuf.data[2] & 0x20);
	// TODO: blink rate
	vid->regCrsEnd = (upd->inbuf.data[3] >> 3) & 0x1f;

}

// com = 47	pitch
// p1		words/line
void upd7220_pitch(Video* vid, upd7220* upd) {
}

// com = 46	zoom
// p1:7-4	display zoom factor
// p1:3-0	(zoom factor for graphics character writing and area filling)
void upd7220_zoom(Video* vid, upd7220* upd) {
}

// com = 7x	pram
// com:3-0	starting address in param.ram
// p1...	bytes writing to param.ram (capacity is 16 bytes, address incremented, breaks at next command)
// param.ram: 4 blocks x 4 bytes (text mode), 2 blocks x 4 bytes + 8bytes/4words (mix mode)
// +0,1		start address (text:13bits, mixed:18bits (b0,1 of +2 is msb)
// +2,3		b7-4(+2),b5-0(+3) - length of area (line count)
// +3.bit6	in mixed mode 0=grf 1=txt)
// +3.bit7	WD - wide display, 2words/cycle (?)
void upd7220_pram(Video* vid, upd7220* upd) {
	upd->par[upd->inbuf.data[0] & 0x0f] = upd->inbuf.data[1];
	upd->inbuf.data[0]++;	// move to next address
	upd->inbuf.data[0] &= 0x0f;
	upd->inbuf.data[0] |= 0x70;
	upd->inbuf.pos = 1;	// waiting next parameter (reset by command)
	upd->inbuf.cnt = 1;
}

// com = 49		set cursor EAD
// p1,p2,p3:bit1,0	address (p3 is graphic mode only)
// p3:7-4		dot address in word
// NOTE: EAD is word number, real address is (EAD*2)
void upd7220_curs(Video* vid, upd7220* upd) {
	upd->ead = (upd->inbuf.data[1] | (upd->inbuf.data[2] << 8)) << 1; // | ((upd->inbuf.data[3] & 3) << 16);
	// upd->dpos = (upd->inbuf.data[3] >> 4) & 15;
}

// 001.type:2.0.mod:2		write
// 101.type:2.0.mod:2		read
// address = EAD from com49
// type: 00-word, 01-low byte (00xx),11-high byte (xx00),10-invalid
// mod: 00-replace,01-compliment,10-reset,11-set (write only)

void upd7220_rdat(Video* vid, upd7220* upd) {
//	int type = (upd->inbuf.data[0] >> 3) & 3;
}

void upd7220_wsetcnt(upd7220* upd, int type) {
	switch(type) {
		case 0: upd->inbuf.cnt = 2;
			break;
		case 1:
		case 3: upd->inbuf.cnt = 1;
			break;
		case 2:	// wut to do here?
			break;
	}
}

void upd7220_wdat(Video* vid, upd7220* upd) {
	int type = (upd->inbuf.data[0] >> 3) & 3;
//	int mode = upd->inbuf.data[0] & 3;
	if (upd->inbuf.pos == 1) {	// no params transfered yet
		upd7220_wsetcnt(upd, type);
	} else {
		switch(type) {
			case 0:
				vid->ram[upd->ead & 0x3fff] = upd->inbuf.data[1];
				vid->ram[(upd->ead + 1) & 0x3fff] = upd->inbuf.data[2];
				break;
			case 1:
				vid->ram[upd->ead & 0x3fff] = upd->inbuf.data[1];
				break;
			case 3:
				vid->ram[(upd->ead + 1) & 0x3fff] = upd->inbuf.data[1];
				break;
		}
		upd->ead += 2;
		upd->inbuf.pos = 1;
		upd7220_wsetcnt(upd, type);
	}
}

struct upd7220com {
	int mask;
	int com;
	int pcnt;
	void(*exec)(Video*, upd7220*);
} gdc_com_tab[] = {
	{0xff, 0x00, 0, upd7220_reset},		// reset
	{0xfe, 0x0e, 8, upd7220_sync},		// sync (set geometry)
	{0xfe, 0x6e, 0, upd7220_master},	// (b0 = 1:master/0:slave) - sync mode
	{0xff, 0x6b, 0, upd7220_start},		// start - enable output
	{0xff, 0x0d, 0, upd7220_start},		// start - (same)
	{0xff, 0x0c, 0, upd7220_stop},		// stop - disable output
	{0xff, 0x46, 1, upd7220_zoom},		// zoom
	{0xf0, 0x70, 1, upd7220_pram},		// scroll (pram?)
	{0xff, 0x4b, 4, upd7220_cchar},	// csrform - cursor params
	{0xff, 0x47, 1, upd7220_pitch},	// pitch
	{0xff, 0xc0, 3, NULL},	// lpen
	{0xff, 0x4c, 11, NULL},	// vectw	drawing
	{0xff, 0x6c, 0, NULL},	// vecte
	{0xf8, 0x78, 8, NULL},	// textw
	{0xff, 0x68, 0, NULL},	// texte
	{0xff, 0x49, 2, upd7220_curs},	// csrw		set cursor address
	{0xff, 0xe0, 5, NULL},	// csrr			read cursor address
	{0xff, 0x4a, 1, NULL},	// mask
	{0xe4, 0x40, 0, upd7220_wdat},	// write
	{0xe4, 0xa0, 0, upd7220_rdat},	// read
	{0xe4, 0x44, 0, NULL},	// dmaw
	{0xe4, 0xa4, 0, NULL},	// dmar
	{0,0,0, NULL}		// (eot)
};

void upd7220_exec(Video* vid, upd7220* upd) {
	int adr = 0;
	upd7220_reset_buf(&upd->inbuf);
	while (gdc_com_tab[adr].mask && ((upd->inbuf.data[0] ^ gdc_com_tab[adr].com) & gdc_com_tab[adr].mask)) {
		adr++;
	}
	if (gdc_com_tab[adr].exec != NULL) {
		gdc_com_tab[adr].exec(vid, upd);	// execute command
	} else if (gdc_com_tab[adr].com != 0) {
		printf("upd7220 command %.2X not implemented\n", upd->inbuf.data[0]);
		vid_irq(vid, IRQ_BRK);
	}
}

int upd7220_rd(Video* vid, upd7220* upd, int adr) {
	int res = 0;
	if (adr & 1) {
		// out data rd
		if (upd->outbuf.cnt > 0) {
			res = upd->outbuf.data[upd->outbuf.pos];
			upd->outbuf.pos++;
			upd->outbuf.cnt--;
		} else {
			res = 0xff;
		}
	} else {
		// status rd
		if (vid->hblank) res |= 0x40;
		if (vid->vblank) res |= 0x20;
		if (upd->inbuf.queue > 16) {
			res |= 0x02;
		} else if (upd->inbuf.queue == 0) {
			res |= 0x04;
		}
		if (upd->outbuf.cnt > 0) res |= 0x01;
	}
	return res;
}

// TODO: you can write less/more params for command, but every byte must be counted
// example: csrw command may have 2 or 3 params
void upd7220_wr(Video* vid, upd7220* upd, int adr, int val) {
	if (adr & 1) {
		// fifo wr:command
		upd->inbuf.data[0] = val & 0xff;
		upd->inbuf.cnt = 0;
		upd->inbuf.pos = 1;
		upd->inbuf.queue = 1;
		adr = 0;
		while (gdc_com_tab[adr].mask && ((val ^ gdc_com_tab[adr].com) & gdc_com_tab[adr].mask)) {
			adr++;
		}
		if (gdc_com_tab[adr].mask) {	// valid
			upd->inbuf.cnt = gdc_com_tab[adr].pcnt;
			printf("7220: com %.2X waiting %i params\n", upd->inbuf.data[0], upd->inbuf.cnt);
			if (upd->inbuf.cnt == 0) {	// no params
				upd7220_exec(vid, upd);
			}
		} else {			// not valid
			// ...
		}
	} else {
		// fifo wr:param
		if (upd->inbuf.cnt > 0) {
			printf("par %i = %.2X\n", upd->inbuf.pos, val);
			upd->inbuf.data[upd->inbuf.pos] = val & 0xff;
			upd->inbuf.cnt--;
			upd->inbuf.pos++;
			upd->inbuf.queue++;
			if (upd->inbuf.cnt == 0) {
				upd7220_exec(vid, upd);
			}
		}
	}
}

// drawing
// text
// A0000 - line 0, 80 chars x 2 bytes (low,hi)
// A00A0 - line 1
// A0F00 - line 24
// A2000 - attributes, same 25 lines x 80 chars x 2 bytes
// A2FA0 - end of text data
// atr:	b0
//	b1: blink
//	b2: reverse
//	b3: underline
//	b4: bv/gl ?
//	b5,6,7 = b,r,g
// text mode: 80x25, 640x400
// char: 16x16 (half-width pixels)

// grf
// A8000 - plane 0 line 0, 640dots/8bits = 80 bytes, msb first
// A8050 - plane 0 line 1 ...
// AFCB0 - plane 0 line 399
// B0000 - plane 1
// B8000 - plane 2
// E0000 - plane 3
// MSBs from each plane = color [index?]
// 16col - all planes
// 8col - 0,1,2. plane 3 unused
// mono - each plane is separate screen

void upd7220_dot(Video* vid) {
	int pos = vid->ray.xs << 1;
	int tc = vid->line[pos];
	int gc = vid->linb[pos];
	vid_dot_half(vid, tc ? tc : gc);
	pos++;
	tc = vid->line[pos];
	gc = vid->linb[pos];
	vid_dot_half(vid, tc ? tc : gc);
}

// higres = DIPSW1-1 (1:highres, 0:normal). read - pc98xx port33.bit3
// lowres = double dots, double lines
// font: 00xx - ank (8x8)
void upd7220_line(Video* vid) {
	if (vid->txt7220->inbuf.queue > 0) vid->txt7220->inbuf.queue--;
	if (vid->grf7220->inbuf.queue > 0) vid->grf7220->inbuf.queue--;
// form text line (vid->line)
	int line = vid->ray.ys / vid->regCharHeight;
	int clin = vid->ray.ys % vid->regCharHeight;
	int c = 7;
	int adr = line * 160;		// adr of line start (160 bytes / line)
	int pos = 0;
//	int clin = vid->ray.ys & 0x07;			// line inside char
	int i, j;
	int chr;
	int atr;
	if (vid->txt7220->off || vid->nogfx) {
		memset(vid->line, 0x00, 0x500);
	} else {
		// vid->vga.cpl = 40/80 - chars in line
		for (i = 0; i < 80; i++) {
			chr = (vid->ram[adr] & 0xff) | (vid->ram[adr + 1] << 8);
			atr = (vid->ram[adr | 0x2000] & 0xff) | (vid->ram[adr | 0x2001] << 8);	// char attribute
			c = 7; // (atr >> 5) & 7;				// color
			chr = (chr << 3) + clin;			// for 8x8 chars
			chr = (vid_fnt_rd(vid, chr) << 8) | (vid_fnt_rd(vid, chr | 1) & 0xff);		// 16 bit of pixels, do something with 283K kanjirom
			if (atr & 4) chr ^= 0xffff;			// invert
			if ((atr & 2) && vid->flash) chr ^= 0xffff;	// blink TODO:blink rate

			if (/*vid->flgShowCrs && */(adr == vid->txt7220->ead)) {
				if (((vid->ray.ys & 7) == 7)) { // >= vid->regCrsStart) && ((vid->ray.ys & 7) < vid->regCrsEnd)) {
					chr = 0xffff;
				}
			}

			// TODO: atr.invert, atr.blink, atr.stroked
			for (j = 0; j < 8; j++) {
				// TODO: fg/bg color for text mode
				// TODO: lowres mode
				vid->line[pos++] = (chr & 0x8000) ? c : 0;
				vid->line[pos++] = (chr & 0x8000) ? c : 0;
				chr <<= 1;
			}
			adr += 2;								// next char
		}
	}
// form graphic line (vid->linb)
	if (vid->grf7220->off || vid->nogfx) {
		memset(vid->linb, 0x00, 0x500);
	} else {
		int pln0, pln1, pln2, pln3;	// data from planes
		pos = 0;
		adr = vid->ray.ys * 80;
		for (i = 0; i < 80; i++) {
			pln0 = vid->ram[0x8000 + adr];
			pln1 = vid->ram[0x10000 + adr];
			pln2 = vid->ram[0x18000 + adr];
			pln3 = vid->ram[0x20000 + adr];
			chr = 0x80;
			for (j = 0; j < 8; j++) {
				c = 0;
				if (pln0 & chr) c |= 1;
				if (pln1 & chr) c |= 2;
				if (pln2 & chr) c |= 4;
				if (pln3 & chr) c |= 8;
				vid->linb[pos++] = c;
				vid->linb[pos++] = c;
				chr >>= 1;
			}
			adr++;
		}
	}
}

void upd7220_frame(Video* vid) {

}
