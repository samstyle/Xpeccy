// NOTE: FF is BDI-port, but 1F,3F,5F,7F belongs to FDC (VG93)

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "bdi.h"

#define	ONEDELAY	140			// transit: 1 dot = 140 ns
#define	BYTEDELAY	224 * ONEDELAY
#define IDXDELAY	600 * ONEDELAY
#define	MSDELAY		7160 * ONEDELAY		// 1ms delay
#define	TRBDELAY	MSDELAY * 3

int pcatch = 0;	// 1 when bdi takes i/o request
int fdcFlag = 0;

// FDC

FDC* fdcCreate(int tp) {
	FDC* fdc = (FDC*)malloc(sizeof(FDC));
	fdc->flop[0] = flpCreate(0);
	fdc->flop[1] = flpCreate(1);
	fdc->flop[2] = flpCreate(2);
	fdc->flop[3] = flpCreate(3);
	fdc->fptr = fdc->flop[0];
	fdc->type = tp;
	fdc->t = 0;
	fdc->tf = 0;
	fdc->wptr = NULL;
	fdc->count = 0;
	fdc->idle = 1;
	fdc->status = FDC_IDLE;
	return fdc;
}

void fdcDestroy(FDC* fdc) {
	flpDestroy(fdc->flop[0]);
	flpDestroy(fdc->flop[1]);
	flpDestroy(fdc->flop[2]);
	flpDestroy(fdc->flop[3]);
	free(fdc);
}

// VG93

// THIS IS BYTECODE FOR FDC COMMANDS EXECUTION

/*
01,n	jump to work n
02,n	call work n
03	return after 02,n

10	pause (com & 3)
11,n	pause (n)

20,n	set mode n
21,n,d	if (mode == n) jr d

30	drq = 0
31	drq = 1
32,d	if (drq == 0) jr d
33	dio = 0 (cpu -> fdc)
34	dio = 1 (fdc -> cpu)

40	wait for drq == 0 (cpu give Rdat)
41	wait for drq == 0 (cpu take Rdat)
42,n	Rdat = ? (3:S3)

50	get Rdat as HU
51	TP
52	TR
53	HD
54	SC
55	SZ
56	LS
57	GP
58	SL
59	FB
5a	NM
5b,d	if (HD != Rdat)
5c,d	if (SZ != Rdat)

60,a,o	S0 = S0 & a | o
61,a,o	S1 = S1 & a | o
62,a,o	S2 = S2 & a | o

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
AB,d	seek field 1 while ic spins; if found - jr d
AC,d	seek field (2|3) while ic spins; if found - jr d
AD	prev byte

B0,n	icnt = n
B1	icnt--
B2	icnt++
B3,n,d	if (icnt==n) jr d
B4,n,d	if (icnt!=n) jr d
B5	ic = buf[0]:128,256,512,1024

C0,a,b	flag & a | b
C1	fill fields @ curr.flp.trk
C2	next sector (uPD765)

D0	init CRC
D1,n	add byte n to CRC
D2	add Rdat to CRC (if field != 0)
D3	add Rbus to CRC (if field != 0)
D4	read flp.CRC (full)
D5,d	if (CRC == flp.CRC) jr d
D7	read fcrc.hi
D8	read fcrc.low

E0,n	n: 0 - stop motor/unload head; 1 - start motor/load head
E1,n	set status n
E2,d	if (side = 0) jr d

F0,d	jr d
F7,d	if (flp.DS) jr d
F8,d	if (flp.wprt) jr d
F9,d	if (flp.ready (insert)) jr d
FA,d	if (sdir = out (1)) jr d
FB,n,d	if (flp.field = n) jr d
FC,d	if (index) jr d
FD,n,d	if (com & n == 0) jr d
FE,n,d	if (com & n != 0) jr d
FF	end
*/

// VG93 (WD1793) FDC section

unsigned char vgidle[256] = {
	0xb0,15,
	0xa8,
	0xfc,2,
	0xf0,-5,
	0xb1,
	0xb4,0,-9,
	0xe0,0,
	0xf1
};

unsigned char vgwork[32][256] = {
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
		0x91,0xd2,0x88,0x31,0x41,0xa8,	// read byte, add to crc, Rsec = byte, drq=1, next(slow)
		0x91,0xd2,0x31,0x41,0xa8,	// read byte, add to crc, drq=1, next(slow)
		0x91,0xd2,0x31,0x41,0xa8,
		0x91,0xd2,0x31,0x41,0xa8,
		0x91,0xd7,0x31,0x41,0xa8,	// read byte, read fcrc.low, drq=1, next(slow)
		0x91,0xd8,0x31,0x41,	// read byte, read fcrc.hi, drq=1, next(slow)
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
		0x91,0x31,0x41,0xa8,	// Rdat = flpRd, drq=1, wait for cpu rd, next
		0xfc,2,0xf0,-8,		// ifn STRB jr -14 (@ 91)
		0xff			// END
	},
