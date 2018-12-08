#include "fdc.h"

#include <stdio.h>

// NOTE: cdb4 = crc for A1,A1,A1
// init value of crc must be FFFF, accumulation starting from 1st A1

static int pauses[4] = {6000,12000,20000,30000};	// pause in ns for 1st type commands

// 1818vg93

// add byte to CRC
void add_crc_16(FDC* fdc, unsigned char val) {
	unsigned int tkk = fdc->crc;
	int i;
	tkk ^= val << 8;
	for (i = 8; i; i--) {
		if ((tkk *= 2) & 0x10000) tkk ^= 0x1021;
	}
	fdc->crc = tkk & 0xffff;
}

// read FCRC from disk (hi-low)
void rdFCRC(FDC* fdc) {
	fdc->fcrc = (flpRd(fdc->flp) << 8);
	flpNext(fdc->flp, fdc->side);
	fdc->fcrc |= flpRd(fdc->flp);
	flpNext(fdc->flp, fdc->side);
}

// save CRC to disk (hi-low)
void wrCRC(FDC* fdc) {
	flpWr(fdc->flp, (fdc->crc >> 8) & 0xff);
	flpNext(fdc->flp, fdc->side);
	flpWr(fdc->flp, fdc->crc & 0xff);
	flpNext(fdc->flp, fdc->side);
}

// wait ADR
// return: 0 : not ADR, 1 : ADR, 2 : IDX
int waitADR(FDC* fdc) {
	fdc->wait += turbo ? 1 : BYTEDELAY;
	if (flpNext(fdc->flp, fdc->side)) return 2;	// 2:IDX
	if (fdc->flp->field != 1) return 0;		// 0:not ADR
	return 1;
}

// wait ADR mark & read to BUF[4] and FCRC
// return: 0 : not ADR, 1 : ADR readed in buf + fcrc, 2 : IDX
int seekADR(FDC* fdc) {
	int res = waitADR(fdc);
	if (res != 1) return res;
	fdc->crc = 0xcdb4;
	add_crc_16(fdc,0xfe);
	for (int i = 0; i < 4; i++) {
		fdc->buf[i] = flpRd(fdc->flp);
		add_crc_16(fdc, fdc->buf[i]);
		flpNext(fdc->flp, fdc->side);
	}
	rdFCRC(fdc);
	fdc->wait += turbo ? 1 : (6 * BYTEDELAY);	// delay : 6 byte reading
	return 1;					// 1:ADR
}

// send byte FDC->Floppy (wr commands)
int vgSendByte(FDC* fdc) {
	int res = 0;
	if (turbo) {
		if (fdc->drq) {
			if (fdc->tns > BYTEDELAY) {
				fdc->state |= 0x04;
				flpNext(fdc->flp, fdc->side);
				fdc->tns = 0;
				res = 1;
			}
		} else {
			add_crc_16(fdc, fdc->data);
			flpWr(fdc->flp, fdc->data);
			flpNext(fdc->flp, fdc->side);
			fdc->drq = 1;
			fdc->tns = 1;
			res = 1;
		}
		fdc->wait = 1;
	} else {
		if (fdc->drq) {			// time to write byte, but isn't get it from CPU
			fdc->state |= 0x04;
		}
		add_crc_16(fdc, fdc->data);
		flpWr(fdc->flp, fdc->data);
		flpNext(fdc->flp, fdc->side);
		fdc->drq = 1;
		fdc->wait += BYTEDELAY;
		fdc->tns = 0;
		res = 1;
	}
	return res;
}

// get byte Floppy->FDC (rd commands)
int vgGetByte(FDC* fdc) {
	int res = 0;
	if (turbo) {
		if (fdc->drq) {
			if (fdc->tns > BYTEDELAY) {
				fdc->state |= 0x04;
				flpNext(fdc->flp, fdc->side);
				fdc->tns = 0;
				res = 1;
			}
		} else {
			fdc->data = flpRd(fdc->flp);
			flpNext(fdc->flp, fdc->side);
			fdc->drq = 1;
			fdc->tns = 0;
			res = 1;
		}
		fdc->wait = 1;				// check every time (no delay)
	} else {
		if (fdc->drq) {
			fdc->state |= 0x04;		// data lost - time to read next byte, but previous is not transfered
		}
		fdc->data = flpRd(fdc->flp);
		flpNext(fdc->flp, fdc->side);
		fdc->drq = 1;
		fdc->wait += BYTEDELAY;
		fdc->tns = 0;
		res = 1;
	}
	return res;
}

