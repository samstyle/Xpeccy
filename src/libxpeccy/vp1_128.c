#include "fdc.h"

#include <stdio.h>

enum {
	VP1_SEEK = 0,
	VP1_RD,
	VP1_WR
};

// seek
void vpseek(FDC* fdc) {
	fdc->data = flpRd(fdc->flp, fdc->side);
	if ((fdc->flp->field == 0) && (fdc->data == 0xa1)) {	// catch syncro byte A1
		fdc->tdata = fdc->data;				// copy to temp register
		fdc->cnt = 0;					// next byte will complete word
		fdc->drq = 0;					// word is still incomplete
		fdc->crchi = 0;					// reset 'crc is valid' flag
		fdc->crc = 0xffff;				// init crc
		add_crc_16(fdc, 0xa1);				// add 1st byte to crc
		fdc->state = VP1_RD;				// move to reading
		fdc->pos = 1;
	}
	flpNext(fdc->flp, fdc->side);
	fdc->wait += BYTEDELAY;
}

void vpread(FDC* fdc) {
	fdc->tdata <<= 8;		// shift temp register
	fdc->data = flpRd(fdc->flp, fdc->side);	// read next byte from floppy
	fdc->tdata |= fdc->data;	// add it to temp register
	add_crc_16(fdc, fdc->data);	// add it to crc. crc will be 0 after reading crc (hi-low)
	if (fdc->crc == 0)		// if crc is valid now, set flag
		fdc->crchi = 1;
	fdc->cnt++;			// inc byte counter
	if (fdc->cnt & 1) {		// each 2nd byte (starting from 0) - move word from temp register to data register and set drq
		fdc->wdata = fdc->tdata;
		fdc->drq = 1;
	}
	flpNext(fdc->flp, fdc->side);	// move to next byte
	fdc->wait += BYTEDELAY;		// set 32mks delay
}

void vpwrite(FDC* fdc) {
		flpNext(fdc->flp, fdc->side);
		fdc->wait += BYTEDELAY;
}

static fdcCall vpSpin[] = {vpseek, vpread, vpwrite, NULL};

// extern

void vp1_reset(FDC* fdc) {
	fdc->state = VP1_SEEK;
	fdc->pos = 0;
	fdc->mr = 0;
	fdc->drq = 0;
}

// status
// 0	trk0
// 1	ready
// 2	write protect
// 7	drq
// 14	??? crc
// 15	index
unsigned short vp1_rd(FDC* fdc, int port) {
	unsigned short res = 0;
	if (port & 1) {
		res = fdc->drq ? fdc->wdata : 0x0000;
		fdc->drq = 0;
	} else {
		if (fdc->flp->trk == 0) res |= 1;
		res |= 2;
		if (fdc->flp->protect) res |= 4;
		if (fdc->drq) res |= 0x80;
		if (fdc->crchi) res |= 0x4000;
		if (fdc->flp->index && fdc->flp->insert && fdc->flp->motor)
			res |= 0x8000;
	}
	return res;
}

// 0..3	select drive 0..3
// 4	motor
// 5	0:upper head, 1:bottom head
// 6	step dir
// 7	step
// 8	start reading (1 -> 0: set state to seek)
// 9	??? write marker
// 10	???
void vp1_wr(FDC* fdc, int port, unsigned short val) {
	if (port & 1) {
		// data
	} else {
		// printf("wr %.4X\n",val);
		fdc->flp->motor = 0;
		if (val & 1) {
			fdc->flp = fdc->flop[0];
		} else if (val & 2) {
			fdc->flp = fdc->flop[1];
		} else if (val & 4) {
			fdc->flp = fdc->flop[2];
		} else if (val & 8) {
			fdc->flp = fdc->flop[3];
		}
		fdc->flp->motor = (val & 0x10) ? 1 : 0;
		if (fdc->flp->motor) {
			fdc->plan = vpSpin;
			fdc->pos = 0;
			fdc->wait = BYTEDELAY;
		} else {
			fdc->plan = NULL;
		}
		fdc->side = (val & 0x20) ? 0 : 1;
		if (val & 0x80)
			flpStep(fdc->flp, (val & 0x40) ? FLP_FORWARD : FLP_BACK);
		if (val & 0x100) {
			fdc->mr = 1;
		} else {
			if (fdc->mr) {
				fdc->state = VP1_SEEK;
				fdc->pos = 0;
				fdc->drq = 0;
			}
			fdc->mr = 0;
		}
		if (val & 0x200) {
			// b9
		}
	}
}