// 9: write track
	{
		0x20,1,			// set flag mode 1
		0xc0,0x83,0x00,		// res b2-6 in flag
		0xf9,1,0xff,		// ifn flp.ready END
		0xfd,4,2,0x11,4,	// if (b2,com) wait 15ms
		0xe0,1,
		0xf8,2,			// if write protect jr +2
		0xf0,4,			// jr +4 (@ 31)
		0xc0,0xbf,0x40,0xff,	// WRITE PROTECT, END
		0x31,			// drq = 1
		0xfc,4,			// if strb, jr @40 (skip 31, cuz drq is already 1)
		0xa8,			// next
		0xf0,-5,		// jr @fc
		0x31,			// drq = 1
		0x40,			// wait cpu wr
		0x93,			// write Rdat
		0xa8,			// next
		0xfc,2,0xf0,-8,		// if !strb jr @31
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
		0xab,4,			// wait for field 1 during IC spins, jr +4 on success
		0xc0,0xef,0x10,0xff,	// SEEK ERROR, end
		0x03			// return
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
		0x91,0xa8,		// read byte in Rdat, next (slow)
		0x8c,0xf8,11,		// if Rdat = F8 jr @(set b5,flag)
		0x8c,0xfb,13,		// if Rdat = FB jr @(res b5,flag)
		0xb1,0xb4,0,-12,	// F8 | FB must be in next 30(?) bytes
		0xc0,0xef,0x10,0xff,	// else: ARRAY NOT FOUND, END
		0xc0,0xbf,0x20,		// F8: set b5,flag
		0xf0,3,			// jr +3
		0xc0,0xbf,0x00,		// FB: res b5,flag
		0xd0,0xd2,		// init crc, CRC << Rdat (F8 or FB)
		0xb5,			// ic = sector len

		0x02,20,		// call READ_IC_BYTES
/*
		0x30,			// drq=0
		0x91,0xd2,0x31,0x41,	// read byte in Rdat, CRC << Rdat, drq=1, wait for cpu rd
		0xa8,			// next
		0xb1,			// ic--
		0xb4,0,-9,		// if ic!=0 jr -(@ 32)
*/
		0xd7,0xa8,0xd8,0xa8,	// read fcrc
		0xd5,4,			// if fcrc == crc jr +4
		0xc0,0xf7,0x08,0xff,	// set CRC ERROR, END
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
		0xa8,			// next (slow)
		0xb1,			// ic--
		0xb4,0,-12,		// if ic!=0 jr -11 (@ 92)
		0xc0,0xef,0x10,0xff,	// ARRAY NOT FOUND, END
		0x8f,0xf8,		// Rbus = F8
		0xfe,1,2,0x8f,0xfb,	// ifn (b0,com) Rbus = FB
		0xd0,0xd3,		// init crc, add Rbus (F8 | FB) to crc
		0x94,			// write Rbus (F8 or FB)
		0xa8,			// next (fast)
		0xb5,			// ic = sec.len
		0xf0,2,			// skip waiting 1st byte, cuz it already in Rdat
		0x31,0x40,		// drq=1, wait data from cpu (yes, it's lame, but...)
		0x93,0xd2,0xa8,		// write Rdat, add Rdat to CRC, flpNext
		0xb1,			// ic--
		0xb4,0,-9,		// if (ic != 0) jr @31
		0xd9,0xa8,0xda,		// write (crc.hi, crc.low) - crc
		0xfd,16,0xff,		// ifn (bit 4,com) END [multisector]
		0x8d,			// Rsec++
		0x01,15			// back to start (WORK 15)
	},
// 16 [CALL]: uPD765: result phase: S0 S1 S2 buf[0] buf[1] buf[2] buf[3]
	{
		0xe1,FDC_OUTPUT,
		0x34,			// result phase
		0x42,0,0x31,0x41,	// s0
		0x42,1,0x31,0x41,	// s1
		0x42,2,0x31,0x41,	// s2
		0x42,20,0x31,0x41,	// buf[0]: tr
		0x42,21,0x31,0x41,	// buf[1]: hd
		0x42,22,0x31,0x41,	// buf[2]: sc
		0x42,23,0x31,0x41,	// buf[3]: sz
		0x03
	},
// 17 [CALL]: uPD765: input phase: hu tr hd sc sz ls gp sl
	{
		0xe1,FDC_INPUT,
		0x33,
		0x31,0x40,0x50,	// HU
		0x31,0x40,0x52,	// TR
		0x31,0x40,0x53,	// HD
		0x31,0x40,0x54,	// SC
		0x31,0x40,0x55,	// SZ
		0x31,0x40,0x56,	// LS
		0x31,0x40,0x57,	// GP
		0x31,0x40,0x58,	// SL
		0x03
	},
// 18 [CALL]: uPD765: find sector SC data
	{
		0xb0,2,
		0xab,9,		// find AM (succes - @a8)
		0x60,0x3f,0x40,
		0x61,~FDC_ND,FDC_ND,
		0x20,FDC_OUTPUT,
		0x03,
		0xa8,		// skip TR
		0xa8,		// skip HD
		0x91,0xa8,	// Rdat = SC
		0x90,0,		// buf0 = SZ
		0x8a,-19,	// if Rdat != Rsec jr back @ab
		0xb5,		// if = sec.size(buf0)
		0xac,8,		// find DM (succes - @03)
		0x60,0x3f,0x40,
		0x61,~FDC_ND,FDC_ND,
		0x20,FDC_OUTPUT,
		0x03
	},
// 19 [CALL]: uPD765: read ic bytes fdc->cpu
	{
		0x34,			// fdc->cpu
		0x91,0xd2,		// Rdat = flpRd, CRC << Rdat
		0x31,0x41,0xa8,		// drq=1, wait for cpu rd, next
		0xb1,0xb4,0,-9,		// ic--; if (ic != 0) jr @91
		0x03			// ret
	},
// 20 [CALL]: WD1793: read ic bytes fdc->cpu (old version, with data lost)
	{
		0x30,			// drq=0
		0x32,5,			// if (drq=0) jr +5
		0xc0,0xfb,0x04,		// set DATA LOST
		0xf0,3,			// jr +3
		0x91,0xd2,0x31,		// read byte in Rdat, CRC << Rdat, drq=1
		0xa8,			// next
		0xb1,			// ic--
		0xb4,0,-15,		// if ic!=0 jr (@ 32)
		0x03			// ret
	},
// 21 [CALL]: uPD765 output phase (2)
	{
		0xe1,FDC_OUTPUT,
		0x34,
		0x42,0,0x31,0x41,	// sr0
		0x42,1,0x31,0x41,	// sr1
		0x42,2,0x31,0x41,	// sr2
		0x42,11,0x31,0x41,	// Rtrk
		0x42,12,0x31,0x41,	// Rsec
		0x42,13,0x31,0x41,	// side
		0x42,14,0x31,0x41,	// SZ
		0x03			// ret
	},
// 22 [CALL]: uPD765: write ic bytes cpu->fdc
	{
		0x33,		// cpu->fdc
		0x31,0x41,	// drq = 1; wait for drq = 0
		0xd2,		// CRC << Rdat
		0x93,0xa8,	// write Rdat, next
		0xb1,		// ic--
		0xb4,0,-8,	// if (ic != 0) jr @31
		0x03,
	}
};

// uPD765 FDC section
/*

MT  Bit7  Multi Track (continue multi-sector-function on other head)
MF  Bit6  MFM-Mode-Bit (Default 1=Double Density)
SK  Bit5  Skip-Bit (set if secs with deleted DAM shall be skipped)

HU  b0,1=Unit/Drive Number, b2=Physical Head Number, other bits zero
TP  Physical Track Number
TR  Track-ID (usually same value as TP)
HD  Head-ID
SC  First Sector-ID (sector you want to read)
SZ  Sector Size (80h shl n) (default=02h for 200h bytes)
LS  Last Sector-ID (should be same as SC when reading a single sector)
GP  Gap (default=2Ah except command 0D: default=52h)
SL  Sectorlen if SZ=0 (default=FFh)
Sn  Status Register 0..3
FB  Fillbyte (for the sector data areas) (default=E5h)
NM  Number of Sectors (default=09h)
XX  b0..3=headunload n*32ms (8" only), b4..7=steprate (16-n)*2ms
YY  b0=DMA_disable, b1-7=headload n*4ms (8" only)
*/

