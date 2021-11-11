#include "hardware.h"

void ibm_mem_map(Computer* comp) {}	// no hw memory mapping

void ibm_reset(Computer* comp) {
	pit_reset(&comp->pit);
	pic_reset(&comp->mpic);
	pic_reset(&comp->spic);
	comp->keyb->row = -1;
}

// ibm pc/at
// 00000..9FFFF : ram
// A0000..BFFFF : video
// C0000..CFFFF : adapter roms
// D0000..DFFFF : ram pages
// E0000..EFFFF :
// F0000..FFFFF : bios
// 100000+	: ram

int ibm_mrd(Computer* comp, int adr, int m1) {
	int res = -1;
	if (adr < 0xa0000) {
		res = comp->mem->ramData[adr & comp->mem->ramMask];		// ram up to 640K
	} else if (adr < 0xc0000) {
		// printf("video mem rd %.6X\n",adr);
		// videomem
	} else if (adr < 0xd0000) {
		// ext.bios
	} else if (adr < 0xe0000) {
		res = comp->mem->ramData[adr & comp->mem->ramMask];		// ram pages ?
	} else if (adr < 0x100000) {
		res = comp->mem->romData[adr & comp->mem->romMask];
	} else {
		res = comp->mem->ramData[adr & comp->mem->ramMask];		// ram 640K+
	}
	return res;
}

void ibm_mwr(Computer* comp, int adr, int val) {
	if (adr < 0xa0000) {
		comp->mem->ramData[adr & comp->mem->ramMask] = val & 0xff;
	} else if (adr < 0xc0000) {
		// printf("video mem wr %.6X,%.2X\n",adr,val);
		// video mem
	} else if (adr < 0xd0000) {
		// ext.bios
	} else if (adr < 0xe0000) {
		comp->mem->ramData[adr & comp->mem->ramMask] = val & 0xff;
	} else if (adr < 0x100000) {
		// bios
	} else {
		comp->mem->ramData[adr & comp->mem->ramMask] = val & 0xff;
	}
}

// in/out

// 20 master pic command
// 21 master pic data
// A0 slave pic command
// A1 slave pic data

int ibm_inPIC(Computer* comp, int adr) {
	PIC* pic = (adr & 0x80) ? &comp->spic : &comp->mpic;	// select master or slave
	return pic_rd(pic, adr);
}

void ibm_outPIC(Computer* comp, int adr, int data) {
	PIC* pic = (adr & 0x80) ? &comp->spic : &comp->mpic;
	pic_wr(pic, adr, data);
}

/*
	040-05F  8253 or 8254 Programmable Interval Timer (PIT, see ~8253~)
	040 8253 channel 0, counter divisor
	041 8253 channel 1, RAM refresh counter
	042 8253 channel 2, Cassette and speaker functions
	043 8253 mode control  (see 8253)
not us	044 8254 PS/2 extended timer
	047 8254 Channel 3 control byte
*/

int ibm_inPIT(Computer* comp, int adr) {
	return pit_rd(&comp->pit, adr);
}

void ibm_outPIT(Computer* comp, int adr, int val) {
	pit_wr(&comp->pit,adr,val);
}

// ps/2 controller

int ibm_inKbd(Computer* comp, int adr) {
	int res = -1;
	switch (adr & 0x0f) {
		case 0:
			res = ps2c_rd(comp->ps2c, PS2_RDATA);
			break;
		case 1:
			res = comp->reg[0x61];
			break;
		case 4:
			res = ps2c_rd(comp->ps2c, PS2_RSTATUS);
			break;
	}
	return res;
}

void ibm_outKbd(Computer* comp, int adr, int val) {
	switch(adr & 0x0f) {
		case 0:
			ps2c_wr(comp->ps2c, PS2_RDATA, val);
			break;
		case 1:
			comp->reg[0x61] = val & 0xff;
			if (!(val & 0x80))
				comp->keyb->outbuf = 0;
			break;
		case 4:
			ps2c_wr(comp->ps2c, PS2_RCMD, val);
			if (comp->ps2c->reset)
				compReset(comp, RES_DEFAULT);
			break;
	}
}

// dma1
// 000-007	bar/car & bwcr/cwr, b1,2=channel nr, b0?bwcr:bar
// 87,83,81,82	par of ch0-3
// 08-0f	controller port (b0-2)

// dma2
// 0c0-0ce	bar/car && bwcr/cwr, b2,3=channel, b0=0, b1?bwcr:bar
// xx,8a,89,8b	par of ch4-7
// d0-de	controller port (b1-3), b0=0

// cmos

