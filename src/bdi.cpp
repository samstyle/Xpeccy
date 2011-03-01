#include "bdi.h"
#include "z80.h"

#include <stdio.h>

#define	BYTEDELAY	224
#define IDXDELAY	600
#define	MSDELAY		7160		// 1ms delay
#define	TRBDELAY	MSDELAY/2

bool vgdebug = false;

// BDI

BDI::BDI() {
	for (int32_t i=0; i<4; i++) flop[i].id = i;
	vg93.fptr = &flop[0];
	tf = tab = BYTEDELAY;
	t = 0;
}

int32_t BDI::getport(int32_t p) {
	int32_t port = p;
	pcatch = false;
	if (enable && active && (p & 0x03)==0x03) {
		if ((p & 0x82) == 0x82) port=0xff;
		if ((p & 0xe2) == 0x02) port=0x1f;
		if ((p & 0xe2) == 0x22) port=0x3f;
		if ((p & 0xe2) == 0x42) port=0x5f;
		if ((p & 0xe2) == 0x62) port=0x7f;
		pcatch = true;
	}
	return port;
}

bool BDI::out(int32_t port,uint8_t val) {
	port = getport(port);
	if (!pcatch) return false;
	switch (port) {
		case 0x1f: vg93.command(val); break;
		case 0x3f: vg93.trk = val; break;
		case 0x5f: vg93.sec = val; break;
		case 0x7f: vg93.data = val;
			vg93.drq = false;
			break;
		case 0xff: // vg93.drv = val&0x03;
			vg93.fptr = &flop[val&0x03];	// selet floppy
			vg93.setmr(val&0x04);		// master reset
			vg93.block = val&0x08;
			vg93.side = (val&0x10)?1:0;
			vg93.mfm = val&0x40;
			break;
	}
	return true;
}

bool BDI::in(int32_t port, uint8_t* val) {
	port = getport(port);
	if (!pcatch) return false;
	*val = 0xff;
	switch (port) {
		case 0x1f: *val = vg93.getflag(); vg93.irq = false; break;
		case 0x3f: *val = vg93.trk; break;
		case 0x5f: *val = vg93.sec; break;
		case 0x7f: *val = vg93.data; vg93.drq = false; break;
		case 0xff: *val = (vg93.irq << 7) | (vg93.drq << 6); break;
	}
	return true;
}

void BDI::sync(uint32_t tk) {
	uint32_t tz;
	while (tk > 0) {
		if (tk < tf) {
			tz = tk;
			tf -= tk;
		} else {
			tz = tf;
			tf = tab;
			vg93.fptr->next();
		}
		t += tz;
		vg93.idxold = vg93.idx;
		vg93.idx = ((t - vg93.fptr->ti) < IDXDELAY);
		vg93.strb = (!vg93.idxold) && vg93.idx;
		if (vg93.wptr != NULL) {
			vg93.count -= tz;
			while ((vg93.wptr != NULL) && (vg93.count < 0)) vg93.tick();
		}
		tk -= tz;
	}
}

// VG93

VG93::VG93() {
	wptr = NULL;
	count = 0;
	idle = true;
}