// XX	-	80	invalid operation
unsigned char	op765_no[] = {
	0xe1,FDC_EXEC,
	0x60,0x00,0x80,	// sr0 = 0x80
	0xe1,FDC_OUTPUT,
	0x34,		// fdc->cpu
	0x42,0,		// data = sr0
	0x31,0x41,	// wait for data reading
	0xf1
};

// 02+MF+SK    HU TR HD ?? SZ NM GP SL <R> S0 S1 S2 TR HD NM SZ read track
unsigned char op765_02[] = {
	0xe1,FDC_INPUT,
	0x33,
	0x31,0x40,0x50,	// HU
	0x31,0x40,0x52,	// TR
	0x31,0x40,0x53,	// HD
	0x31,0x40,	// ??
	0x31,0x40,0x55,	// SZ
	0x31,0x40,0x5A,	// NM
	0x31,0x40,0x57,	// GP
	0x31,0x40,0x58,	// SL
	0xe1,FDC_READ,
	0xf9,8,			// if flp.ready
	0x60,0x37,0x48,		// s0 |= 48 (drive not ready)
	0xf0,13,		// @e1 (@output)
	0xfc,3,0xa8,0xf0,-5,	// wait for IDX
	0x91,0x31,0x41,0xa8,	// Rdat = flpRd, drq=1, wait for rd, next
	0xfc,2,0xf0,-8,		// if !IDX jr @91
	0xe1,FDC_OUTPUT,
	0x42,0,0x31,0x41,	// sr0
	0x42,1,0x31,0x41,	// sr1
	0x42,2,0x31,0x41,	// sr2
	0x42,11,0x31,0x41,	// Rtrk
	0x42,13,0x31,0x41,	// side
	0x42,15,0x31,0x41,	// nm
	0x42,16,0x31,0x41,	// sl
	0xe1,FDC_IDLE,
	0xf1
};

// 03          XX YY                    -                       specify spd/dma
unsigned char op765_03[] = {
	0xe1,FDC_INPUT,	// status = input
	0x33,		// dir = cpu->fdc
	0x31,0x40,
	0x31,0x40,
	0xe1,FDC_EXEC,	// status = exec
	0xe1,FDC_IDLE,
	0xf1		// end
};

// 04          HU                       -  S3                   sense drive state
unsigned char op765_04[] = {
	0xe1,FDC_INPUT,
	0x33,		// cpu->fdc
	0x31,0x40,0x50,	// HU
	0xe1,FDC_EXEC,
	0x42,3,		// data = s3 (+build s3)
	0x34,		// fdc->cpu
	0x31,		// drq = 1
	0x41,		// wait for reading
	0xe1,FDC_IDLE,
	0xf1		// end
};

// 05+MT+MF    HU TR HD SC SZ LS GP SL <W> S0 S1 S2 TR HD LS SZ write sector(s)
unsigned char op765_05[] = {
	0x02,17,	// hu tr hd sc sz ls gp sl
	0xe1,FDC_WRITE,
	0x60,0x00,0x00,		// sr0 = 0
	0x61,0x00,0x00,		// sr1 = 0
	0x62,0x00,0x00,		// sr2 = 0
	0xf9,8,16,		// if drive is ready @f8
	0x60,0x37,0x48,		// s0 |= 0x48 (drive not ready)
	0x02,21,		// result phase
	0xe1,FDC_IDLE,
	0xf1,
	0x60,0x00,0x40,		// WPRT ERROR
	0x61,0x00,0x02,
	0xf0,-13,
	0xf8,-10,		// if write protect, jr back WPRT ERROR

	0x02,18,		// find sector SN (ic = sec.len)
	0x21,FDC_OUTPUT,-20,	// if error jr @02,21 (result phase)
	0xd0,			// init CRC
	0xad,			// prev byte
	0x8f,0xfb,0x94,0xa8,	// write FB (data mark)
	0xd1,0xfb,		// CRC << FB
	0x02,22,		// write IC bytes
	0xd9,0xa8,0xda,		// write CRC
	0xc2,			// next sector
	0x21,FDC_EXEC,-22,	// if still exec jr @02,18

	0x02,21,
	0xe1,FDC_IDLE,
	0xf1
};

// 06+MT+MF+SK HU TR HD SC SZ LS GP SL <R> S0 S1 S2 TR HD LS SZ read sector(s)
unsigned char op765_06[] = {
	0x02,17,	// hu tr hd sc sz ls gp sl
	0xe1,FDC_READ,
	0x60,0x00,0x00,		// sr0 = 0
	0x61,0x00,0x00,		// sr1 = 0
	0x62,0x00,0x00,		// sr2 = 0
	0x8f,0,			// Rbus = 0 (counter of EOT)
	0xf9,8,			// if drive is ready @02,18
	0x60,0x37,0x48,		// s0 |= 48 (drive not ready)
	0x02,16,
	0xe1,FDC_IDLE,
	0xf1,

	0x02,18,		// find sector SN (ic = sec.len)
	0x21,FDC_OUTPUT,20,	// if error - jr to output
	0xfb,2,11,		// if field = 2 (normal sec) jr @b5
	0xfd,FDC_SK,4,		// if !com:SK jr @02
	0xc2,			// com:SK & del.sec - next sector
	0x21,FDC_READ,-15,	// if still exec, jr back @02
	// sec:del + !com:SK
	0x02,19,		// ic bytes fdd->cpu
	0xf0,6,
	// sec:norm
	0x02,19,		// ic bytes fdd->cpu
	0xc2,			// next sector
	0x21,FDC_EXEC,-25,	// if still exec jr @02
	// result phase
	0x02,21,
	0xe1,FDC_IDLE,
	0xf1
};

// 07          HU                       -                       recalib.seek TP=0
unsigned char op765_07[] = {
	0xe1,FDC_INPUT,
	0x33,
	0x31,0x40,0x50,	// HU
	0xe1,FDC_EXEC,
	0x60,0x00,0x00,		// sr0 = 0
	0x61,0x00,0x00,		// sr1 = 0
	0x62,0x00,0x00,		// sr2 = 0
	0xb0,77,	// ic = 77
	0xb3,0,15,	// if (ic == 0)
	0x83,0,4,	// if (flp.trk == 0)
	0xa0,		// step in
	0xb1,		// ic--
	0xf0,-10,	// jr @b3
	0x60,0x00,FDC_SE,	// sr0: SE, IC=00 (success)
	0xe1,FDC_IDLE,
	0x20,4,
	0xf1,
	0x60,0x00,FDC_SE | FDC_EC | 0x40,	// sr0: SE, EC, IC=01 (errors)
	0xe1,FDC_IDLE,
	0x20,4,		// mode = 4
	0xf1
};

