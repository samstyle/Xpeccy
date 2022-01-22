#include <string.h>

#include "hardware.h"
#include "../video/vga.h"

// ibm pc/at
// 00000..9FFFF : ram
// A0000..BFFFF : video
// C0000..CFFFF : adapter roms
// D0000..DFFFF : ram pages
// E0000..EFFFF :
// F0000..FFFFF : bios
// 100000+	: ram

// ram
int ibm_ram_rd(int adr, void* p) {
	Computer* comp = (Computer*)p;
	return (adr < comp->mem->ramSize) ? comp->mem->ramData[adr] : 0xff;
}

void ibm_ram_wr(int adr, int val, void* p) {
	Computer* comp = (Computer*)p;
	if (adr < comp->mem->ramSize)
		comp->mem->ramData[adr] = val;
}

// a0000..bffff = videomem
int ibm_vga_rd(int adr, void* p) {
	return vga_mrd(((Computer*)p)->vid, adr);
}

void ibm_vga_wr(int adr, int val, void* p) {
	vga_mwr(((Computer*)p)->vid, adr, val);
}

// c0000..cffff adapter bios
int ibm_dum_rd(int adr, void* p) {return -1;}
void ibm_dum_wr(int adr, int val, void* p) {}

// e0000..fffff bios
int ibm_bios_rd(int adr, void* p) {
	Computer* comp = (Computer*)p;
	return comp->mem->romData[adr & comp->mem->romMask & 0x1ffff];
}

void ibm_mem_map(Computer* comp) {
	memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_64K, ibm_ram_rd, ibm_ram_wr, comp);		// all is ram (up to 16M), except:
	memSetBank(comp->mem, 0x0a, MEM_EXT, 0, 0x200, ibm_vga_rd, ibm_vga_wr, comp);		// a0000..bffff video
	memSetBank(comp->mem, 0x0c, MEM_EXT, 0, 0x100, ibm_dum_rd, ibm_dum_wr, comp);		// c0000..cffff adapter bios
	memSetBank(comp->mem, 0x0e, MEM_ROM, 0, 0x200, ibm_bios_rd, ibm_dum_wr, comp);		// e0000..fffff bios
}

int ibm_mrd(Computer* comp, int adr, int m1) {
	return memRd(comp->mem, adr);
}