/*
01,n	jump to work n

10	pause (com & 3)
11,n	pause (n)

20,n	set flag mode n

30	drq = 0
31	drq = 1

80,n	Rtr = n
81	Rtr--
82	Rtr++
83,n,d	if (flp::trk == n) jr d
84,n,d	if (Rtr == n) jr d
85,d	if (Rtr == Atr) jr d
86,d	if (Rtr == Rdat) jr d
87,d	if (Rtr < Rdat) jr d
88	Rsec = Rdat
89,d	if (Rtr != Rdat) jr d
8a,d	if (Rsec != Rdat) jr d
8b,d	if (side != Rdat) jr d
8c,n,d	if (Rdat == n) jr d
8d	Rsec++
8e,n,d	if (Rbus == n) jr d
8f,n	Rbus = n

90,n	read byte from floppy in buf[n]
91	read byte in Rdat
92	read byte in Rbus
93	write from Rdat
94	write from Rbus

A0	step in
A1	step out
A2	step (prev)

A8	next byte (seek)
A9	next byte (rd/wr)
AA	change floppy side

B0,n	icnt = n
B1	icnt--
B2	icnt++
B3,n,d	if (icnt==n) jr d
B4,n,d	if (icnt!=n) jr d
B5	ic = buf[0]:128,256,512,1024

C0,a,b	flag & a | b
C1	fill fields @ curr.flp.trk

D0	init CRC
D1,n	add byte n to CRC
D2	add Rdat to CRC (if field != 0)
D3	add Rbus to CRC (if field != 0)
D4	read flp.CRC (full)
D5,d	if (CRC == flp.CRC) jr d
D7	read fcrc.hi
D8	read fcrc.low

E0,n	n: 0 - stop motor/unload head; 1 - start motor/load head

F0,d	jr d
F8,d	if (flp.wprt) jr d
F9,d	if (flp.ready (insert)) jr d
FA,d	if (sdir = out (1)) jr d
FB,n,d	if (flp.field = n) jr d
FC,d	if (index) jr d
FD,n,d	if (com & n == 0) jr d
FE,n,d	if (com & n != 0) jr d
FF	end
*/

uint8_t vgidle[256] = {
	0xb0,15,
	0xa8,
	0xfc,2,
	0xf0,-5,
	0xb1,
	0xb4,0,-9,
	0xe0,0,
	0xf1
};