// 08          -                        -  S0 TP                sense int.state
unsigned char op765_08[] = {
//	0xe1,FDC_EXEC,			// remove status change 'cuz it affect mode
//	0xe1,FDC_OUTPUT,
	0x21,4,13,	// if int.mode = 4 (seek, recalib) @34
	0x60,0x00,0x80,	// st0 = 0x80
	0x34,		// fdc->cpu
	0x42,0,		// data = st0
	0x31,0x41,	// wait reading
	0xe1,FDC_IDLE,	// end
	0x20,0,		// mode = 0
	0xf1,
	0x34,
	0x42,0,		// Rdat = sr0
	0x31,0x41,	// wait for reading
	0x42,11,	// Rdat = Rtrk
	0x31,0x41,	// wait for reading
	0xe1,FDC_IDLE,
	0x20,0,		// mode = 0
	0xf1
};

// 09+MT+MF    HU TR HD SC SZ LS GP SL <W> S0 S1 S2 TR HD LS SZ wr deleted sec(s)
unsigned char op765_09[] = {
	0x02,17,	// hu tr hd sc sz ls gp sl
	0xe1,FDC_EXEC,
	0x60,0,0x40,	// error
	0x61,0,2,	// write protect
	0x62,0,0,
	0xe1,FDC_OUTPUT,
	0x42,0,0x31,0x41,	// sr0
	0x42,1,0x31,0x41,	// sr1
	0x42,2,0x31,0x41,	// sr2
	0x42,11,0x31,0x41,	// Rtrk
	0x42,12,0x31,0x41,	// Rsec
	0x42,13,0x31,0x41,	// side
	0x42,14,0x31,0x41,	// SZ
	0xe1,FDC_IDLE,
	0xf1
};

// 0A+MF       HU                       -  S0 S1 S2 TR HD LS SZ		 read ID
unsigned char op765_0A[] = {
	0xe1,FDC_INPUT,
	0x33,
	0x31,0x40,0x50,	// HU
	0xe1,FDC_EXEC,
	0x60,0,0,		// sr0 = sr1 = sr2 = 0
	0x61,0,0,
	0x62,0,0,
	0xf9,8,			// if drive is ready @b0
	0x60,0x37,0x48,		// s0 |= 48 (drive not ready)
	0x02,16,
	0xe1,FDC_IDLE,
	0xf1,
	0xb0,3,			// ic = 2
	0xab,11,		// seek address mark, jr @90 if success
	0x60,0x3f,0x40,		// st0: IC = 40
	0x61,~FDC_ND,FDC_ND,	// st1: ND (no data found)
	0x02,16,		// s0 s1 s2 b0 b1 b2 b3
	0xe1,FDC_IDLE,
	0xf1,
	0x90,0,0xa8,		// read 4 bytes to buf
	0x90,1,0xa8,
	0x90,2,0xa8,
	0x90,3,0xa8,
	0x02,16,		// s0 s1 s2 b0 b1 b2 b3
	0xe1,FDC_IDLE,
	0xf1
};

// 0C+MT+MF+SK HU TR HD SC SZ LS GP SL <R> S0 S1 S2 TR HD LS SZ rd deleted sec(s)
unsigned char op765_0C[] = {
	0x02,17,	// hu tr hd sc sz ls gp sl
	0xe1,FDC_READ,
	0x60,0x00,0x00,		// sr0 = 0
	0x61,0x00,0x00,		// sr1 = 0
	0x62,0x00,0x00,		// sr2 = 0
	0x8f,0,			// Rbus = 0 (counter of EOT)
	0xf9,8,			// if drive is ready @02,18
	0x60,0x37,0x48,		// s0 |= 48 (drive not ready)
	0x02,16,
	0xe1,FDC_IDLE,
	0xf1,

	0x02,18,		// find sector SN (ic = sec.len)
	0x21,FDC_OUTPUT,20,	// if error - jr to output
	0xfb,3,11,		// if field = 3 (deleted sec) jr @b5
	0xfd,FDC_SK,4,		// if !com:SK jr @02
	0xc2,			// com:SK & del.sec - next sector
	0x21,FDC_READ,-15,	// if still exec, jr back @02
	// sec:del + !com:SK
	0x02,19,		// ic bytes fdd->cpu
	0xf0,6,
	// sec:del
	0x02,19,		// ic bytes fdd->cpu
	0xc2,			// next sector
	0x21,FDC_EXEC,-25,	// if still exec jr @02
	// result phase
	0xe1,FDC_OUTPUT,
	0x34,
	0x42,0,0x31,0x41,	// s0
	0x42,1,0x31,0x41,	// s1
	0x42,2,0x31,0x41,	// s2
	0x42,11,0x31,0x41,	// Rtrk
	0x42,12,0x31,0x41,	// Rsec
	0x42,13,0x31,0x41,	// side
	0x42,14,0x31,0x41,	// SZ
	0xe1,FDC_IDLE,
	0xf1
};

// 0D+MF       HU SZ NM GP FB          <W> S0 S1 S2 TR HD LS SZ format track
unsigned char op765_0D[] = {
	0xe1,FDC_INPUT,
	0x33,
	0x31,0x40,0x50,	// HU
	0x31,0x40,0x55,	// SZ
	0x31,0x40,0x5a,	// NM
	0x31,0x40,0x57,	// GP
	0x31,0x40,0x59,	// FB
	0xe1,FDC_EXEC,
	0x60,0,0x40,	// error
	0x61,0,2,	// write protect
	0x62,0,0,
	0xe1,FDC_OUTPUT,
	0x42,0,0x31,0x41,	// sr0
	0x42,1,0x31,0x41,	// sr1
	0x42,2,0x31,0x41,	// sr2
	0x42,11,0x31,0x41,	// Rtrk
	0x42,12,0x31,0x41,	// Rsec
	0x42,13,0x31,0x41,	// side
	0x42,14,0x31,0x41,	// SZ
	0xe1,FDC_IDLE,
	0xf1
};

