#include "fdc.h"

#include <stdio.h>

// job

// constantly spin floppy & read data (words) to fdc->wdata
void vpspin00(FDC* fdc) {
	if (fdc->flp->pos & 1) {
		flpNext(fdc->flp, fdc->side);
		fdc->wait += BYTEDELAY;
	} else {
		fdc->wdata = flpRd(fdc->flp);
		flpNext(fdc->flp, fdc->side);
		fdc->wait += BYTEDELAY;
		fdc->wdata |= flpRd(fdc->flp) << 8;
		flpNext(fdc->flp, fdc->side);
		fdc->wait += BYTEDELAY;
		fdc->drq = 1;
	}
}

static fdcCall vpSpin[] = {vpspin00, NULL};

// extern

void vp1_reset(FDC* fdc) {

}

// status
// 0	trk0
// 1	ready
// 2	write protect
// 7	drq
// 14	??? crc
// 15	index
unsigned short vp1_rd(FDC* fdc, int port) {
	unsigned short res = 0xffff;
	if (port & 1) {
		res = fdc->wdata;
		fdc->drq = 0;
	} else {
		res = 0x0000;
		if (fdc->flp->trk == 0) res |= 1;
		if (fdc->idle) res |= 2;
		if (fdc->flp->protect) res |= 4;
		if (fdc->drq) res |= 0x80;
		if (fdc->flp->index) res |= 0x8000;
	}
	return res;
}

// 0..3	select drive 0..3
// 4	motor
// 5	0:upper head, 1:bottom head
// 6	step dir
// 7	step
// 8	??? start reading
// 9	??? write marker
// 10	???
void vp1_wr(FDC* fdc, int port, unsigned short val) {
	if (port & 1) {
		// data
	} else {
		printf("wr %.4X\n",val);
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
		fdc->side = (val & 0x20) ? 1 : 0;
		if (val & 0x80) flpStep(fdc->flp, (val & 0x40) ? FLP_FORWARD : FLP_BACK);
		if (val & 0x100) {
			// b8
		}
		if (val & 0x200) {
			// b9
		}
	}
}
