// for future: NEC PC98xx (1st gen: 9801/E/F/M)
// CPU: 8086
// Hardware: almost IBM PC

// Memory map:
// 00000-9ffff - ram
// a0000-a7fff - text video mode data
// a8000-bffff - graphics video mode data (layers 0-2)
// c0000-c7fff - expansion rom (user)
// c8000-dffff - expansion rom (system)
// e0000-e7fff - video ram (optional - layer 3)
// e8000-fffff - bios
// reset @ ffff:0000 = ffff0

#define portA	reg[64]
#define portB	reg[65]
#define portC	reg[66]

#include "../spectrum.h"

// ram
int pc98xx_ram_rd(int adr, void* p) {
	Computer* comp = (Computer*)p;
	return (adr < comp->mem->ramSize) ? comp->mem->ramData[adr] : 0xff;
}

void pc98xx_ram_wr(int adr, int val, void* p) {
	Computer* comp = (Computer*)p;
	if (adr < comp->mem->ramSize)
		comp->mem->ramData[adr] = val & 0xff;
}

// video
int pc98xx_vid_rd(int adr, void* p) {
	return -1;
}

void pc98xx_vid_wr(int adr, int val, void* p) {

}

// bios
int pc98xx_bios_rd(int adr, void* p) {
	Computer* comp = (Computer*)p;
	adr -= 0xe8000;			// rom size is 96K (rounded to 128K), without this e8000 will be at 8000 in rom
	return comp->mem->romData[adr & comp->mem->romMask];
}

// adr bus width 20 bit
// max.adr = FFFFF (1MB)
// page size = 4096 bytes

void pc98xx_mem_map(Computer* comp) {
	memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_1M, pc98xx_ram_rd, pc98xx_ram_wr, comp);	// ram (1M)
	memSetBank(comp->mem, 0xa0, MEM_EXT, 0, MEM_128K, pc98xx_vid_rd, pc98xx_vid_wr, comp);	// video ram (128K)
	memSetBank(comp->mem, 0xc0, MEM_EXT, 0, MEM_128K, NULL, NULL, comp);			// expand roms (128K)
	memSetBank(comp->mem, 0xe0, MEM_EXT, 0, MEM_32K, NULL, NULL, comp);			// video ram (32K)
	memSetBank(comp->mem, 0xe8, MEM_ROM, 0, MEM_64K, pc98xx_bios_rd, NULL, comp);		// bios (96K)
	memSetBank(comp->mem, 0xf8, MEM_ROM, 2, MEM_32K, pc98xx_bios_rd, NULL, comp);
}

void pc98xx_reset(Computer* comp) {
	cpu_reset(comp->cpu);
	comp->portC = 0xff;
	pc98xx_mem_map(comp);
}

// uPD8255 - misc data (rs232,soft reset,misc flags) (31,33,35,37)
// portA - system switch (ro)
// portB (ro)	b0:CDAT (upd4990 data out)
//		b1:EMCK (ext.ram parity error)
//		b2:IMCK (int.ram parity error)
//		b3:CRTT (1:24KHz, 0:15KHz) - display hires
//		b4:INT3 (HDD)
//		b5:CD - RS232
//		b6:CS - RS232
//		b7:CI - RS232
// portC (r)	b0:RXRE - RS232
//		b1:TXEE - RS232
//		b2:TXRE - RS232
//		b3:BUZ (1:speaker off, 0:on)
//		b4:MCKEN enable memory check, portB EMCK/IMCK
//		b5:SHUT1 (0x:continue execution after reset; 10:shutdown? 11:hard.reset)
//		b6:PSTBM (printer)
//		b7:SHUT0
// portC (35w)	see portC rd
// portC (37w)	b0:DT
//		b1,2,3:ADR0,1,2
//		meaning: write bit(ADR)=DT
int pc98xx_sys_rd(Computer* comp, int adr) {
	int res = -1;
	switch((adr >> 1) & 3) {
		case 0:			// port A - system switch
			res = comp->portA;
			break;
		case 1:			// port B
			res = comp->portB;
			break;
		case 2:			// port C
			res = comp->portC;
			break;
	}
	return res;
}