// idle : wait 15 IDX pulses & stop motor

void vgstp00(FDC* fdc) {
//	printf("stop\n");
	fdc->irq = 1;
	fdc->idle = 1;
	if (!fdc->flp->motor || !fdc->flp->insert) {
		fdc->plan = NULL;
	} else {
		fdc->cnt = 15;
		fdc->pos++;
	}
}

void vgstp01(FDC* fdc) {
	fdc->wait = turbo ? 1 : BYTEDELAY;
	if (flpNext(fdc->flp, fdc->side)) {	// turn floppy to next byte, check IDX
		fdc->cnt--;
		if (fdc->cnt < 1) {		// check 15 IDX pulses
			fdc->flp->motor = 0;	// and off
			fdc->plan = NULL;
		}
	}
}

fdcCall vgStop[] = {&vgstp00, &vgstp01, NULL};

void vgstp(FDC* fdc) {
	fdc->plan = vgStop;
	fdc->pos = 0;
}

// ================================
// check head pos (h=1, v=1, motor)

// prepare : check if check is on, if disk inserted, set IDX count to 9
void vgchk00(FDC* fdc) {
	if ((fdc->com & 0x0c) != 0x0c) {	// if (h=0 || v=0), stops
		vgstp(fdc);
	} else if (!fdc->flp->insert) {		// if no disk, seek error, stop
		fdc->state |= 0x10;
		vgstp(fdc);
	} else {
		fdc->cnt = 9;		// try seek ADR in 9 spins
		fdc->pos++;
	}
}

// read ADR mark in cnt spins, change side every IDX, read ADR to buf
void vgchk01(FDC* fdc) {
	int res = seekADR(fdc);
	if (res == 2) {
		fdc->side ^= 1;
		fdc->cnt--;
		if (fdc->cnt < 1) {
			fdc->state |= 0x10;		// SEEK error
			vgstp(fdc);
		}
	} else if ((res == 1) && (fdc->buf[0] == fdc->trk)) {
		fdc->pos++;
	}
}

// check CRC
void vgchk02(FDC* fdc) {
	if (fdc->crc != fdc->fcrc)
		fdc->state |= 0x08;		// crc error
	fdc->pos++;
}

fdcCall vgCheck[] = {&vgchk00,&vgchk01,&vgchk02,&vgstp};	// prepare, seek/read ADR, check CRC, stop

void vgchk(FDC* fdc) {
	fdc->plan = vgCheck;
	fdc->pos = 0;
}

// =======
// restore

// prepare, do start delay (h=1)
void vgres00(FDC* fdc) {
	fdc->fmode = 0;
	fdc->trk = 0xff;
	fdc->wait += 1000000;		// delay for BV
	if (fdc->com & 8) {		// if h=1 : start motor, pause 15 ms
		fdc->wait += turbo ? 5000 : 15000;
		fdc->flp->motor = 1;
	}
	fdc->pos++;
}

// do step in until TRK0 or Rtrk=0
void vgres01(FDC* fdc) {
	if ((fdc->flp->trk == 0) || (fdc->trk == 0)) {
		if (fdc->flp->trk != 0)
			fdc->state |= 0x10;
		fdc->trk = 0;
		fdc->pos++;
	} else {
		fdc->wait += turbo ? 1 : pauses[fdc->com & 3];
		fdc->trk--;
		flpStep(fdc->flp, FLP_BACK);
	}
}

fdcCall vgRest[] = {&vgres00, &vgres01, &vgchk};

// ====
// seek

void vgseek00(FDC* fdc) {
	fdc->fmode = 0;
	fdc->wait += 1000000;		// 1ms delay for BV :)
	if (fdc->com & 8) {
		fdc->wait += turbo ? 1 : 15000;
		fdc->flp->motor = 1;
	}
	fdc->pos++;
}

void vgseek01(FDC* fdc) {
	if (fdc->trk == fdc->data) {
		fdc->pos++;
	} else if (fdc->trk < fdc->data) {
		flpStep(fdc->flp, FLP_FORWARD);
		fdc->trk++;
		fdc->wait += turbo ? 1 : BYTEDELAY;
	} else {
		flpStep(fdc->flp, FLP_BACK);
		fdc->trk--;
		fdc->wait += turbo ? 1 : BYTEDELAY;
	}
}

fdcCall vgSeek[] = {&vgseek00, &vgseek01, &vgchk};

// ===========================
// step/step forward/step back

void vgstpf(FDC* fdc) {
	fdc->step = 1;
	fdc->pos++;
}