uint8_t vgwork[16][256] = {
// 0: restore
	{
		0x20,0,			// flag mode 0
		0xfd,8,2,0xe0,1,	// if h=0 delay 50 mks
		0x80,255,		// Rtr = 255
		0x83,0,8,		// if flp.trk==0 jr +8 (@ 80)
		0x84,0,5,		// if Rtr==0 jr +5 (@ 80)
		0x10,			// pause (com & 3)
		0xa0,			// step in
		0x81,			// Rtr--
		0xf0,-11,		// jr -11 (@ 83)
		0x80,0,			// Rtr = 0
		0x01,12			// WORK 12 (check head pos)
	},
// 1: seek
	{
		0x20,0,			// flag mode 0
		0xfd,8,2,0xe0,1,
		0x86,12,		// if Rtr = Rdat jr +d (@ 01)
		0x87,5,			// if Rtr < Rdat jr +d (@ 82)
		0x10,			// pause (com & 3)
		0x81,			// Rtr--
		0xa0,			// step in
		0xf0,-9,		// jr -9 (@ 86)
		0x10,			// pause (com & 3)
		0x82,			// Rtr++
		0xa1,			// step out
		0xf0,-7,		// jr -7 (@ jr @ 86)
		0x01,12			// WORK 12
	},
// 2: step
	{
		0xfa,2,			// if sdir = out, jr +2 (jp work 4)
		0x01,3,			// WORK 3 (step in)
		0x01,4			// WORK 4 (step out)
	},
// 3: step in
	{
		0x20,0,			// flag mode 0
		0xfd,8,2,0xe0,1,	
		0xfd,16,1,0x82,		// if (b4,com) Rtr++
		0xa1,			// step out
		0x10,			// pause (com & 3)
		0x01,12			// WORK 12
	},
// 4: step out
	{
		0x20,0,			// flag mode 0
		0xfd,8,2,0xe0,1,
		0xfd,16,1,0x81,		// if (b4,com) Rtr--
		0x83,0,4,		// if (flp.tr0) jr +4 (@ 80)
		0xa0,			// step in
		0x10,			// pause (com & 3)
		0x01,12,		// WORK 12
		0x80,0,			// Rtr = 0
		0x01,12			// WORK 12
	},
// 5: read sector
	{
		0x20,1,			// flag mode 1
		0xc0,0x9f,0x00,		// reset b 5,6 in flag
		0xf9,1,0xff,		// ifn flp.ready END
		0xfd,4,2,0x11,4,	// if (b2,com) wait (15ms)
		0xe0,1,
		0x01,14			// WORK 14 (ХИТРОСТЬ DESU)
	},
// 6: write sector
	{
		0x20,1,			// flag mode 1
		0xc0,0x83,0x00,		// res b2-6 in flag
		0xf9,1,0xff,		// ifn flp.ready END
		0xfd,4,2,0x11,4,	// if (b2,com) wait 15ms
		0xe0,1,
		0xf8,2,			// if write protect jr +2
		0x01,15,		// WORK 15
		0xc0,0xbf,0x40,		// set b6 (write protect)
		0xff			// end
	},
// 7: read address
	{
		0x20,1,			// flag mode 1
		0xc0,0x83,0x00,		// reset b 2-6 in flag
		0xf9,1,0xff,		// ifn flp.ready END
		0xfd,4,2,0x11,4,	// if (b2,com) wait (15ms)
		0xe0,1,
		0xb0,6,			// ic = 6
		0x02,13,		// CALL WORK 13 (wait for address field)
		0xd0,0xd1,0xfe,		// init CRC, CRC << 0xfe (address marker)
		0x91,0xd2,0x88,0x31,0xa9,	// read byte, add to crc, Rsec = byte, drq=1, next(slow)
		0x91,0xd2,0x31,0xa9,	// read byte, add to crc, drq=1, next(slow)
		0x91,0xd2,0x31,0xa9,
		0x91,0xd2,0x31,0xa9,
		0x91,0xd7,0x31,0xa9,	// read byte, read fcrc.low, drq=1, next(slow)
		0x91,0xd8,0x31,0xa9,	// read byte, read fcrc.hi, drq=1, next(slow)
		0xc0,0xf7,0x00,		// reset CRC ERROR
		0xd5,3,			// if crc == fcrc, jr +3 (end)
		0xc0,0xf7,0x08,		// set CRC ERROR (08)
		0xff			// END
	},
// 8: read track
	{
		0x20,1,			// set flag mode 1
		0xc0,0x83,0x00,		// reset b 2-6 in flag
		0xf9,1,0xff,		// ifn flp.ready END
		0xfd,4,2,0x11,4,	// if (b2,com) wait 15ms
		0xe0,1,
		0x30,			// drq = 0
		0xfc,3,0xa8,0xf0,-5,	// wait for STRB
		0x32,5,			// if (drq=0) jr +3
		0xc0,0xfb,0x04,0xf0,2,	// DATA LOST
		0x91,0x31,		// read byte in Rdat, drq=1
		0xa9,			// next (flase)
		0xfc,2,0xf0,-14,	// ifn STRB jr -14 (@ 32)
		0xff			// END
	},
// 9: write track
	{
//	0x00,1,
		0x20,1,			// set flag mode 1
		0xc0,0x83,0x00,		// res b2-6 in flag
		0xf9,1,0xff,		// ifn flp.ready END
		0xfd,4,2,0x11,4,	// if (b2,com) wait 15ms
		0xe0,1,
		0xf8,2,			// if write protect jr +2
		0xf0,4,			// jr +4 (@ 31)
		0xc0,0xbf,0x40,0xff,	// WRITE PROTECT, END
		0x31,			// drq = 1
//		0xa9,0xa9,0xa9,		// wait 3 bytes (slow)
//		0x32,4,			// if (drq == 0) jr +4
//		0xc0,0xfb,0x04,0xff,	// DATA LOST, END
		0xfc,3,0xa9,0xf0,-5,	// wait for STRB (slow)
		0x32,5,			// if (drq == 0) jr +5
		0xc0,0xfb,0x04,		// DATA LOST
		0xf0,2,			// jr +2
		0x93,			// write Rdat
		0x31,			// drq = 1
		0xa9,			// next (slow)
		0xfc,2,			// if STRB jr +2 (@ C1)
		0xf0,-14,		// jr -14 (@ 32)
		0xc1,			// fill fields (change F5,F6,F7)
		0xff
	},
// 10: interrupt (TODO: conditions)
	{
		0xff
	},
// 11: undef
	{
		0xff
	},
// 12: check head pos (end of restore/seek/step commands)
	{
		0xfe,4,1,0xff,		// if (com & 4 == 0) END
		0x11,4,			// pause 15 ms
		0xfb,0,5,		// WAIT FOR STRB or field 0
		0xfc,3,
		0xa8,0xf0,-8,
		0xb0,0,			// ic = 0
		0xfb,1,16,		// if fptr.field = 1 (header) jr +16
		0xa8,			// next (fast)
		0xfc,2,			// if STRB jr +2
		0xf0,-8,		// jr -8 (@ FB)
		0xaa,			// change floppy head (up/down)
		0xb2,			// ic++
		0xb3,9,2,		// if (icnt==9) jr +2
		0xf0,-9,		// jr -9 (@ jr -8 @ FB)
		0xc0,0xef,0x10,0xff,	// SEEK ERROR, END
		0xd0,0xd1,0xfe,		// init CRC, CRC << FE (address marker)
		0x90,0,0xd3,0xa8,	// read 6 bytes in buffer (Rbus = byte, CRC << Rbus)
		0x90,1,0xd3,0xa8,
		0x90,2,0xd3,0xa8,
		0x90,3,0xd3,0xa8,
		0x90,4,0xd7,0xa8,	// d7: read fptr.hi
		0x90,5,0xd8,		// d8: read fptr.low
		0x85,2,0xf0,-36,	// if (Atr != Rtr) jr -28 (@ jr -9 @ jr -8 @ FB)
		0xc0,0xf7,0x00,		// reset CRC ERROR
		0xd5,3,			// if fcrc == crc, jr +3 (end)
		0xc0,0xf7,0x08,		// set CRC ERROR
		0xff			// END
	},
// 13: [CALL] wait for address field, in: ic=X (X IDX during execution = END with SEEK ERROR, or RETURN if succes)
	{
		0xfb,0,5,		// wait for field 0 or IDX
		0xa8,
		0xfc,2,
		0xf0,-8,
		0xfb,1,13,		// if fptr.field = 1 (header) jr +13 (@ 03)
		0xa8,			// next (false)
		0xfc,2,			// if STRB jr +2
		0xf0,-8,		// jr -8 (@ FB)
		0xb1,			// ic--
		0xb4,0,-6,		// if (ic!=0) jr -6 (@ jr -8 @ FB)
		0xc0,0xef,0x10,0xff,	// SEEK ERROR, END
		0x03			// RET
	},
// 14: sector read [main]
	{
		0xb0,5,			// ic = 5
		0x02,13,		// CALL WORK 13 (wait address)
		0xd0,0xd1,0xfe,		// init CRC, CRC << fe
		0x91,0xd2,0xa8,		// read [trk] in Rdat, add to CRC, next
		0x89,-10,		// if Rtrk != Rdat jr -(@ 02)
		0x91,0xd2,0xa8,		// read [side] in Rdat, add to CRC, next
		0xfd,2,2,		// ifn (b1,com) skip side check
		0x8b,-18,		// if (com & 8) ? Rdat jr -(@ 02)
		0x91,0xd2,0xa8,		// read [sect] in Rdat, add to CRC, next
		0x8a,-23,		// if Rsec != Rdat jr -17 (@ 02)
		0x90,0,0xd3,0xa8,	// read [len] in buf[0] & Rbus, add to CRC (Rbus), next
		0xd7,0xa8,0xd8,0xa8,	// read fcrc
		0xd5,5,			// if fcrc == crc, jr +(read data)
		0xc0,0xf7,0x08,		// set CRC ERROR
		0xf0,-38,		// seek again
		0xc0,0xf7,0x00,		// reset CRC ERROR
		0xb0,22,
		0xa8,0xb1,0xb4,0,-5,	// skip 22 bytes (space)
		0xb0,30,
		0x91,0xa9,		// read byte in Rdat, next (slow)
		0x8c,0xf8,11,		// if Rdat = F8 jr @(set b5,flag)
		0x8c,0xfb,13,		// if Rdat = FB jr @(res b5,flag)
		0xb1,0xb4,0,-12,	// F8 | FB must be in next 30(?) bytes
		0xc0,0xef,0x10,0xff,	// else: ARRAY NOT FOUND, END
		0xc0,0xbf,0x20,		// F8: set b5,flag
		0xf0,3,			// jr +3
		0xc0,0xbf,0x00,		// FB: res b5,flag
//	0,1,
		0xd0,0xd2,		// init crc, CRC << Rdat (F8 or FB)
		0xb5,			// ic = sector len
		0x30,			// drq=0
		0x32,5,			// if (drq=0) jr +5
		0xc0,0xfb,0x04,		// set DATA LOST
		0xf0,3,
		0x91,0xd2,0x31,		// read byte in Rdat, CRC << Rdat, drq=1
		0xa9,			// next (flase)
		0xb1,0xb4,0,-15,	// ic--; if ic!=0 jr -(@ 32)
		0xd7,0xa8,0xd8,0xa8,	// read fptr.crc
		0xd5,4,
		0xc0,0xf7,0x08,0xff,	// set CRC ERROR, END
//	0,0,
		0xfd,16,0xff,		// ifn (bit 4,com) END [multisector]
		0x8d,			// Rsec++
		0x01,14			// back to start (WORK 14)
	},
// 15: sector write [main]
	{
		0xb0,5,			// ic = 5
		0x02,13,		// CALL WORK 13 (wait address)
		0xd0,0xd1,0xfe,		// init CRC, CRC << fe (address marker)
		0x91,0xd2,0xa8,		// read [trk] in Rdat, add to CRC, next
		0x89,-10,		// if Rtrk != Rdat jr -6 (@ 02)
		0x91,0xd2,0xa8,		// read [side] in Rdat, add yo CRC, next
		0xfd,2,2,		// ifn (b1,com) skip side check
		0x8b,-18,		// if (com & 8) ? Rdat jr -(@ 02)
		0x91,0xd2,0xa8,		// read [sect] in Rdat, add to CRC, next
		0x8a,-23,		// if Rsec != Rdat jr -(@ 02)
		0x90,0,0xd3,0xa8,	// read [len] in buf[0] & Rbus, add Rbus to CRC, next
		0xd7,0xa8,0xd8,0xa8,	// read fcrc
		0xd5,5,			// if fcrc == crc, jr @(write data)
		0xc0,0xf7,0x08,		// set CRC ERROR
		0xf0,-38,		// seek again
		0xa8,0xa8,		// skip 2 bytes
		0x31,			// drq = 1	(in next 8 bytes Rdat must be loaded)
		0xa9,0xa9,0xa9,0xa9,	// skip 8 bytes (slow)
		0xa9,0xa9,0xa9,0xa9,
		0x32,4,			// if (drq == 0) jr +4
		0xc0,0xfb,0x04,0xff,	// DATA LOST, END
		0xb0,45,		// ic = 45 (wait for F8/FB in next 45 bytes)
		0x92,			// read in Rbus
		0x8e,0xf8,12,		// if Rbus = F8
		0x8e,0xfB,9,		// 	or = FB jr +9 (@ 8f)
		0xa9,			// next (slow)
		0xb1,			// ic--
		0xb4,0,-12,		// if ic!=0 jr -11 (@ 92)
		0xc0,0xef,0x10,0xff,	// ARRAY NOT FOUND, END
		0x8f,0xf8,		// Rbus = F8
		0xfe,1,2,0x8f,0xfb,	// ifn (b0,com) Rbus = FB
		0xd0,0xd3,		// init crc, add Rbus (F8 | FB) to crc
		0x94,			// write Rbus (F8 or FB) (add to crc automaticly)
		0xa9,			// next (slow)
		0xb5,			// ic = sec.len
		0x32,5,			// if (drq = 0) jr +5
		0xc0,0xfb,0x04,		// DATA LOST
		0xf0,3,			// jr +3
		0x93,0xd2,0x31,		// write Rdat, add Rdat to CRC, drq = 1
		0xa9,			// next (false)
		0xb1,			// ic--
		0xb4,0,-15,		// if (ic != 0) jr -(@ 32)
		
		0xd9,0xa8,0xda,		// write (crc.hi, crc.low) - crc
		
		0xfd,16,0xff,		// ifn (bit 4,com) END [multisector]
		0x8d,			// Rsec++
		0x01,15			// back to start (WORK 15)
	}
};

