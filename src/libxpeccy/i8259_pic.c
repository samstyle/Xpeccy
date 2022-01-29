// https://wiki.osdev.org/PIC
// http://www.brokenthorn.com/Resources/OSDevPic.html

#include <stdio.h>

#include "i8259_pic.h"

void pic_reset(PIC* pic) {
	pic->imr = 0xff;
	pic->isr = 0;
	pic->irr = 0;
	pic->oint = 0;
}

int pic_int(PIC* pic, int num) {
	if (pic->isr != 0) return 0;		//  don't process many ints at once
	int mask = (1 << (num & 7));
	mask &= ~pic->imr;		// 0=masked
	mask &= ~pic->isr;		// 0=this int is not ended
	if (!mask) return 0;
	pic->num = num & 7;
	pic->mask = mask;		// fix mask
//	pic->irr |= mask;		// input accepted
	pic->isr |= mask;		// set isr bit (remains 1 until eoi)
	pic->oint = 1;			// send INT
	if (pic->master && (pic->icw3 & mask)) {		// if this input is slave pic, get vector from there
		pic->vec = -1;
	} else {					// else calc vector
		pic->vec = (pic->icw2 & 0xf8) | (num & 7);
	}
	return 1;
}

void pic_eoi(PIC* pic, int num) {
	pic->isr &= ~(1 << (num & 7));
	pic->irr &= ~(1 << (num & 7));
}

// return vector for int with hightst priority with bits irr=1 imr=0 & isr=0
int pic_ack(PIC* pic) {
	if (pic->icw4 & 2) {		// automatic eoi
		pic_eoi(pic, pic->num);
	}
	pic->oint = 0;
	return pic->vec;
}

// rd/wr

void pic_wr(PIC* pic, int adr, int data) {
	if (adr & 1) {		// wr data
		switch(pic->mode) {
			case PIC_OCWX:
				pic->imr = data & 0xff;
				break;
			case PIC_ICW2:
				pic->icw2 = data & 0xff;
				if (data & 2) {
					pic->icw3 = 0;
					pic->mode = (pic->icw1 & 1) ? PIC_ICW4 : PIC_OCWX;
				} else {
					pic->mode = PIC_ICW3;
				}
				break;
			case PIC_ICW3:
				pic->icw3 = data & 0xff;
				pic->mode = (pic->icw1 & 1) ? PIC_ICW4 : PIC_OCWX;
				break;
			case PIC_ICW4:
				pic->icw4 = data & 0xff;
				pic->mode = PIC_OCWX;
				break;
		}
	} else {		// wr command
		if (data & 0x10) {		// init
			pic->icw1 = data & 0xff;
			pic->mode = PIC_ICW2;
		} else if (data & 0x08) {	// ocw3 (example: 0b = 00001011)
			pic->ocw3 = data & 0xff;
			if (data & 0x40) {
				pic->smm = (data & 0x20) ? 1 : 0;
			}
			if (data & 2) {				// read isr/irr on next rd pulse
				pic->srd = 1;
				pic->sdt = (data & 1) ? pic->isr : pic->irr;
			}
		} else {			// ocw2 (example: 20)
			pic->ocw2 = data & 0xff;
			if (data & 0x20) {			// eoi
				if (data & 0x40) {		// specific eoi - reset specific isr bit
					pic_eoi(pic, data & 7);
				} else {			// non-specific eoi (TODO: reset isr bit with highest priority)
					pic_eoi(pic, pic->num & 7);	// eoi
				}
			}
		}
	}
}

int pic_rd(PIC* pic, int adr) {
	int res = -1;
	if (adr & 1) {	// rd data
		if (pic->mode == PIC_OCWX) {
			if (pic->srd) {
				pic->srd = 0;
				res = pic->sdt;
			} else {
				res = pic->imr;
			}
		}
	} else {	// rd command
	}
	return res;
}