// 0F          HU TP                    -                       seek track n
unsigned char op765_0F[] = {
	0xe1,FDC_INPUT,
	0x60,0x00,0x03,	// all drives is busy
	0x33,
	0x31,0x40,0x50,	// HU
	0x31,0x40,	// TP (Rdat)
	0xe1,FDC_EXEC,
	0x60,0x00,0x00,	// all drives is not-busy
	0x42,10,	// Rtrk = flp::trk
	0x86,10,	// if (Rtr = Rdat)
	0x87,4,		// if (Rtr < Rdat)
	0x81,		// Rtr--
	0xa0,		// step in
	0xf0,-8,	// @86
	0x82,		// Rtr++
	0xa1,		// step out
	0xf0,-6,	// @f0
	0x60,0x00,FDC_SE,	// SE,IC=0 (success)
	0xe1,FDC_IDLE,
	0x20,4,		// mode = 4
	0xf1
};

// 11+MT+MF+SK HU TR HD SC SZ LS GP SL <W> S0 S1 S2 TR HD LS SZ scan equal
unsigned char op765_11[] = {
	0x02,17,	// hu tr hd sc sz ls gp sl
	0xe1,FDC_EXEC,
	0xe1,FDC_IDLE,
	0xf1
};

// 19+MT+MF+SK HU TR HD SC SZ LS GP SL <W> S0 S1 S2 TR HD LS SZ scan low or equal
unsigned char op765_19[] = {
	0x02,17,	// hu tr hd sc sz ls gp sl
	0xe1,FDC_EXEC,
	0xe1,FDC_IDLE,
	0xf1
};

// 1D+MT+MF+SK HU TR HD SC SZ LS GP SL <W> S0 S1 S2 TR HD LS SZ scan high or equal
unsigned char op765_1D[] = {
	0x02,17,	// hu tr hd sc sz ls gp sl
	0xe1,FDC_EXEC,
	0xe1,FDC_IDLE,
	0xf1
};

typedef unsigned char* f765wptr;
f765wptr fdc765workTab[32] = {
	op765_no,op765_no,op765_02,op765_03,op765_04,op765_05,op765_06,op765_07,
	op765_08,op765_09,op765_0A,op765_no,op765_0C,op765_0D,op765_no,op765_0F,
	op765_no,op765_11,op765_no,op765_no,op765_no,op765_no,op765_no,op765_no,
	op765_no,op765_19,op765_no,op765_no,op765_no,op765_1D,op765_no,op765_no
};

void fdcExec(FDC* fdc, unsigned char val) {
	switch (fdc->type) {
		case FDC_93:
			if (fdc->mr == 0) break;			// no commands aviable during master reset
			if (fdc->idle) {
				fdc->com = val;
				fdc->wptr = NULL;
				if ((val & 0xf0) == 0x00) fdc->wptr = vgwork[0];	// restore		00..0f
				if ((val & 0xf0) == 0x10) fdc->wptr = vgwork[1];	// seek			10..1f
				if ((val & 0xe0) == 0x20) fdc->wptr = vgwork[2];	// step			20..3f
				if ((val & 0xe0) == 0x40) fdc->wptr = vgwork[3];	// step in		40..5f
				if ((val & 0xe0) == 0x60) fdc->wptr = vgwork[4];	// step out		60..7f
				if ((val & 0xe1) == 0x80) {
					fdc->wptr = vgwork[5];	// read sector
					fdc->status = FDC_READ;
				}
				if ((val & 0xe0) == 0xa0) {
					fdc->wptr = vgwork[6];	// write sector
					fdc->status = FDC_WRITE;
				}
				if ((val & 0xfb) == 0xc0) {
					fdc->wptr = vgwork[7];	// read address
					fdc->status = FDC_READ;
				}
				if ((val & 0xfb) == 0xe0) {
					fdc->wptr = vgwork[8];	// read track
					fdc->status = FDC_READ;
				}
				if ((val & 0xfb) == 0xf0) {
					fdc->wptr = vgwork[9];	// write track
					fdc->status = FDC_WRITE;
				}
				if (fdc->wptr == NULL) fdc->wptr = vgwork[11];
				fdc->count = -1;
				fdc->idle = 0;
				fdc->irq = 0;
				fdc->drq = 0;
				fdc->flag = 0;
			}
			if ((val & 0xf0) == 0xd0) {
				fdc->wptr = vgwork[10];		// interrupt
				fdc->count = -1;
				fdc->idle = 0;
				fdc->irq = 0;
				fdc->drq = 0;
				fdc->flag = 0;
			}
			break;
		case FDC_765:
//			printf("DEBUG: uPD765 exec %.2X\n",val);
			fdc->wptr = fdc765workTab[val & 0x1f];
			fdc->idle = 0;
			fdc->irq = 0;
			fdc->drq = 0;
			fdc->count = -1;
			fdc->com = val;
			break;
	}
}

void fdcSetMr(FDC* fdc,int z) {
	if (z == 0) {
		fdc->mr = z;
		fdcExec(fdc,0x03);	// restore
		fdc->idle = 1;
		fdc->status = FDC_IDLE;
		fdc->sec = 1;
	} else {
		fdc->mr = z;
	}
}

unsigned char fdcRd(FDC* fdc,int port) {
	unsigned char res = 0xff;
	switch (fdc->type) {
		case FDC_93:
			switch(port) {
				case FDC_STATE:
					res = ((fdc->fptr->flag & FLP_INSERT) ? 0 : 0x80) | ((fdc->idle == 0) ? 1 : 0);
					switch (fdc->mode) {
						case 0:
							res |= ((fdc->fptr->flag & FLP_PROTECT) ? 0x40 : 0)\
								| 0x20\
								| (fdc->flag & 0x18)\
								| ((fdc->fptr->trk == 0) ? 4 : 0)\
								| (fdc->idx ? 2 : 0);
							break;
						case 1:
						case 2:
							res |= (fdc->flag & 0x7c) | (fdc->drq ? 2 : 0);
							break;
					}
					fdc->irq = 0;
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
					break;
			}
			break;
		case FDC_765:
			switch (port) {
				case FDC_STATE:
					if (fdc->idle) {
						res = FDC_RQM;
					} else {
						res = (fdc->drq ? FDC_RQM : 0) | \
							(fdc->ioDir ? FDC_DIO : 0) | \
							(((fdc->status == FDC_EXEC) || (fdc->status == FDC_READ) || (fdc->status == FDC_WRITE)) ? FDC_EXM : 0) | \
							((fdc->idle) ? 0 : FDC_BSY);
					}
//					printf("state: %.2X\n",res);
					break;
				case FDC_DATA:
					if (fdc->ioDir && fdc->drq) {
						res = fdc->data;
						fdc->drq = 0;
					} else {
						res = 0xff;
					}
			}
			break;
	}
	return res;
}