uint8_t p1,dlt;
int32_t delays[6]={6 * MSDELAY,12 * MSDELAY,24 * MSDELAY,32 * MSDELAY,15 * MSDELAY, 50 * MSDELAY};	// 6, 12, 20, 32, 15, 50ms (hlt-hld)

void v00(VG93* p) {p1 = *(p->wptr++); vgdebug = (p!=0); printf("== DEBUG\n");}

void v01(VG93* p) {p1 = *(p->wptr++); p->wptr = vgwork[p1];}
void v02(VG93* p) {p1 = *(p->wptr++); p->sp = p->wptr; p->wptr = vgwork[p1];}
void v03(VG93* p) {p->wptr = p->sp;}

void v10(VG93* p) {p->count += bdi->turbo?TRBDELAY:delays[p->com & 3];}
void v11(VG93* p) {p1 = *(p->wptr++); p->count += bdi->turbo?TRBDELAY:delays[p1];}

void v20(VG93* p) {p->mode = *(p->wptr++);}

void v30(VG93* p) {p->drq = false;}
void v31(VG93* p) {p->drq = true;}
void v32(VG93* p) {dlt = *(p->wptr++); if (!p->drq) p->wptr += (int8_t)dlt;}

void v80(VG93* p) {p->trk = *(p->wptr++);}
void v81(VG93* p) {p->trk--;}
void v82(VG93* p) {p->trk++;}
void v83(VG93* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->fptr->trk == p1) p->wptr += (int8_t)dlt;}
void v84(VG93* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->trk == p1) p->wptr += (int8_t)dlt;}
void v85(VG93* p) {dlt = *(p->wptr++); if (p->buf[0] == p->trk) p->wptr += (int8_t)dlt;}
void v86(VG93* p) {dlt = *(p->wptr++); if (p->trk == p->data) p->wptr += (int8_t)dlt;}
void v87(VG93* p) {dlt = *(p->wptr++); if (p->trk < p->data) p->wptr += (int8_t)dlt;}
void v88(VG93* p) {p->sec = p->data;}
void v89(VG93* p) {dlt = *(p->wptr++); if (p->trk != p->data) p->wptr += (int8_t)dlt;}
void v8A(VG93* p) {dlt = *(p->wptr++); if (p->sec != p->data) p->wptr += (int8_t)dlt;}
void v8B(VG93* p) {dlt = *(p->wptr++); if (((p->com & 8)?0:1) != p->data) p->wptr += (int8_t)dlt;}
void v8C(VG93* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->data == p1) p->wptr += (int8_t)dlt;}
void v8D(VG93* p) {p->sec++;}
void v8E(VG93* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->bus == p1) p->wptr += (int8_t)dlt;}
void v8F(VG93* p) {p->bus = *(p->wptr++);}