void vgstpb(FDC* fdc) {
	fdc->step = 0;
	fdc->pos++;
}

void vgstep(FDC* fdc) {
	flpStep(fdc->flp, fdc->step ? FLP_FORWARD : FLP_BACK);
	fdc->wait += turbo ? 1 : pauses[fdc->com & 3];
	if (fdc->com & 0x10) {
		if (fdc->step)
			fdc->trk++;
		else
			fdc->trk--;
	}
	fdc->pos++;
}

fdcCall vgStepF[] = {&vgseek00,&vgstpf,&vgstep,&vgchk};
fdcCall vgStepB[] = {&vgseek00,&vgstpb,&vgstep,&vgchk};
fdcCall vgStep[] = {&vgseek00,&vgstep,&vgchk};

// ============
// read sectors

void vgrdsDBG(FDC* fdc) {
	fdc->pos++;
}

// prepare
void vgrds00(FDC* fdc) {
//	if ((fdc->com & 0xe1) == 0x80) printf("RDSec(%.2X)...T:%.2X S:%.2X H:%i (FT:%.2X)\n",fdc->com, fdc->trk, fdc->sec, fdc->side, fdc->flp->trk);
	//printf("fdc com %.2X\n",fdc->com);
	fdc->fmode = 1;
	if (!fdc->flp->insert) {
		vgstp(fdc);
	} else {
		fdc->flp->motor = 1;
		if (fdc->com & 4) fdc->wait += 15000;	// if (e=0) pause 15ms
		fdc->cnt = 5;				// seek sector in 5 spins
		fdc->pos++;
	}
}

// seek right sector ADR
void vgrds01(FDC* fdc) {
	int res = seekADR(fdc);
	if (res == 2) {
		fdc->cnt--;
		if (fdc->cnt < 1) {
			fdc->state |= 0x10;							// sector not found
			vgstp(fdc);
		}
	} else if (res == 1) {
		if ((fdc->buf[0] == fdc->trk) && (fdc->buf[2] == fdc->sec)) {			// check TRK,SEC
			if ((~fdc->com & 2) || (fdc->buf[1] == ((fdc->com & 8) ? 1 : 0))) {	// check HEAD (s, if c=1)
				if (fdc->crc != fdc->fcrc) {					// check CRC
					fdc->state |= 0x08;					// ADR crc error
					vgstp(fdc);
				} else {
					fdc->cnt = 52;						// DATA must be in next (22 + 30) bytes
					fdc->pos++;						// ADR found, next step
				}
			}
		}
	}
}

// seek sector DATA in next CNT bytes, else - array not found
void vgrds02(FDC* fdc) {
	if (fdc->cnt > 0) {
		fdc->tmp = flpRd(fdc->flp);
		flpNext(fdc->flp, fdc->side);
		fdc->wait += turbo ? 1 : BYTEDELAY;
		if ((fdc->flp->field == 2) || (fdc->flp->field == 3)) {
			fdc->buf[4] = fdc->tmp;
			fdc->pos++;
			fdc->wait = 0;
		}
		fdc->cnt--;
	} else {
		fdc->state |= 0x10;		// sector not found (array not found)
		vgstp(fdc);
	}
}

// init crc, set sector size
void vgrds03(FDC* fdc) {
	if (fdc->buf[4] == 0xf8) fdc->state |= 0x20;
	fdc->crc = 0xcdb4;
	add_crc_16(fdc, fdc->buf[4]);			// add DATA mark (F8 | FB)
	fdc->cnt = (0x80 << (fdc->buf[3] & 3));		// sector size (128,256,512,1024)
	fdc->drq = 0;
	fdc->dir = 1;					// dir: FDC to CPU
	fdc->wait = BYTEDELAY;
	fdc->pos++;
	fdc->tns = 0;
//	printf("vgrds03: %i.%i.%i\n",fdc->trk,fdc->sec,fdc->cnt);
}

// transfer CNT bytes flp->cpu
void vgrds04(FDC* fdc) {
	if (!vgGetByte(fdc)) return;
	add_crc_16(fdc, fdc->data);
	fdc->cnt--;
	if (fdc->cnt < 1) {
		fdc->wait = BYTEDELAY;
		fdc->pos++;
	}
}

// end: read, compare CRC, check multisector, stop
void vgrds05(FDC* fdc) {
	rdFCRC(fdc);
	fdc->wait += turbo ? 1 : (2 * BYTEDELAY);
	if (fdc->crc != fdc->fcrc) {
		// printf("crc error\n");
		fdc->state |= 0x08;
		fdc->pos++;
	} else if (fdc->com & 0x10) {
		fdc->sec++;
		// printf("sec %i\n", fdc->sec);
		fdc->pos = 1;
		fdc->cnt = 5;
	} else {
		fdc->pos++;
	}
}