void pc98xx_sys_wr(Computer* comp, int adr, int val) {
	int msk;
	switch((adr >> 1) & 3) {
		case 2:			// port C
			comp->portC = val & 0xff;
			break;
		case 3:			// port C (alt). b1,2,3 = bit nr, b0 = bit state
			msk = 1 << ((val >> 1) & 7);
			if (val & 1) {		// set
				comp->portC |= msk;
			} else {		// reset
				comp->portC &= ~msk;
			}
			break;
	}
}

// uPD8253 - PIT (rw:71,73,75, w:77), timer
// adr 1..3 -> pit adr 0..2
int pc98xx_pit_rd(Computer* comp, int adr) {
	return pit_rd(comp->pit, (adr >> 1) & 7);
}

void pc98xx_pit_wr(Computer* comp, int adr, int val) {
	pit_wr(comp->pit, (adr >> 1) & 7, val);
}

// uPD8259 - PIC
// b3:0-master,1-slave
// b1:PIC A0
// pic	master	0:timer
//		1:keyboard
//		2:crtv (vsync)
//		3,5,6: external bus int
//		4:rs232
//		7:slave
//	slave	0:printer (v30 only)
//		1:hdd
//		2:dd fdc
//		3:hd fdc
//		4:fm sound
//		5:mouse
//		6:coprocessor (v30 only)
int pc98xx_pic_rd(Computer* comp, int adr) {
	return pic_rd((adr & 8) ? comp->spic : comp->mpic, (adr >> 1) & 1);
}

void pc98xx_pic_wr(Computer* comp, int adr, int val) {
	pic_wr((adr & 8) ? comp->spic : comp->mpic, (adr >> 1) & 1, val);
}

// uPD4990 - clock/calendar RTC (serial data transfer) (20,33)
void pc98xx_rtc_wr(Computer* comp, int adr, int val) {
	// upd4990_wr(comp->upd4990, val);
}

// uPD8251 - keyboard controller (r:41,rw:43)
// video(txt)
// text GDC
// CRTC
// video(gra)
// FDC
// RS232
// mouse
// printer
// fm sound (Yamaha 2203 ?)

// deBUG
int pc98xx_dbg_rd(Computer* comp, int adr) {
	printf("pc98xx: ird %X\n", adr);
	comp_irq(IRQ_BRK, comp);
	return -1;
}

void pc98xx_dbg_wr(Computer* comp, int adr, int val) {
	printf("pc98xx: iwr %X, %X\n", adr, val);
	comp_irq(IRQ_BRK, comp);
}