void fdcWr(FDC* fdc,int port,unsigned char val) {
	switch (fdc->type) {
		case FDC_93:
			switch (port) {
				case FDC_COM:
					fdcExec(fdc,val);
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
			break;
		case FDC_765:
			switch (port) {
				case FDC_DATA:
					switch (fdc->status) {
						case FDC_IDLE:
							fdc->com = val;
							fdcExec(fdc,val);
							break;
						case FDC_INPUT:
						case FDC_EXEC:
							if (fdc->drq & !fdc->ioDir) {
								fdc->data = val;
								fdc->drq = 0;
							}
							break;
					}

					break;
			}

			break;
	}
}

void fdcAddCrc(FDC* fdc,unsigned char val) {
	unsigned int tkk = fdc->crc;
	int i;
	tkk ^= val << 8;
	for (i = 8; i; i--) {
		if ((tkk *= 2) & 0x10000) tkk ^= 0x1021;
	}
	fdc->crc = tkk & 0xffff;
}

unsigned char p1,p2,dlt;
int delays[6]={6 * MSDELAY,12 * MSDELAY,24 * MSDELAY,32 * MSDELAY,15 * MSDELAY, 50 * MSDELAY};	// 6, 12, 20, 32, 15, 50ms (hlt-hld)

void v01(FDC* p) {p1 = *(p->wptr++); p->wptr = vgwork[p1];}
void v02(FDC* p) {p1 = *(p->wptr++); p->sp = p->wptr; p->wptr = vgwork[p1];}
void v03(FDC* p) {p->wptr = p->sp;}

void v10(FDC* p) {p->count += (fdcFlag & FDC_FAST) ? TRBDELAY : delays[p->com & 3];}
void v11(FDC* p) {p1 = *(p->wptr++); p->count += (fdcFlag & FDC_FAST) ? TRBDELAY : delays[p1];}

void v20(FDC* p) {p->mode = *(p->wptr++);}
void v21(FDC* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->mode == p1) p->wptr += (char)dlt;}

void v30(FDC* p) {p->drq = 0;}
void v31(FDC* p) {p->drq = 1;}
void v32(FDC* p) {dlt = *(p->wptr++); if (!p->drq) p->wptr += (signed char)dlt;}
void v33(FDC* p) {p->ioDir = 0;}
void v34(FDC* p) {p->ioDir = 1;}

void v40(FDC* p) {if (p->drq) {p->wptr--;p->count = 0;}}
void v41(FDC* p) {if (p->drq) {p->wptr--;p->count = 0;}}
void v42(FDC *p) {
	p1 = *(p->wptr++);
	switch (p1) {
		case 0: p->data = p->sr0; break;
		case 1: p->data = p->sr1; break;
		case 2: p->data = p->sr2; break;
		case 3: p->data = (p->fptr->id & 3) |\
				((p->side) ? FDC_HD : 0) |\
				((p->fptr->flag & FLP_DS) ? 0 : FDC_DS) |\
				((p->fptr->trk == 0) ? FDC_T0 : 0) |\
				((p->fptr->flag & FLP_INSERT) ? FDC_RY : 0) |\
				((p->fptr->flag & FLP_PROTECT) ? FDC_WP : 0);
			break;
		case 10: p->trk = p->fptr->trk; break;
		case 11: p->data = p->trk; break;
		case 12: p->data = p->sec; break;
		case 13: p->data = p->side; break;
		case 14: p->data = p->sz; break;
		case 15: p->data = p->nm; break;
		case 16: p->data = p->sl; break;
		case 20: p->data = p->buf[0]; break;
		case 21: p->data = p->buf[1]; break;
		case 22: p->data = p->buf[2]; break;
		case 23: p->data = p->buf[3]; break;
		default:
			printf("FDC 42,%.2X is unknown\n",p1);
			break;
	}
}

void v50(FDC* p) {
	p->fptr = p->flop[p->data & 3];
	p->side = (p->data & 4) ? 0 : 1;
	p->sr0 &= 0xf8;
	p->sr0 |= (p->data & 7);
//	printf("unit %i, side %i\n",p->data & 3, p->side);
}	// HU: b0,1:drive; b2: side
void v51(FDC* p) {p->trk = p->data;}							// TP: physical track
void v52(FDC* p) {p->trk = p->data;}							// TR: track id
void v53(FDC* p) {p->hd = p->data;}							// HD: head id
void v54(FDC* p) {p->sec = p->data;}							// SC: first sector
void v55(FDC* p) {p->sz = p->data;}							// SZ: sector size code
void v56(FDC* p) {p->ls = p->data;}							// LS: last sector
void v57(FDC* p) {p->gp = p->data;}							// GP: gap
void v58(FDC* p) {p->sl = p->data;}							// SL: sector len if SZ = 0
void v59(FDC* p) {p->fb = p->data;}							// FB: fill byte
void v5A(FDC* p) {p->nm = p->data;}							// NM: number of sectors
void v5B(FDC* p) {dlt = *(p->wptr++); if (p->hd != p->data) p->wptr += (char)dlt;}	// if Rdat != HD jr
void v5C(FDC* p) {dlt = *(p->wptr++); if (p->sz != p->data) p->wptr += (char)dlt;}	// if Rdat != SZ jr

void v60(FDC* p) {p1 = *(p->wptr++); p2 = *(p->wptr++); p->sr0 &= p1; p->sr0 |= p2;}
void v61(FDC* p) {p1 = *(p->wptr++); p2 = *(p->wptr++); p->sr1 &= p1; p->sr1 |= p2;}
void v62(FDC* p) {p1 = *(p->wptr++); p2 = *(p->wptr++); p->sr2 &= p1; p->sr2 |= p2;}

void v80(FDC* p) {p->trk = *(p->wptr++);}
void v81(FDC* p) {p->trk--;}
void v82(FDC* p) {p->trk++;}
void v83(FDC* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->fptr->trk == p1) p->wptr += (signed char)dlt;}
void v84(FDC* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->trk == p1) p->wptr += (signed char)dlt;}
void v85(FDC* p) {dlt = *(p->wptr++); if (p->buf[0] == p->trk) p->wptr += (signed char)dlt;}
void v86(FDC* p) {dlt = *(p->wptr++); if (p->trk == p->data) p->wptr += (signed char)dlt;}
void v87(FDC* p) {dlt = *(p->wptr++); if (p->trk < p->data) p->wptr += (signed char)dlt;}
void v88(FDC* p) {p->sec = p->data;}
void v89(FDC* p) {dlt = *(p->wptr++); if (p->trk != p->data) p->wptr += (signed char)dlt;}
void v8A(FDC* p) {dlt = *(p->wptr++); if (p->sec != p->data) p->wptr += (signed char)dlt;}
void v8B(FDC* p) {dlt = *(p->wptr++); if (((p->com & 8)?0:1) != p->data) p->wptr += (signed char)dlt;}
void v8C(FDC* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->data == p1) p->wptr += (signed char)dlt;}
void v8D(FDC* p) {p->sec++;}
void v8E(FDC* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->bus == p1) p->wptr += (signed char)dlt;}
void v8F(FDC* p) {p->bus = *(p->wptr++);}