fdcCall vgRdSec[] = {&vgrds00,&vgrds01,&vgrds02,&vgrds03,&vgrds04,&vgrds05,&vgstp};

// =============
// write sectors

// check write protect
void vgwrs00(FDC* fdc) {
	if (fdc->flp->protect) {
		fdc->state |= 0x40;
		vgstp(fdc);
	} else {
		fdc->drq = 1;
		fdc->dir = 0;
		fdc->pos++;
	}
}

// init crc, set F8/F9 (need roll flp back), prepare to write data
void vgwrs01(FDC* fdc) {
	flpPrev(fdc->flp, fdc->side);		// back to DATA mark
	fdc->crc = 0xcdb4;
	fdc->tmp = (fdc->com & 1) ? 0xf8 : 0xfb;
	add_crc_16(fdc, fdc->tmp);
	flpWr(fdc->flp, fdc->tmp);
	flpNext(fdc->flp, fdc->side);
	fdc->cnt = 0x80 << (fdc->buf[3] & 3);	// sector size
	fdc->wait = BYTEDELAY;
	fdc->tns = 0;
	fdc->pos++;
}

// write CNT bytes
void vgwrs02(FDC* fdc) {
	if (!vgSendByte(fdc)) return;
	fdc->cnt--;
	if (fdc->cnt < 1) {
		fdc->drq = 0;
		fdc->pos++;
	}
}

// write crc, check multisector, end
void vgwrs03(FDC* fdc) {
	wrCRC(fdc);
	fdc->wait += turbo ? 1 : (2 * BYTEDELAY);
	if (fdc->com & 0x10) {
		fdc->sec++;
		fdc->pos = 2;
		fdc->cnt = 5;
	} else {
		fdc->pos++;
	}
}

fdcCall vgWrSec[] = {&vgrds00,&vgwrs00,&vgrds01,&vgrds02,&vgwrs01,&vgwrs02,&vgwrs03,&vgstp};

// ============
// read address

void vgrda00(FDC* fdc) {
	int res = waitADR(fdc);
	if (res == 2) {
		fdc->cnt--;
		if (fdc->cnt < 1) {
			fdc->state |= 0x10;							// sector not found
			vgstp(fdc);
		}
	} else if (res == 1) {
		fdc->fcrc = 0;
		fdc->crc = 0xcdb4;
		add_crc_16(fdc,0xfe);				// add ADR mark
		fdc->cnt = 6;					// send 6 bytes FDC->CPU
		fdc->drq = 0;
		fdc->dir = 1;
		fdc->wait = BYTEDELAY;
		fdc->pos++;
	}
}

// read CNT bytes, last 2 bytes is CRC
void vgrda01(FDC* fdc) {
	if (!vgGetByte(fdc)) return;
	fdc->cnt--;
	switch (fdc->cnt) {
		case 0: fdc->fcrc |= (fdc->data & 0xff);
			fdc->wait = BYTEDELAY;
			fdc->pos++;
			break;
		case 1: fdc->fcrc = (fdc->data << 8) & 0xff00;
			break;
		case 5: fdc->sec = fdc->data;
		default: add_crc_16(fdc, fdc->data);
			break;

	}
}

fdcCall vgRdAdr[] = {&vgrds00,&vgrda00,&vgrda01,&vgchk02,&vgstp};

// ==========
// read track

// wait IDX
void vgrdt00(FDC* fdc) {
	fdc->wait += turbo ? 1 : BYTEDELAY;
	if (flpNext(fdc->flp, fdc->side)) {
		fdc->drq = 0;
		fdc->dir = 1;
		fdc->pos++;
	}
}

void vgrdt01(FDC* fdc) {
	if (!vgGetByte(fdc)) return;
	if (fdc->flp->pos == 0) fdc->pos++;		// end of trk (same as IDX)
}

fdcCall vgRdTrk[] = {&vgrds00,&vgrdt00,&vgrdt01,&vgstp};

// write track

void vgwrt00(FDC* fdc) {
	fdc->wait += turbo ? 1 : BYTEDELAY;
	if (flpNext(fdc->flp, fdc->side)) {
		fdc->pos++;
		fdc->wait = BYTEDELAY;
	}
}