void v90(VG93* p) {p1 = *(p->wptr++); p->buf[p1] = p->fptr->rd(); p->bus = p->buf[p1];}
void v91(VG93* p) {p->data = p->fptr->rd();}
void v92(VG93* p) {p->bus = p->fptr->rd();}
void v93(VG93* p) {p->fptr->wr(p->data);}
void v94(VG93* p) {p->fptr->wr(p->bus);}

void vA0(VG93* p) {p->sdir = false; p->fptr->step(false);}
void vA1(VG93* p) {p->sdir = true; p->fptr->step(true);}
void vA2(VG93* p) {p->fptr->step(p->sdir);}

void vA8(VG93* p) {if (bdi->turbo) {bdi->tf = BYTEDELAY; p->count = 0; p->fptr->next();} else {p->count += bdi->tf;}}
void vA9(VG93* p) {p->count += bdi->tf;}
void vAA(VG93* p) {p->side = !p->side;}

void vB0(VG93* p) {p->ic = *(p->wptr++);}
void vB1(VG93* p) {p->ic--;}
void vB2(VG93* p) {p->ic++;}
void vB3(VG93* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->ic == p1) p->wptr += (int8_t)dlt;}
void vB4(VG93* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->ic != p1) p->wptr += (int8_t)dlt;}
void vB5(VG93* p) {p->ic = (128 << (p->buf[0] & 3));}				// 128,256,512,1024