/*
0070	w	CMOS RAM index register port (ISA, EISA)
		 bit 7	 = 1  NMI disabled
			 = 0  NMI enabled
		 bit 6-0      CMOS RAM index (64 bytes, sometimes 128 bytes)

		any write to 0070 should be followed by an action to 0071
		or the RTC wil be left in an unknown state.

0071	r/w	CMOS RAM data port (ISA, EISA)
*/
void ibm_out70(Computer* comp, int adr, int val) {
	cmos_wr(&comp->cmos, CMOS_ADR, val);
}

void ibm_out71(Computer* comp, int adr, int val) {
	cmos_wr(&comp->cmos, CMOS_DATA, val);
}

int ibm_in71(Computer* comp, int adr) {
	return cmos_rd(&comp->cmos, CMOS_DATA);
}

// post code

void ibm_out80(Computer* comp, int adr, int val) {
	printf("POST %.2X\n",val & 0xff);
}

// undef

int ibm_inDBG(Computer* comp, int adr) {
	printf("ibm %.4X: in %.4X\n",comp->cpu->oldpc, adr & 0xffff);
	assert(0);
	return -1;
}

void ibm_outDBG(Computer* comp, int adr, int val) {
	printf("ibm %.4X: out %.4X,%.2X\n",comp->cpu->oldpc, adr & 0xffff, val & 0xff);
	assert(0);
}

static xPort ibmPortMap[] = {
	{0xff62,0x0020,2,2,2,ibm_inPIC,	ibm_outPIC},	// 20,21:master pic, a0,a1:slave pic	s01x xx00. s-slave
	{0xffe0,0x0040,2,2,2,ibm_inPIT,	ibm_outPIT},	// programmable interval timer
	{0xfff0,0x0060,2,2,2,ibm_inKbd,	ibm_outKbd},	// 8042: ps/2 keyboard/mouse controller
	{0xffff,0x0070,2,2,2,NULL,	ibm_out70},	// cmos
	{0xffff,0x0071,2,2,2,ibm_in71,	ibm_out71},
	{0xffff,0x0080,2,2,2,NULL,	ibm_out80},	// post code
	{0x0000,0x0000,2,2,2,ibm_inDBG,	ibm_outDBG}
};

int ibm_iord(Computer* comp, int adr, int nonsence) {
//	printf("in %.4X\n",adr);
	return hwIn(ibmPortMap, comp, adr, 0);
}

void ibm_iowr(Computer* comp, int adr, int val, int nonsense) {
//	printf("out %.4X\n",adr);
	hwOut(ibmPortMap, comp, adr, val, 0, 0);
}

void ibm_init(Computer* comp) {
	comp->keyb->mem[0] = 0x00;	// kbd config
}

void ibm_sync(Computer* comp, int ns) {
	bcSync(comp->beep, ns);
	// ps/2 controller
	if (comp->ps2c->intk) {
		comp->ps2c->intk = 0;
		pic_int(&comp->mpic, 1);	// int1:keyboard interrypt
	}
	if (comp->ps2c->intm) {
		comp->ps2c->intm = 0;
		pic_int(&comp->spic, 4);	// int12:mouse interrupt (int4 on slave pic)
	}

	// pit
	pit_sync(&comp->pit, ns);
	// ch0 connected to int0
	if (!comp->pit.ch0.lout && comp->pit.ch0.out) {		// 0->1
		pic_int(&comp->mpic, 0);	// int0 master pic
	}
	comp->pit.ch0.lout = comp->pit.ch0.out;
	// ch2 connected to speaker
	comp->pit.ch2.lout = comp->pit.ch2.out;
	comp->beep->lev = (comp->reg[0x61] & 2) ? comp->pit.ch2.out : 1;

	// pic
	if (comp->spic.oint)		// slave pic int -> master pic int2
		pic_int(&comp->mpic, 2);
	if (comp->mpic.oint) {
		int v = pic_ack(&comp->mpic);
		if (v < 0) v = pic_ack(&comp->spic);
		comp->cpu->intrq |= I286_INT;
		comp->cpu->intvec = v & 0xffff;
	}
}

// key press/release
void ibm_keyp(Computer* comp, keyEntry kent) {
	ps2c_wr_ob(comp->ps2c, comp->keyb->outbuf);
	comp->keyb->outbuf = 0;
}

void ibm_keyr(Computer* comp, keyEntry kent) {
	ps2c_wr_ob(comp->ps2c, comp->keyb->outbuf);
	comp->keyb->outbuf = 0;
}

sndPair ibm_vol(Computer* comp, sndVolume* vol) {
	sndPair res;
	res.left = comp->beep->val * vol->beep / 4;
	res.right = res.left;
	return res;
}