void v90(FDC* p) {p1 = *(p->wptr++); p->buf[p1] = flpRd(p->fptr); p->bus = p->buf[p1];}
void v91(FDC* p) {p->data = flpRd(p->fptr);}
void v92(FDC* p) {p->bus = flpRd(p->fptr);}
void v93(FDC* p) {flpWr(p->fptr,p->data);}
void v94(FDC* p) {flpWr(p->fptr,p->bus);}

void vA0(FDC* p) {p->sdir = FLP_BACK; flpStep(p->fptr,p->sdir);}
void vA1(FDC* p) {p->sdir = FLP_FORWARD; flpStep(p->fptr,p->sdir);}
void vA2(FDC* p) {flpStep(p->fptr,p->sdir);}

void vA8(FDC* p) {
	p->t += BYTEDELAY;
	p->strb = 0;
	if (flpNext(p->fptr,p->side)) {p->t = 0; p->strb = 1;}
	p->count += p->tf;
}
void vA9(FDC* p) {
	p->t += BYTEDELAY;
	p->strb = 0;
	if (flpNext(p->fptr,p->side)) {p->t = 0; p->strb = 1;}
	p->count += p->tf;
}
void vAA(FDC* p) {p->side = !p->side;}
void vAB(FDC* p) {
	dlt = *(p->wptr++);
	while (p->fptr->field != 0) {
		if (flpNext(p->fptr,p->side)) {p->ic--; p->t = 0;}
		if (~fdcFlag & FDC_FAST) p->count += BYTEDELAY;
		if (p->ic == 0) return;
	}
	while (p->fptr->field != 1) {
		if (flpNext(p->fptr,p->side)) {p->ic--; p->t = 0;}
		if (~fdcFlag & FDC_FAST) p->count += BYTEDELAY;
		if (p->ic == 0) return;
	}
	p->wptr += (char)dlt;	// success
}
void vAC(FDC* p) {
	dlt = *(p->wptr++);
	while (p->fptr->field != 0) {
		if (flpNext(p->fptr,p->side)) {p->ic--; p->t = 0;}
		if (~fdcFlag & FDC_FAST) p->count += BYTEDELAY;
		if (p->ic == 0) return;
	}
	while ((p->fptr->field != 2) && (p->fptr->field != 3)) {
		if (flpNext(p->fptr,p->side)) {p->ic--; p->t = 0;}
		if (~fdcFlag & FDC_FAST) p->count += BYTEDELAY;
		if (p->ic == 0) return;
	}
	p->wptr += (char)dlt;	// success
}
void vAD(FDC* p) {flpPrev(p->fptr,p->side);}

void vB0(FDC* p) {p->ic = *(p->wptr++);}
void vB1(FDC* p) {p->ic--;}
void vB2(FDC* p) {p->ic++;}
void vB3(FDC* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->ic == p1) p->wptr += (signed char)dlt;}
void vB4(FDC* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->ic != p1) p->wptr += (signed char)dlt;}
void vB5(FDC* p) {p->ic = (128 << (p->buf[0] & 3));}				// 128,256,512,1024

void vC0(FDC* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); p->flag &= p1; p->flag |= dlt;}
void vC1(FDC* p) {flpFillFields(p->fptr,p->fptr->rtrk,1);}
void vC2(FDC* p) {
	if (p->sec < p->ls) {		// if SC < EOT
		p->sec++;
		p->sr1 &= ~FDC_EN;
	} else {
		p->sr1 |= FDC_EN;
		p->sec = 1;
		if (p->com & FDC_MT) {
			p->side = !p->side;
			p->bus++;
			if (!p->side) p->trk++;
			if (p->bus > 1) p->mode = FDC_BREAK;
		} else {
			p->trk++;
			p->mode = FDC_BREAK;
		}
	}
}

void vD0(FDC* p) {p->crc = 0xcdb4; p->crchi = 1;}
void vD1(FDC* p) {fdcAddCrc(p,*(p->wptr++));}
void vD2(FDC* p) {fdcAddCrc(p,p->data);}
void vD3(FDC* p) {fdcAddCrc(p,p->bus);}
void vD4(FDC* p) {
	p->fcrc = flpRd(p->fptr);
	if (flpNext(p->fptr,p->side)) p->t = 0;	// if (flpNext(p->fptr,p->side)) p->ti = p->t;
	p->fcrc |= (flpRd(p->fptr) << 8);
}	// read crc from floppy
void vD5(FDC* p) {
	dlt = *(p->wptr++); if (p->crc == p->fcrc) p->wptr += (signed char)dlt;
}
void vD7(FDC* p) {p->fcrc = (flpRd(p->fptr) << 8);}
void vD8(FDC* p) {p->fcrc |= flpRd(p->fptr);}
void vD9(FDC* p) {flpWr(p->fptr,(p->crc & 0xff00) >> 8);}
void vDA(FDC* p) {flpWr(p->fptr,p->crc & 0xff);}
void vDF(FDC* p) {if (p->crchi) {
			flpWr(p->fptr,(p->crc & 0xff00) >> 8);
		} else {
			flpWr(p->fptr,p->crc & 0xff);
		}
		p->crchi = !p->crchi;
}

void vE0(FDC* p) {
	p1 = *(p->wptr++);
	if (p1 == 0) {
		p->fptr->flag &= ~(FLP_MOTOR | FLP_HEAD);
	} else {
		if (!(p->fptr->flag & FLP_HEAD)) p->count += (fdcFlag & FDC_FAST) ? TRBDELAY : delays[5];
		p->fptr->flag |= (FLP_MOTOR | FLP_HEAD);
	}
}
void vE1(FDC* p) {
	p->status = *(p->wptr++);
	switch(p->status) {
		case FDC_READ:
		case FDC_WRITE:
		case FDC_EXEC: p->mode = 2; break;
		case FDC_OUTPUT: p->mode = 1; break;
		case FDC_IDLE: if (p->mode < 3) p->mode = 0; break;
	}
}
void vE2(FDC* p) {dlt = *(p->wptr++); if (p->side == 0) p->wptr += (char)dlt;}