void vC0(VG93* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); p->flag &= p1; p->flag |= dlt;}
void vC1(VG93* p) {p->fptr->fillfields(p->fptr->rtrk,true);}

void vD0(VG93* p) {p->crc = 0xcdb4; p->crchi = true;}
void vD1(VG93* p) {p->addcrc(*(p->wptr++));}
void vD2(VG93* p) {p->addcrc(p->data);}
void vD3(VG93* p) {p->addcrc(p->bus);}
void vD4(VG93* p) {p->fcrc = p->fptr->rd(); p->fptr->next(); p->fcrc |= (p->fptr->rd() << 8);}	// read crc from floppy
void vD5(VG93* p) {
//	printf ("CC\tVG: %.8X\tFLP: %.8X\n",p->crc,p->fcrc);
	dlt = *(p->wptr++); if (p->crc == p->fcrc) p->wptr += (int8_t)dlt;
}
void vD7(VG93* p) {p->fcrc = (p->fptr->rd() << 8);}
void vD8(VG93* p) {p->fcrc |= p->fptr->rd();}
void vD9(VG93* p) {p->fptr->wr((p->crc & 0xff00) >> 8);}
void vDA(VG93* p) {p->fptr->wr(p->crc & 0xff);}
void vDF(VG93* p) {if (p->crchi) {
			p->fptr->wr((p->crc & 0xff00) >> 8);
		} else {
			p->fptr->wr(p->crc & 0xff);
		}
		p->crchi = !p->crchi;
}