void vgwrt01(FDC* fdc) {
	if (!vgSendByte(fdc)) return;
	if (fdc->flp->pos == 0) {
		fdc->drq = 0;
		fdc->pos++;
	}
}

void vgwrt02(FDC* fdc) {
	flpFillFields(fdc->flp, fdc->flp->rtrk, 1);
	fdc->pos++;
}

fdcCall vgWrTrk[] = {&vgrds00,&vgwrs00,&vgwrt00,&vgwrt01,&vgwrt02,&vgstp};

typedef struct {
	int mask;
	int val;
	fdcCall* plan;
} vgComItem;

vgComItem vgComTab[] = {
	{0xf0, 0x00, vgRest},		// 0000xxxx - restore
	{0xf0, 0x10, vgSeek},		// 0001xxxx - seek
	{0xe0, 0x20, vgStep},		// 001xxxxx - step
	{0xe0, 0x40, vgStepF},		// 010xxxxx - step forward
	{0xe0, 0x60, vgStepB},		// 011xxxxx - step back
	{0xe1, 0x80, vgRdSec},		// 100xxxx0 - read sectors
	{0xe0, 0xa0, vgWrSec},		// 101xxxxx - write sectors
	{0xfb, 0xc0, vgRdAdr},		// 11000x00 - read address
	{0xfb, 0xe0, vgRdTrk},		// 11100x00 - read track
	{0xfb, 0xf0, vgWrTrk},		// 11110x00 - write track
	{0x00, 0x00, vgStop}		// othercom - do nothing
};

void vgExec(FDC* fdc, unsigned char com) {
	int idx;
	if ((com & 0xf0) == 0xd0) {	// interrupt (doesn't mind about FDC is idle)
		// printf("INT:%.2X\n",com);
		fdc->wait = 0;
		vgstp(fdc);
	} else if (fdc->idle) {		// if FDC is idle
		fdc->com = com;
		idx = 0;
		while (1) {		// vgComTab[idx].mask != 0
			if ((com & vgComTab[idx].mask) == vgComTab[idx].val) {
				fdc->plan = vgComTab[idx].plan;
				fdc->wait = 0;
				fdc->pos = 0;
				fdc->idle = 0;
				fdc->state = 0;
				fdc->irq = 0;
				break;
			}
			idx++;
		}
	}
}

void vgWrite(FDC* fdc, int adr, unsigned char val) {
	switch (adr) {
		case FDC_COM:
			if (!fdc->mr) break;		// no commands during master reset
			vgExec(fdc, val);
			break;
		case FDC_TRK:
			fdc->trk = val;
			break;
		case FDC_SEC:
			fdc->sec = val;
			break;
		case FDC_DATA:
			fdc->data = val;
			fdc->drq = 0;
			break;
	}
}

unsigned char vgRead(FDC* fdc, int adr) {
	unsigned char res = 0xff;
	switch (adr) {
		case FDC_COM:
			//fdc->state &= ~0x08;		// debug: reset crc error
			fdc->state &= 0x7e;
			if (!fdc->flp->insert) fdc->state |= 0x80;
			if (!fdc->idle) fdc->state |= 0x01;
			if (fdc->fmode == 0) {
				fdc->state &= 0x99;
				if (fdc->flp->protect) fdc->state |= 0x40;
				if (fdc->flp->motor) fdc->state |= 0x20;
				if (fdc->flp->trk == 0) fdc->state |= 0x04;
				if (fdc->flp->insert && fdc->flp->index) fdc->state |= 0x02;
			} else if (fdc->fmode == 1) {
				fdc->state &= 0xfd;
				if (fdc->drq) fdc->state |= 0x02;
			}
			res = fdc->state;
//			printf("in 1F = %.2X\n",res);
			break;
		case FDC_TRK:
			res = fdc->trk;
			break;
		case FDC_SEC:
			res = fdc->sec;
			break;
		case FDC_DATA:
			res = fdc->data;
			fdc->drq = 0;
//			printf("%.2X ",res);
			break;
	}
	return res;
}

void vgReset(FDC* fdc) {
	fdc->trk = 0;
	fdc->sec = 0;
	fdc->data = 0;
	fdc->state = 0;
	fdc->idle = 0;
	fdc->irq = 1;
	fdc->drq = 0;
	fdc->side= 0;
	fdc->plan = NULL;
	fdc->pos = 0;
	fdc->wait = -1;
}

void vgSetMR(FDC* fdc, int z) {
	fdc->mr = z;
	if (z == 0) {
		fdc->idle = 1;
		vgExec(fdc,0x03);	// restore
		fdc->sec = 1;
	}
}