void vF0(FDC* p) {dlt = *(p->wptr++); p->wptr += (signed char)dlt;}
void vF1(FDC* p) {p->wptr = NULL; p->irq = 1; p->idle = 1; p->drq = 0; p->count = 0; p->status = FDC_IDLE;}
void vF7(FDC* p) {dlt = *(p->wptr++); if (p->fptr->flag & FLP_DS) p->wptr += (char)dlt;}	// DS
void vF8(FDC* p) {dlt = *(p->wptr++); if (p->fptr->flag & FLP_PROTECT) p->wptr += (signed char)dlt;}	// PROTECT
void vF9(FDC* p) {dlt = *(p->wptr++); if (p->fptr->flag & FLP_INSERT) p->wptr += (signed char)dlt;}	// INSERT
void vFA(FDC* p) {dlt = *(p->wptr++); if (p->sdir) p->wptr += (signed char)dlt;}
void vFB(FDC* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if (p->fptr->field == p1) p->wptr += (signed char)dlt;}
void vFC(FDC* p) {dlt = *(p->wptr++); if (p->strb) p->wptr += (signed char)dlt;}
void vFD(FDC* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if ((p->com & p1) == 0) p->wptr += (signed char)dlt;}
void vFE(FDC* p) {p1 = *(p->wptr++); dlt = *(p->wptr++); if ((p->com & p1) != 0) p->wptr += (signed char)dlt;}
void vFF(FDC* p) {p->irq = 1; p->idle = 1; p->wptr = &vgidle[0]; p->status = FDC_IDLE;}

typedef void(*VGOp)(FDC*);
VGOp vgfunc[256] = {
	NULL,&v01,&v02,&v03,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&v10,&v11,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&v20,&v21,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&v30,&v31,&v32,&v33,&v34,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&v40,&v41,&v42,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&v50,&v51,&v52,&v53,&v54,&v55,&v56,&v57,&v58,&v59,&v5A,&v5B,&v5C,NULL,NULL,NULL,
	&v60,&v61,&v62,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&v80,&v81,&v82,&v83,&v84,&v85,&v86,&v87,&v88,&v89,&v8A,&v8B,&v8C,&v8D,&v8E,&v8F,
	&v90,&v91,&v92,&v93,&v94,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&vA0,&vA1,&vA2,NULL,NULL,NULL,NULL,NULL,&vA8,&vA9,&vAA,&vAB,&vAC,&vAD,NULL,NULL,
	&vB0,&vB1,&vB2,&vB3,&vB4,&vB5,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&vC0,&vC1,&vC2,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&vD0,&vD1,&vD2,&vD3,&vD4,&vD5,NULL,&vD7,&vD8,&vD9,&vDA,NULL,NULL,NULL,NULL,&vDF,
	&vE0,&vE1,&vE2,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	&vF0,&vF1,NULL,NULL,NULL,NULL,NULL,&vF7,&vF8,&vF9,&vFA,&vFB,&vFC,&vFD,&vFE,&vFF
};

// BDI

BDI* bdiCreate() {
	BDI* bdi = (BDI*)malloc(sizeof(BDI));
	bdi->fdc = fdcCreate(FDC_93);
	bdi->flag = 0;

	return bdi;
}

void bdiDestroy(BDI* bdi) {
	fdcDestroy(bdi->fdc);
	free(bdi);
}

void bdiReset(BDI* bdi) {
	bdi->fdc->count = 0;
	bdiOut(bdi,BDI_SYS,0);
}

int bdiGetPort(int p) {
	int port = p;
	pcatch = 0;
	if ((p & 0x03)==0x03) {
		if ((p & 0x82) == 0x82) port=BDI_SYS;
		if ((p & 0xe2) == 0x02) port=FDC_COM;
		if ((p & 0xe2) == 0x22) port=FDC_TRK;
		if ((p & 0xe2) == 0x42) port=FDC_SEC;
		if ((p & 0xe2) == 0x62) port=FDC_DATA;
		pcatch = 1;
	}
	return port;
}

int bdiOut(BDI* bdi,int port,unsigned char val) {
//	if ((port == FDC_COM) || (port == BDI_SYS)) printf("bdiOut(%.4X,%.2X)\n",port,val);
	switch (port) {
		case FDC_COM:
		case FDC_TRK:
		case FDC_SEC:
		case FDC_DATA:
			fdcWr(bdi->fdc,port,val);
			break;
		case BDI_SYS:
			bdi->fdc->fptr = bdi->fdc->flop[val & 0x03];	// selet floppy
			fdcSetMr(bdi->fdc,(val & 0x04) ? 1 : 0);		// master reset
			bdi->fdc->block = val & 0x08;
			bdi->fdc->side = (val & 0x10) ? 1 : 0;		// side
			bdi->fdc->mfm = val & 0x40;
			break;
	}
	return 1;
}

unsigned char bdiIn(BDI* bdi,int port) {
	unsigned char res = 0xff;
	switch (port) {
		case FDC_COM:
		case FDC_TRK:
		case FDC_SEC:
		case FDC_DATA:
			res = fdcRd(bdi->fdc,port);
			break;
		case BDI_SYS:
			res = (bdi->fdc->irq ? 0x80 : 0x00) | (bdi->fdc->drq ? 0x40 : 0x00);
			break;
	}
//	if (port == FDC_COM) printf("bdiIn(%.4X) = %.2X\n",port,res);
	return res;
}

void bdiSync(BDI* bdi,int tk) {
	unsigned int tz;
	while (tk > 0) {
		if (tk < (int)bdi->fdc->tf) {
			tz = tk;
			bdi->fdc->tf -= tk;
		} else {
			tz = bdi->fdc->tf;
			bdi->fdc->tf = BYTEDELAY;
		}
		bdi->fdc->t += tz;
		bdi->fdc->idx = (bdi->fdc->t < IDXDELAY) ? 1 : 0;
		if (bdi->fdc->wptr != NULL) {
			bdi->fdc->count -= tz;
			while ((bdi->fdc->wptr != NULL) && (bdi->fdc->count < 0)) {
				bdi->fdc->cop = *(bdi->fdc->wptr++);
				if (vgfunc[bdi->fdc->cop] == NULL) {
					printf("unknown cop %.2X\n",bdi->fdc->cop);
					assert(vgfunc[bdi->fdc->cop] != NULL);		// for debug
				} else {
					vgfunc[bdi->fdc->cop](bdi->fdc);
				}
			}
		}
		tk -= tz;
	}
}