void vE0(VG93* p) {
	p1 = *(p->wptr++);
	if (p1 == 0) {
		p->fptr->motor = false;
		p->fptr->head = false;
	} else {
		if (!p->fptr->head) p->count += bdi->turbo?TRBDELAY:delays[5];
		p->fptr->motor = true;
		p->fptr->head = true;
	}
}

void vF0(VG93* p) {dlt = *(p->wptr++); p->wptr += (int8_t)dlt;}
void vF1(VG93* p) {p->wptr = NULL; p->count = 0;}
void vF8(VG93* p) {dlt = *(p->wptr++); if (p->fptr->protect) p->wptr += (int8_t)dlt;}
void vF9(VG93* p) {dlt = *(p->wptr++); if (p->fptr->insert) p->wptr += (int8_t)dlt;}	// READY
void vFA(VG93* p) {dlt = *(p->wptr++); if (p->sdir) p->wptr += (int8_t)dlt;}
void vFB(VG93* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->fptr->field == p1) p->wptr += (int8_t)dlt;}
void vFC(VG93* p) {dlt = *(p->wptr++); if (p->strb) p->wptr += (int8_t)dlt;}
void vFD(VG93* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if ((p->com & p1) == 0) p->wptr += (int8_t)dlt;}
void vFE(VG93* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if ((p->com & p1) != 0) p->wptr += (int8_t)dlt;}
void vFF(VG93* p) {p->irq = true; p->idle = true; p->wptr = &vgidle[0]; vgdebug = false;}