// TODO: fill this table
// ports from 'pc9801 programmers bible'
// NOTE: run with --panic argument to exit on unknown i/o address
// 43d - switch rom pages ???
xPort pc98xx_io_map[] = {
	{0xcf1, 0x000, 0, 0, 0, pc98xx_pic_rd,	pc98xx_pic_wr},		// 00,02,08,0a PIC, b3:slave, b1:A0
//	{0xce1, 0x001, 0, 0, 0, pc98xx_dma_rd,	pc98xx_dma_wr},		// upd8237, dma controller (official docs: dma & pic ports conflicted)
//	{0xcf1, 0x020, 0, 0, 0, NULL,		pc98xx_rtc_wr},		// 20: upd4990,rtc
//	{0xcf1, 0x021, 0, 0, 0, NULL,		NULL},			// 21,23,25,27: dma bank
//	{0xcf1, 0x030, 0, 0, 0, NULL,		NULL},			// 30,32: rs232 on upd8251
	{0xcf1, 0x031, 0, 0, 0, pc98xx_sys_rd,	pc98xx_sys_wr},		// 31,33,35,37: system ports on upd8255
//	{0xcf1, 0x040, 0, 0, 0, NULL,		NULL},			// 40,42,44,46: printer on upd8255
//	{0xcf1, 0x041, 0, 0, 0, NULL,		NULL},			// 41,43: keyboard on upd8251
//	{0xcf1, 0x050, 0, 0, 0, NULL,		NULL},			// 50,52: NMI controller
//	{0xcf1, 0x060, 0, 0, 0, NULL,		NULL},			// 60,62,64,66,68,6a,6c,6e: text video upd7220a
//	{0xcf1, 0x070, 0, 0, 0, NULL,		NULL},			// 70,72,74,76,78,7a,7c,7e: crtc, grcg
	{0xcf1, 0x071, 0, 0, 0, pc98xx_pit_rd,	pc98xx_pit_wr},		// 71,73,75,77: PIT on upd8253
//	{0x0fd, 0x080, 0, 0, 0, NULL,		NULL},			// 80,82: hard disk inerface
//	{0xff9, 0x188, 0, 0, 0, NULL,		NULL},			// 188,18a,18c,18e: ym2203
//	{0x0f9, 0x089, 0, 0, 0, NULL,		NULL},			// 89,8b,8d,8f: network interface
//	{0x0f1, 0x090, 0, 0, 0, NULL,		NULL},			// 90,92,94,96: hd fdd on upd765
//	{0x0fd, 0x099, 0, 0, 0, NULL,		NULL},			// 99,9b: gp-ib switch
//	{0xcf1, 0x0a0, 0, 0, 0, NULL,		NULL},			// a0,a2,a4,a6,a8,aa,ac,ae: graphic video upd7220a
//	{0xcf1, 0x1a0, 0, 0, 0, NULL,		NULL},			// ecg
//	{0xff1, 0x9a0, 0, 0, 0, NULL,		NULL},			// graphic control
//	{0xcf1, 0x0a1, 0, 0, 0, NULL,		NULL},			// kanjirom
//	{0x0f0, 0x0b0, 0, 0, 0, NULL,		NULL},			// b0..bf: communication controller / rs232 expansion interface; 0xbe: fdd switcher
//	{0x0f9, 0x0c8, 0, 0, 0, NULL,		NULL},			// c8,ca,cc,ce: dd drive on upd765
//	{0x0f1, 0x0c1, 0, 0, 0, NULL,		NULL},			// gp-ib on upd7210
//	{0xfff9,0x7fd9,0, 0, 0, NULL,		NULL},			// 7fd9/b/d/f: mouse controller on upd8255
//	{0xfffb,0x3fdb,0, 0, 0, NULL,		NULL},			// 3fdb/f: upd8253 (timer for beeper)
//	{0xffff,0xbfdb,0, 0, 0, NULL,		NULL},			// bfdb: mouse interrupt ?
//	{0xcf9, 0x0f0, 0, 0, 0, NULL,		NULL},			// f0,f2,f4,f6: cpu ?
//	{0xcf8, 0x0f8, 0, 0, 0, NULL,		NULL},			// f8..ff: ndp ?
	{0x000, 0x000, 0, 0, 0, pc98xx_dbg_rd,	pc98xx_dbg_wr}
};

int pc98xx_iord(Computer* comp, int adr) {
	return hwIn(pc98xx_io_map, comp, adr);
}

void pc98xx_iowr(Computer* comp, int adr, int val) {
	hwOut(pc98xx_io_map, comp, adr, val, 0);
}

int pc98xx_mrd(Computer* comp, int adr, int m1) {
	return memRd(comp->mem, adr);
}

void pc98xx_mwr(Computer* comp, int adr, int val) {
	memWr(comp->mem, adr, val);
}

void pc98xx_irq(Computer* comp, int id) {
	switch(id) {
		case IRQ_PIT_CH0: break;	// ch0: interval
		case IRQ_PIT_CH1: break;	// ch1: speaker NOTE:9801/E/F/M for memory refresh
		case IRQ_PIT_CH2: break;	// ch2: rs232 timer
	}
}

void pc98xx_sync(Computer* comp, int ns) {
	pit_sync(comp->pit, ns);		// timers
}

sndPair pc98xx_vol(Computer* comp, sndVolume* vol) {
	sndPair v;
	v.left = 0;
	v.right = 0;
	return v;
}