void ibm_mwr(Computer* comp, int adr, int val) {
	memWr(comp->mem, adr, val);
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

/*
----------------------------------------------------------------------------
Port 61H: PS/2 System Control Port B
----------------------------------------------------------------------------

Write operations:

   Bit   Function

    7    Reset system timer 0 output latch (IRQ0)
    6    Reserved
    5    Reserved
    4    Reserved
    3    Enable channel check
    2    Enable parity check
    1    Speaker data enable
    0    System timer 2 gate to speaker

Read operations:

    7    1 = Parity check occurred
    6    1 = Channel check occurred
    5    System timer 2 output
    4    Toggles with each refresh request
    3    Enable channel check result
    2    Enable parity check result
    1    Speaker data enable result
    0    System timer 2 gate to speaker result
*/

int ibm_inKbd(Computer* comp, int adr) {
	int res = -1;
	switch (adr & 0x0f) {
		case 0:
			res = ps2c_rd(comp->ps2c, PS2_RDATA);
			break;
		case 1:
			// comp->reg[0x61] ^= 0x10;		// Toggles with each refresh request (?)
			res = comp->reg[0x61] & 0x1f;		// b0..3 is copied
			if (comp->pit.ch2.out) res |= 0x20;	// b6: timer2 output
			break;
		case 4:
			res = ps2c_rd(comp->ps2c, PS2_RSTATUS);
			break;
	}
	//printf("%.4X:%.4X kbd in %.3X = %.2X\n",comp->cpu->cs.idx,comp->cpu->pc,adr,res);
	return res;
}

void ibm_outKbd(Computer* comp, int adr, int val) {
	switch(adr & 0x0f) {
		case 0:
			ps2c_wr(comp->ps2c, PS2_RDATA, val);
			break;
		case 1:
			comp->reg[0x61] = val & 0xff;
			if (val & 0x80) {
				comp->keyb->outbuf = 0;
				comp->pit.ch0.out = 0;
			}
			break;
		case 4:
			ps2c_wr(comp->ps2c, PS2_RCMD, val);
			if (comp->ps2c->reset) {
				comp->ps2c->reset = 0;
				compReset(comp, RES_DEFAULT);
			}
			break;
	}
	//printf("%.4X:%.4X kbd out %.3X, %.2X\n",comp->cpu->cs.idx,comp->cpu->pc,adr,val);
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
	cmos_wr(&comp->cmos, CMOS_ADR, val & 0x7f);
}

void ibm_out71(Computer* comp, int adr, int val) {
	cmos_wr(&comp->cmos, CMOS_DATA, val);
}

int ibm_in71(Computer* comp, int adr) {
	return cmos_rd(&comp->cmos, CMOS_DATA);
}

// post code

void ibm_out80(Computer* comp, int adr, int val) {
	printf("%4X:%.4X\tPOST %.2X\n",comp->cpu->cs.idx,comp->cpu->pc,val & 0xff);
}

// hdc (hdd controllers)
// 170..177	secondary hdc
// 1f0..1f7	primary hdc
// 3f6		primary hdc ctrl/astat register

int ibm_in1fx(Computer* comp, int adr) {
	int res = ataRd(comp->ide->curDev, adr & 7);
	// printf("%.4X:%.4X ata rd %.3X = %.2X\n",comp->cpu->cs.idx,comp->cpu->pc,adr,res);
	return res;
}

int ibm_in3f6(Computer* comp, int adr) {
	return ataRd(comp->ide->curDev, HDD_ASTATE);
}

void ibm_out1fx(Computer* comp, int adr, int val) {
	// printf("%.4X:%.4X ata wr %.3X, %.2X\n",comp->cpu->cs.idx,comp->cpu->pc,adr,val);
	if ((adr & 7) == 6)
		comp->ide->curDev = (val & 0x10) ? comp->ide->slave : comp->ide->master;
	ataWr(comp->ide->curDev, adr & 7, val);
}

void ibm_out3f6(Computer* comp, int adr, int val) {
	ataWr(comp->ide->curDev, HDD_CTRL, val);
}

// mda/cga/ega/vga

// 3d0..3d7 = 3b0..3b7 (b0:selects regnum/regval)

void ibm_out3b4(Computer* comp, int adr, int val) {
	vga_wr(comp->vid, VGA_CRTRN, val);
}

void ibm_out3b5(Computer* comp, int adr, int val) {
	vga_wr(comp->vid, VGA_CRTRD, val);
}

void ibm_out3c0(Computer* comp, int adr, int val) {
	vga_wr(comp->vid, VGA_ATREG, val);
}

void ibm_out3c4(Computer* comp, int adr, int val) {
	vga_wr(comp->vid, VGA_SEQRN, val);
}

void ibm_out3c5(Computer* comp, int adr, int val) {
	vga_wr(comp->vid, VGA_SEQRD, val);
}

/*
03D8	r/w	CGA mode control register  (except PCjr)
		 bit 7-6      not used
		 bit 5	 = 1  blink enabled
		 bit 4	 = 1  640*200 graphics mode
		 bit 3	 = 1  video enabled
		 bit 2	 = 1  monochrome signal
		 bit 1	 = 0  text mode
			 = 1  320*200 graphics mode
		 bit 0	 = 0  40*25 text mode
			 = 1  80*25 text mode

03D9	r/w	CGA palette register
		 bit 7-6      not used
		 bit 5	 = 0 active color set: red, green brown
			 = 1 active color set: cyan, magenta, white
		 bit 4	     intense colors in graphics, background colors text
		 bit 3	     intense border in 40*25, intense background in
			     320*200, intense foreground in 640*200
		 bit 2	     red border in 40*25, red background in 320*200,
			     red foreground in 640*200
		 bit 1	     green border in 40*25, green background in
			     320*200, green foreground in 640*200
		 bit 0	     blue border in 40*25, blue background in 320*200,
			     blue foreground in 640*200

03DA	r	CGA status register	EGA/VGA: input status 1 register
		 bit 7-4     not used
		 bit 3 = 1   in vertical retrace
		 bit 2 = 1   light pen switch is off
		 bit 1 = 1   positive edge from light pen has set trigger
		 bit 0 = 0   do not use memory
		       = 1   memory access without interfering with display
*/

void ibm_out3b8(Computer* comp, int adr, int val) {
	vga_wr(comp->vid, VGA_MODE, val);
}

// ega/vga: 3ba = 3da = input status 1 register; mda = crt status
int ibm_in3ba(Computer* comp, int adr) {
	return vga_rd(comp->vid, VGA_STAT1);
}

// fdc (i8272a)

// 3f1-3f7 -> fdc

int ibm_fdc_rd(Computer* comp, int adr) {
	int res = -1;
	difIn(comp->dif, adr, &res, 0);
	// printf("%.4X:%.4X\tin %.3X = %.2X\n",comp->cpu->cs.idx,comp->cpu->pc,adr,res);
	return res;
}

void ibm_fdc_wr(Computer* comp, int adr, int val) {
	// printf("%.4X:%.4X\tout %.3X, %.2X\n",comp->cpu->cs.idx,comp->cpu->pc,adr,val);
	difOut(comp->dif, adr, val, 0);
}

// dma

int ibm_inDMA(Computer* comp, int adr) {
	printf("DMA: rd %.3X\n",adr);
	return -1;
}

void ibm_outDMA(Computer* comp, int adr, int val) {
	switch(adr) {
		case 0x00: dma_wr(comp->dma8, DMA_CH_BAR, 0, val); break;	// base address
		case 0x01: dma_wr(comp->dma8, DMA_CH_BWCR, 0, val); break;	// word count
		case 0x02: dma_wr(comp->dma8, DMA_CH_BAR, 1, val); break;
		case 0x03: dma_wr(comp->dma8, DMA_CH_BWCR, 1, val); break;
		case 0x04: dma_wr(comp->dma8, DMA_CH_BAR, 2, val); break;
		case 0x05: dma_wr(comp->dma8, DMA_CH_BWCR, 2, val); break;
		case 0x06: dma_wr(comp->dma8, DMA_CH_BAR, 3, val); break;
		case 0x07: dma_wr(comp->dma8, DMA_CH_BWCR, 3, val); break;
		case 0x08: dma_wr(comp->dma8, DMA_CR, -1, val); break;		// command register
		case 0x09: dma_wr(comp->dma8, DMA_RR, -1, val); break;		// request register
		case 0x0a: dma_wr(comp->dma8, DMA_CMR, -1, val); break;		// channel mask register
		case 0x0b: dma_wr(comp->dma8, DMA_MR, -1, val); break;		// mode register
		case 0x0c: dma_wr(comp->dma8, DMA_BTR, -1, val); break;		// byte trigger reset
		case 0x0d: dma_wr(comp->dma8, DMA_RES, -1, val); break;		// master clear
		case 0x0e: dma_wr(comp->dma8, DMA_MRES, -1, val); break;	// clear mask register
		case 0x0f: dma_wr(comp->dma8, DMA_WAMR, -1, val); break;	// write mask register
		case 0x81: dma_wr(comp->dma8, DMA_CH_PAR, 2, val & 0x0f); break;// page address registers
		case 0x82: dma_wr(comp->dma8, DMA_CH_PAR, 3, val & 0x0f); break;
		case 0x83: dma_wr(comp->dma8, DMA_CH_PAR, 1, val & 0x0f); break;
		case 0x87: dma_wr(comp->dma8, DMA_CH_PAR, 0, val & 0x0f); break;
	}
}

int ibm_dma_mrd(int adr, void* ptr) {
	return ibm_mrd((Computer*)ptr, adr, 0);
}

void ibm_dma_mwr(int adr, int val, void* ptr) {
	ibm_mwr((Computer*)ptr, adr, val);
}

int ibm_dma_flp_rd(void* ptr, int* f) {
	DiskIF* dif = ((Computer*)ptr)->dif;
	int res = -1;
	*f = 0;
	if (dif->fdc->dma && dif->fdc->irq && dif->fdc->drq && dif->fdc->dir) {		// dma,execution,data request, to cpu;
		*f = difIn(dif, 5, &res, 0);
	}
	return res;
}

void ibm_dma_flp_wr(int val, void* ptr, int* f) {
	DiskIF* dif = ((Computer*)ptr)->dif;
	*f = 0;
	if (dif->fdc->dma && dif->fdc->irq && dif->fdc->drq && !dif->fdc->dir) {	// dma,execution,data request, from cpu;
		*f = difOut(dif, 5, val, 0);
	}
}

int ibm_dma_hdd_rd(void* ptr, int* f) {
	IDE* ide = ((Computer*)ptr)->ide;
	ATADev* dev = ide->curDev;
	int res = -1;
	*f = 0;
	if ((dev->reg.state & HDF_DRQ) && (dev->buf.mode == HDB_READ) && dev->dma) {
		if (ide->hiTrig) {
			res = ide->bus & 0xff;
		} else {
			ide->bus = ataRd(dev, HDD_DATA);
			res = (ide->bus >> 8) & 0xff;
		}
		ide->hiTrig = !ide->hiTrig;
		*f = 1;
	}
	return res;
}

void ibm_dma_hdd_wr(int val, void* ptr, int* f) {
	IDE* ide = ((Computer*)ptr)->ide;
	ATADev* dev = ide->curDev;
	*f = 0;
	if ((dev->reg.state & HDF_DRQ) && (dev->buf.mode == HDB_WRITE) && dev->dma) {
		if (ide->hiTrig) {
			ide->bus &= 0xff00;
			ide->bus |= (val & 0xff);
			ataWr(dev, HDD_DATA, ide->bus);
		} else {
			ide->bus &= 0x00ff;
			ide->bus |= (val << 8) & 0xff00;
		}
	}
}

// undef

int ibm_dumird(Computer* comp, int adr) {return -1;}
void ibm_dumiwr(Computer* comp, int adr, int val) {}

int ibm_inDBG(Computer* comp, int adr) {
	printf("ibm %.4X:%.4X: in %.4X\n",comp->cpu->cs.idx,comp->cpu->oldpc, adr & 0xffff);
	assert(0);
	return -1;
}

void ibm_outDBG(Computer* comp, int adr, int val) {
	printf("ibm %.4X:%.4X: out %.4X,%.2X\n",comp->cpu->cs.idx,comp->cpu->oldpc, adr & 0xffff, val & 0xff);
	assert(0);
}

static xPort ibmPortMap[] = {
	{0x03f0,0x0000,2,2,2,ibm_inDMA, ibm_outDMA},	// dma1 000..00f
	{0x0362,0x0020,2,2,2,ibm_inPIC,	ibm_outPIC},	// 20,21:master pic, a0,a1:slave pic	s01x xx00. s-slave
	{0x03e0,0x0040,2,2,2,ibm_inPIT,	ibm_outPIT},	// programmable interval timer
	{0x03f0,0x0060,2,2,2,ibm_inKbd,	ibm_outKbd},	// 8042: keyboard/mouse controller
	{0x03ff,0x0070,2,2,2,NULL,	ibm_out70},	// cmos
	{0x03ff,0x0071,2,2,2,ibm_in71,	ibm_out71},
	{0x03ff,0x0080,2,2,2,NULL,	ibm_out80},	// post code
	{0x03ff,0x0081,2,2,2,ibm_inDMA, ibm_outDMA},	// dma pages
	{0x03ff,0x0082,2,2,2,ibm_inDMA, ibm_outDMA},
	{0x03ff,0x0083,2,2,2,ibm_inDMA, ibm_outDMA},
	{0x03ff,0x0087,2,2,2,ibm_inDMA, ibm_outDMA},
//	{0x03f8,0x0170,2,2,2,ibm_dumird,ibm_dumiwr},	// secondary ide
	{0x03f8,0x01f0,2,2,2,ibm_in1fx,	ibm_out1fx},	// primary ide
	{0x03ff,0x03f6,2,2,2,ibm_in3f6,	ibm_out3f6},
	{0x03f9,0x03b4,2,2,2,NULL,	ibm_out3b4},	// crt registers
	{0x03f9,0x03d4,2,2,2,NULL,	ibm_out3b4},	// 3d0..3d7 = 3b0..3b7
	{0x03f9,0x03b5,2,2,2,NULL,	ibm_out3b5},
	{0x03f9,0x03d5,2,2,2,NULL,	ibm_out3b5},
	{0x03ff,0x03b8,2,2,2,NULL,	ibm_out3b8},	// video mode (3b8/3d8)
	{0x03ff,0x03d8,2,2,2,NULL,	ibm_out3b8},
	{0x03ff,0x03c0,2,2,2,NULL,	ibm_out3c0},	// 3c0: atr.reg idx/data
	{0x03f9,0x03c4,2,2,2,NULL,	ibm_out3c4},	// seq.registers
	{0x03f9,0x03c5,2,2,2,NULL,	ibm_out3c5},
	{0x03ff,0x03ba,2,2,2,ibm_in3ba,	NULL},		// status reg 1 (3ba/3da)
	{0x03ff,0x03da,2,2,2,ibm_in3ba, NULL},
	{0x03f8,0x03f0,2,2,2,ibm_fdc_rd,ibm_fdc_wr},	// 1st fdd controller
//	{0x03f8,0x0370,2,2,2,NULL,	NULL},		// 2nd fdd controller
	{0x0000,0x0000,2,2,2,ibm_inDBG,	ibm_outDBG}
};

int ibm_iord(Computer* comp, int adr) {
//	printf("in %.4X\n",adr);
//	comp->brk = 1;
	return hwIn(ibmPortMap, comp, adr);
}

void ibm_iowr(Computer* comp, int adr, int val) {
//	printf("out %.4X\n",adr);
//	comp->brk = 1;
	hwOut(ibmPortMap, comp, adr, val, 0);
}

static const int ega_def_idx[16] = {0,1,2,3,4,5,20,7,56,57,58,59,60,61,62,63};

void ibm_reset(Computer* comp) {
//	pit_reset(&comp->pit);		// pit don't have reset input (!)
	pic_reset(&comp->mpic);
	pic_reset(&comp->spic);
	ps2c_reset(comp->ps2c);

	memcpy(comp->vid->ram + 0x20000, comp->vid->font, 0x2000);	// copy default font
	int i;
	xColor xcol;
	for (i = 0; i < 0x40; i++) {
		xcol.r = 0x55 * (((i >> 1) & 2) + ((i >> 5) & 1));
		xcol.g = 0x55 * ((i & 2) + ((i >> 4) & 1));
		xcol.b = 0x55 * (((i << 1) & 2) + ((i >> 3) & 1));
		comp->vid->pal[i + 0x80] = xcol;
	}
	for (i = 0; i < 16; i++) {
		comp->vid->pal[i] = comp->vid->pal[ega_def_idx[i] + 0x80];
	}
	ibm_mem_map(comp);
}

void ibm_init(Computer* comp) {
	comp->vid->nsPerDot = 160;
	fdc_set_hd(comp->dif->fdc, 1);
	dma_set_cb(comp->dma8, ibm_dma_mrd, ibm_dma_mwr);		// mrd/mwr callbacks
	dma_set_chan(comp->dma8, 2, ibm_dma_flp_rd, ibm_dma_flp_wr);	// ch2: fdc
	dma_set_chan(comp->dma8, 3, ibm_dma_hdd_rd, ibm_dma_hdd_wr);	// ch3: hdd
}

void ibm_sync(Computer* comp, int ns) {
	bcSync(comp->beep, ns);
	// dma
	dma_sync(comp->dma8, ns);
	// ps/2 controller
	if (comp->ps2c->intk) {
		comp->ps2c->intk = 0;
		pic_int(&comp->mpic, 1);	// input 1 master pic:keyboard interrupt
	}
	if (comp->ps2c->intm) {
		comp->ps2c->intm = 0;
		//pic_int(&comp->spic, 4);	// input 4 slave pic: mouse interrupt
	}
	// pit
	pit_sync(&comp->pit, ns);
	// ch0 connected to int0
	if (!comp->pit.ch0.lout && comp->pit.ch0.out) {		// 0->1
		pic_int(&comp->mpic, 0);	// input 0 master pic (int 8)
	}
	comp->pit.ch0.lout = comp->pit.ch0.out;
	// ch1 mem refresh
	if (!comp->pit.ch1.lout && comp->pit.ch1.out) {
		comp->reg[0x61] ^= 0x10;
	}
	comp->pit.ch1.lout = comp->pit.ch1.out;
	// ch2 connected to speaker
	comp->pit.ch2.lout = comp->pit.ch2.out;
	comp->beep->lev = (comp->reg[0x61] & 2) ? comp->pit.ch2.out : 1;

	// fdc
	difSync(comp->dif, ns);
	if (comp->dif->intrq) {		// fdc -> master pic int6
		comp->dif->intrq = 0;
		pic_int(&comp->mpic, 6);
	}
	// hdd
	// int14 (slave int6): primary hdc
	if (comp->ide->master->intrq || comp->ide->slave->intrq) {
		comp->ide->master->intrq = 0;
		comp->ide->slave->intrq = 0;
		pic_int(&comp->spic, 6);
	}
	// int15 (slave int7): secondary hdc
	// pic
	if (comp->spic.oint)		// slave pic int -> master pic int2
		pic_int(&comp->mpic, 2);
	if (comp->mpic.oint) {
		int v = pic_ack(&comp->mpic);
		if (v < 0) v = pic_ack(&comp->spic);
		comp->cpu->intrq |= I286_INT;
		comp->cpu->intvec = v & 0xffff;
	}
	// debugging
	if (comp->dif->fdc->brk) {
		comp->dif->fdc->brk = 0;
		comp->brk = 1;
	}
}

// key press/release
void ibm_keyp(Computer* comp, keyEntry kent) {
	ps2c_rd_kbd(comp->ps2c);
}

void ibm_keyr(Computer* comp, keyEntry kent) {
	ps2c_rd_kbd(comp->ps2c);
}

sndPair ibm_vol(Computer* comp, sndVolume* vol) {
	sndPair res;
	res.left = comp->beep->val * vol->beep / 4;
	res.right = res.left;
	return res;
}