VGOp vgfunc[256] = {
	&v00,&v01,&v02,&v03,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&v10,&v11,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&v20,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&v30,&v31,&v32,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&v80,&v81,&v82,&v83,&v84,&v85,&v86,&v87,&v88,&v89,&v8A,&v8B,&v8C,&v8D,&v8E,&v8F,
	&v90,&v91,&v92,&v93,&v94,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&vA0,&vA1,&vA2,NULL,NULL,NULL,NULL,NULL,&vA8,&vA9,&vAA,NULL,NULL,NULL,NULL,NULL,
	&vB0,&vB1,&vB2,&vB3,&vB4,&vB5,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&vC0,&vC1,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&vD0,&vD1,&vD2,&vD3,&vD4,&vD5,NULL,&vD7,&vD8,&vD9,&vDA,NULL,NULL,NULL,NULL,&vDF,
	&vE0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&vF0,&vF1,NULL,NULL,NULL,NULL,NULL,NULL,&vF8,&vF9,&vFA,&vFB,&vFC,&vFD,&vFE,&vFF
};

void VG93::addcrc(uint8_t val) {
	uint32_t tkk = crc;
	tkk ^= val << 8;
	for (int32_t i = 8; i; i--) {
		if ((tkk *= 2) & 0x10000) tkk ^= 0x1021;
	}
	crc = tkk & 0xffff;

}

void VG93::tick() {
	cop = *(wptr++);
	if (vgdebug) {
		switch (cop) {
			case 0xfb: printf("%.2X (fptr.fld = %i)\n",cop,fptr->field); break;
			case 0x91: printf("%.2X (rd = %.2X, pos %i)\n",cop,fptr->rd(),fptr->pos); break;
			case 0x93: printf("%.2X (wr %.2X, pos %i)\n",cop,data,fptr->pos); break;
			default: printf("%.2X\n",cop); break;
		}
	}
	if (vgfunc[cop] == NULL) {printf("VGcom: %.2X\n",cop); throw(1);}
	vgfunc[cop](this);
}

uint8_t VG93::getflag() {
	uint8_t res = (fptr->insert ? 0 : 128) | (idle ? 0 : 1);
	switch (mode) {
		case 0: res |= (fptr->protect?0x40:0) | 0x20 | (flag & 0x18) | ((fptr->trk==0)?4:0) | (idx?2:0); break;
		case 1:
		case 2: res |= (flag & 0x7c) | (drq?2:0); break;
		default: printf("Flag mode\n"); throw(0);
	}
	if (vgdebug) printf("read FLAG %.2X\n",res);
	return res;
}

void VG93::setmr(bool z) {
	if (!mr && z) {		// 0->1 : execute com 3
//		wnum=0xff;
		command(0x03);	// restore
		mr = z;
		sec = 1;
	} else {
		mr = z;
	}
}

void VG93::command(uint8_t val) {
	if (!mr) return;			// no commands aviable during master reset
	if (idle) {
//		printf ("vg com: %.2X\n",val);
		com=val;
		wptr = NULL;
		if ((val&0xf0) == 0x00) wptr = vgwork[0];	// restore		00..0f
		if ((val&0xf0) == 0x10) wptr = vgwork[1];	// seek			10..1f
		if ((val&0xe0) == 0x20) wptr = vgwork[2];	// step			20..3f
		if ((val&0xe0) == 0x40) wptr = vgwork[3];	// step in		40..5f
		if ((val&0xe0) == 0x60) wptr = vgwork[4];	// step out		60..7f
		if ((val&0xe1) == 0x80) wptr = vgwork[5];	// read sector
		if ((val&0xe0) == 0xa0) wptr = vgwork[6];	// write sector
		if ((val&0xfb) == 0xc0) wptr = vgwork[7];	// read address
		if ((val&0xfb) == 0xe0) wptr = vgwork[8];	// read track
		if ((val&0xfb) == 0xf0) wptr = vgwork[9];	// write track
		if (wptr == NULL) wptr = vgwork[11];
		count = -1;
		idle = false;
		irq = false;
		drq = false;
 	}
	if ((val&0xf0) == 0xd0) {
		wptr = vgwork[10];		// interrupt
		count = -1;
		idle = false;
		irq = false;
		drq = false;
	}
/*
	if (vgdebug) {
		if ((wptr == vgwork[5]) | (wptr == vgwork[6])) {
			printf ("VGCOM %.2X (T:%i, Sc:%i)\n",val,trk,sec);
		} else {
			printf("VGCOM %.2X\n",val);
		}
	}
*/
}
