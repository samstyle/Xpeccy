#include "video.h"

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

// reg[00..0F] = fifo for commands, com + params
// reg[10..1F] = output data buffer
#define regCom	reg[0]
#define fifoPos	reg[32]	// +1 each internal rd
#define fifoSiz reg[33]	// +1 each cpu writing, -1 each internal rd. queue size
#define parCnt	reg[34]	// parameters command want to get
#define dOutPos reg[35]	// =0x10 when form output packet, +1 each cpu rd
#define dOutSiz	reg[36]	// output data packet size (0 = no data), -1 each cpu rd

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

struct upd7220com {
	int mask;
	int com;
	int pcnt;
	void(*exec)(Video*);
} gdc_com_tab[] = {
	{0xff, 0x00, 0, NULL},	// reset
	{0xfe, 0x0e, 8, NULL},	// sync
	{0xfe, 0x6e, 0, NULL},	// select chip
	{0xff, 0x6b, 0, NULL},	// start - enable output
	{0xff, 0x0d, 0, NULL},	// start - (same)
	{0xff, 0x0c, 0, NULL},	// stop - disable output
	{0xff, 0x46, 1, NULL},	// zoom
	{0xf0, 0x70, 16, NULL},	// scroll
	{0xff, 0x4b, 4, NULL},	// csrform
	{0xff, 0x47, 1, NULL},	// pitch
	{0xff, 0xc0, 3, NULL},	// lpen
	{0xff, 0x4c, 11, NULL},	// vectw
	{0xff, 0x6c, 0, NULL},	// vecte
	{0xf8, 0x78, 8, NULL},	// textw
	{0xff, 0x68, 0, NULL},	// texte
	{0xff, 0x49, 2, NULL},	// csrw
	{0xff, 0xe0, 5, NULL},	// csrr
	{0xff, 0x4a, 1, NULL},	// mask
	{0xe4, 0x40, 1, NULL},	// write
	{0xe4, 0xa0, 0, NULL},	// read
	{0xe4, 0x44, 0, NULL},	// dmaw
	{0xe4, 0xa4, 0, NULL},	// dmar
	{0,0,0, NULL}		// (eot)
};

void upd7220_exec(Video* vid) {
	int adr = 0;
	while (gdc_com_tab[adr].mask && ((vid->regCom ^ gdc_com_tab[adr].com) & gdc_com_tab[adr].mask)) {
		adr++;
	}
	if (gdc_com_tab[adr].exec != NULL) {
		gdc_com_tab[adr].exec(vid);
	}
}

int upd7220_rd(Video* vid, int adr) {
	int res = 0;
	if (adr & 1) {
		// out data rd
		if (vid->dOutSiz > 0) {
			res = vid->reg[vid->dOutPos];
			vid->dOutPos++;
			vid->dOutSiz--;
		} else {
			res = 0xff;
		}
	} else {
		// status rd
		if (vid->hblank) res |= 0x40;
		if (vid->vblank) res |= 0x20;
		if (vid->fifoSiz > 15) {
			res |= 0x02;
		} else if (vid->fifoSiz == 0) {
			res |= 0x04;
		}
		if (vid->dOutSiz > 0) res |= 0x01;
	}
	return res;
}

void upd7220_wr(Video* vid, int adr, int val) {
	if (adr & 1) {
		// fifo wr:command
		vid->regCom = val & 0xff;
		vid->fifoPos = 0;
		vid->fifoSiz = 1;
		adr = 0;
		while (gdc_com_tab[adr].mask && ((val ^ gdc_com_tab[adr].com) & gdc_com_tab[adr].mask)) {
			adr++;
		}
		if (gdc_com_tab[adr].mask) {	// valid
			vid->parCnt = gdc_com_tab[adr].pcnt;
			if (vid->parCnt == 0) {	// no params
				upd7220_exec(vid);
			}
		} else {			// not valid
			// ...
		}

	} else {
		// fifo wr:param
		if (vid->parCnt > 0) {
			vid->reg[vid->fifoSiz & 15] = val & 15;
			vid->parCnt--;
			vid->fifoSiz++;
			if (vid->parCnt == 0) {
				upd7220_exec(vid);
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

void upd7220_line(Video* vid) {
// form text line (vid->line)
	int c = 7;
	int adr = (vid->ray.ys & 0xf0) * 10;		// adr of line start
	int pos = 0;
	int clin = vid->ray.ys & 0x0f;			// line inside char
	int i, j;
	int chr;
	int atr;
	for (i = 0; i < 80; i++) {
		chr = (vid->ram[adr] & 0xff) | (vid->ram[adr + 1] << 8);		// char code (TODO: lowres mode, 40 ch/line)
		atr = (vid->ram[adr | 0x2000] & 0xff) | (vid->ram[adr | 0x2001] << 8);	// char attribute
		chr = (chr << 5) | (clin << 1);						// char data address (current line)
		//chr = (vid->font[chr] << 8) | (vid->font[chr | 1] & 0xff);		// 16 bit of pixels, do something with 283K kanjirom
		// TODO: atr.invert, atr.blink, atr.stroked
		for (j = 0; j < 16; j++) {
			// TODO: fg/bg color for text mode
			// TODO: lowres mode
			vid->line[pos++] = (chr & 0x8000) ? c : 0;
		}
		adr += 2;								// next char
	}
// form graphic line (vid->linb)
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

void upd7220_frame(Video* vid) {

}
