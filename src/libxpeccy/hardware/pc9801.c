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

#include "../video/upd7220.h"

#define regKBDm reg[50]		// mode
#define regKBDc reg[51]		// command
#define regKBDs	reg[52]		// status (oe,fe,pe)
#define regKBDd	reg[53]		// last transfered byte

//#define portA	reg[64] = DIPSW1
#define portB	reg[65]
#define portC	reg[66]
#define DIPSW1	reg[71]
#define DIPSW2	reg[72]
#define DIPSW3	reg[73]

#define flgNMI	flag[0]
#define flgKBDm flag[1]	// upd8251 mode (0:mode, 1:command)

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
	comp->flgKBDm = 0;
	comp->regKBDs = 0;
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

int pc98xx_ppia_rd(void* comp) {return ((Computer*)comp)->DIPSW1;}
int pc98xx_ppib_rd(void* ptr) {
	Computer* comp = (Computer*)ptr;
	int res = ((Computer*)comp)->portB & 0xfe;
	if (((Computer*)comp)->rtc->data) res |= 1;
	return res;
}
int pc98xx_ppic_rd(void* comp) {return ((Computer*)comp)->portC;}
void pc98xx_ppia_wr(int val, void* comp) {}
void pc98xx_ppib_wr(int val, void* comp) {}
void pc98xx_ppich_wr(int val, void* comp) {((Computer*)comp)->portC &= 0x0f; ((Computer*)comp)->portC |= (val & 0xf0);}
void pc98xx_ppicl_wr(int val, void* comp) {((Computer*)comp)->portC &= 0xf0; ((Computer*)comp)->portC |= (val & 0x0f);}

int pc98xx_sys_rd(Computer* comp, int adr) {
	return ppi_rd(comp->ppi, (adr >> 1) & 3);
}

void pc98xx_sys_wr(Computer* comp, int adr, int val) {
	ppi_wr(comp->ppi, (adr >> 1) & 3, val);
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
// a3:0-master,1-slave
// a1:PIC A0
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
	upd4990_wr(comp->rtc, val);
}

// uPD8237 - DMA controller
// ch0: HDD
// ch1: not used
// ch2: uPD765, 1MB fdd
// ch3: uPD765, 640K fdd

static const int dma_ctrl_regs[8] = {DMA_CR, DMA_RR, DMA_CMR, DMA_MR, DMA_BTR, DMA_RES, DMA_MRES, DMA_WAMR};

int pc98xx_dma_rd(Computer* comp, int adr) {
	int res = -1;
	int ch = (adr >> 2) & 3;
	if (adr & 0x20) {
		// bank
		switch ((adr >> 1) & 3) {
			case 0: res = dma_rd(comp->dma1, DMA_CH_PAR, 2); break;
			case 2: res = dma_rd(comp->dma1, DMA_CH_PAR, 3); break;
			case 3: res = dma_rd(comp->dma1, DMA_CH_PAR, 0); break;
		}
	} else if (adr & 0x10) {
		res = dma_rd(comp->dma1, dma_ctrl_regs[(adr >> 1) & 7], 0);
	} else {
		res = dma_rd(comp->dma1, (adr & 2) ? DMA_CH_CAR : DMA_CH_CWR, ch);
	}
	return res;
}

void pc98xx_dma_wr(Computer* comp, int adr, int val) {
	int ch = (adr >> 2) & 3;
	if (adr & 0x20) {
		switch ((adr >> 1) & 3) {
			case 0: dma_wr(comp->dma1, DMA_CH_PAR, 2, val); break;
			case 2: dma_wr(comp->dma1, DMA_CH_PAR, 3, val); break;
			case 3: dma_wr(comp->dma1, DMA_CH_PAR, 0, val); break;
		}
		// bank
	} else if (adr & 0x10) {	// b1,2,3 = command
		dma_wr(comp->dma1, dma_ctrl_regs[(adr >> 1) & 7], 0, val);
	} else {		// b1 = adr/cnt, b2,3 = channel
		dma_wr(comp->dma1, (adr & 2) ? DMA_CH_BAR : DMA_CH_BWCR, ch, val);
	}
}

// TODO: add uart type UPD_8251 and move to uart*.c on comp->uart

// uPD8251 - usart, keyboard controller (rw:41,rw:43)
// 41 - data
// 43 - status/cmd
// status:
// b5 FE:framing error
// b4 OE:overrun error (pressing key w/o reading previous one)
// b3 PE:parity error
// b1 RxRDY: data is ready (=rd drq)
// mode:
// b0,1: bitrate (00:no, 01:x1, 10:x16, 11:x64)
// b2,3: symbol size (00:5bits, 01:6bits, 10:7bits, 11:8bits)
// b4: parity check on
// b5: parity mode (1:odd, 0:even)
// b6,7: stop bit count (00:no, 01:1bit, 10:1.5bits???, 11:2bits)
// cmd:
// b6 IR:internal reset (1:go to initial mode)
// b5 RTS:1:stop keycode generation
// b4 ER:error reset (reset FE,OE,PE in status reg)
// b3 RST:reset
// b2 RXE:reciever enabled usart->comp enabled
// b1 RTY:1:retry last symbol
// b0 TXEN: comp->usart enabled

#define F_ERR_FE	0x20
#define F_ERR_OE	0x10
#define F_ERR_PE	0x08
#define F_RXRDY		0x02

int pc98xx_kbd_rd(Computer* comp, int adr) {
	int res = 0;
	if (adr & 2) {	// 43:status
		res = comp->regKBDs & (F_ERR_FE | F_ERR_OE | F_ERR_PE);
		if (comp->keyb->outbuf & 0xff)
			res |= F_RXRDY;	// data ready
	} else {	// 41
		res = xt_read(comp->keyb);	// FF if no code
		comp->regKBDd = res;
	}
	return res;
}

void pc98xx_kbd_wr(Computer* comp, int adr, int val) {
	if (adr & 2) {	// 43
		if (!comp->flgKBDm) {		// 1st time after reset: write mode
			comp->regKBDm = val;
			comp->flgKBDm = 1;
		} else {
			// others: write command
			comp->regKBDc = val;
			if (val & 0x40) comp->regKBDm = 0;
			comp->keyb->lock = !!(val & 0x20);
			if (val & 0x10) comp->regKBDs &= ~(F_ERR_FE | F_ERR_OE | F_ERR_PE);
		}
	} else {
		// no 41 wr?
	}
}

// uPD7220 - video(txt)
// text GDC
// == 60, 62 - upd7220 rd/wr
// 60:0	rd	b6:hblank
//		b5:vsync
//		b2:fifo buffer full
//		b1:fifo buffer empty
//		b0:data ready
// 60:0	wr	gdc parameter write
// 62:1	rd	gdc data read
// 62:1	wr	gdc command write
// == CRTC
// 64:2	wr	crt interrupt reset
// 68:4	wr	mode flip-flop
//		b0:flag
//		b1,2,3:param.nr:
//		000:atr4 (graph / vertical line)
//		001:graph mode (mono / color)
//		010:column size (40 / 80 chars)
//		011:font size (7x13 / 6x8)
//		100:88 graph mode (200 lines / other)
//		101:kanji access (bitmap / code)
//		110:static memory (enable / disable)
//		111:allow display (enable / disable)
// 6a:5	wr	mode flip-flop 2
//		b0:flag
//		b1-7:param nr:
//		0000000:color select (16col/8col)
//		0000010:enhanced mode (enhanced / compatible)
//		others:reserved
// 6c:6	wr	border color: b4,5,6 = b,r,g
// 6e:7		?

// 70:0	wr	Character 1st line (0)
// 72:1	wr	Character last line (15)
// 74:2	wr	Character Lines Number (15)
// 76:3	wr	Smooth Scroll Lines Count
// 78:4	wr	Scroll Area Above Position Lines Number
// 7a:5	wr	Scroll Area Lines Number
// 7c:6	wr	Fast Graphics Mode Register:
//		b0..3: p0..3 disable
//		b6:rmw mode
//		b7:cg mode
// 7e:7	wr	Tile Register 0-3

int pc98xx_gdc_rd(Computer* comp, int adr) {
	int res = -1;
	adr = (adr >> 1) & 7;
	switch (adr) {
		case 0:
		case 1: res = upd7220_rd(comp->vid, comp->vid->txt7220, adr); break;
		case 2: break;
		case 3: break;
		case 4: break;
		case 5: break;
		case 6: break;
		case 7: break;
	}
	return res;
}

void pc98xx_gdc_wr(Computer* comp, int adr, int val) {

}

// uPD7220 - video(gra)
// a0 rd:gdc status	wr:gdc par
// a2 rd:gdc data	wr:gdc com
// a4 wr:displaying access mode - b0:plane 1/0
// a6 wr:drawing access mode - b0:plane 1/0
// a8,aa,ac,ae: palete registers

// egc: 04A.xxx.0
// kanjirom: 00A.xxx.1

int pc98xx_gra_rd(Computer* comp, int adr) {
	int res = -1;
	adr = (adr >> 1) & 7;
	switch (adr) {
		case 0:
		case 1: res = upd7220_rd(comp->vid, comp->vid->grf7220, adr); break;
		case 2: break;
		case 3: break;
		case 4: break;
		case 5: break;
		case 6: break;
		case 7: break;
	}
	return res;
}

void pc98xx_gra_wr(Computer* comp, int adr, int val) {

}


// uPD765 - FDC x 2
// 80,82: 5" flop
// 90,92,94,96: 1MB flop
// uPD8251 - RS232

// uPD8255: mouse
// 7fe9 rd: mouse data: left.x.right.x.md3..0
// 7fed wr: control flags: hc.sxy.shl.!int.x.x.x.x;
// 7fef wr:	1xxxxxxx: wr mode	rd:93?
//		0xxxnnnd: wr flag (111:hc, 100:!int)
int pc98xx_mou_rd(Computer* comp, int adr) {
	int res = 0;
	switch((adr >> 1) & 3) {
		case 0: res = 0;
			break;
		default:
			break;
	}
	return res;
}

void pc98xx_mou_wr(Computer* comp, int adr, int val) {

}

// uPD8255 - printer
// 40 wr data
// 42 rd 1.0.mod.sw1-3.sw1-8.!bsy.acpu.0
//	mod: system clock 0:10MHz, 1:8MHz - how?
//	sw1-3: printer display on/off
//	sw1-8: color depth. 1:16col, 0:8col
//	acpu: 1 for V30, 0 for others
// 44 rw pstb.0.0.0.0.0.0.0
// 46 rd 82
// 46 wr 8255 control word/mode
//	b7:!pstb

int pc98xx_prn_rd(Computer* comp, int adr) {
	int res = 0;
	printf("prn rd %.4X\n", adr);
	switch ((adr >> 1) & 3) {
		case 0: break;
		case 1: res = 0xa4;
			if (comp->DIPSW1 & 0x08) res |= 0x10;
			if (comp->DIPSW1 & 0x80) res |= 0x08;
			break;
		case 2: break;
		case 3: res = 0x82;
			break;
	}
	return res;
}

void pc98xx_prn_wr(Computer* comp, int adr, int val) {
	switch ((adr >> 1) & 3) {
		case 0:		// printer data
			break;
		case 2:		// b7:pstb
			break;
		case 3:		// 0E:pstb on, 0F:pstb off
			break;
	}
}

// uPD7261 - HDD controller
// fm sound (Yamaha 2203 ?)

// nmi flipflop (A1):
void pc98xx_nmi_wr(Computer* comp, int adr, int val) {
	comp->flgNMI = !!(adr & 2);
}

// deBUG
int pc98xx_dbg_rd(Computer* comp, int adr) {
	printf("pc98xx: ird %X\n", adr);
	comp_irq(IRQ_BRK, comp);
	return 0x00;
}

void pc98xx_dbg_wr(Computer* comp, int adr, int val) {
	printf("pc98xx: iwr %X, %X\n", adr, val);
	comp_irq(IRQ_BRK, comp);
}

// TODO: fill this table
// ports from 'pc9801 programmers bible'
// NOTE: run with --panic argument to exit on unknown i/o address
// 439 - ???
// 43d - switch rom pages ???
// NOTE: check this:
// 000..3FF is copies of 00..FF
// 400..FFF is copies of 000..3FF for ext.devices
// 1000..FFFF is copies of 000..FFF
xPort pc98xx_io_map[] = {
	{0xcf1, 0x000, 2, 2, 2, pc98xx_pic_rd,	pc98xx_pic_wr},		// 00,02,08,0a PIC, b3:slave, b1:A0
	{0xce1, 0x001, 2, 2, 2, pc98xx_dma_rd,	pc98xx_dma_wr},		// upd8237, dma controller
	{0xcf1, 0x020, 2, 2, 2, NULL,		pc98xx_rtc_wr},		// 20: upd4990,rtc
	{0xcf1, 0x021, 2, 2, 2, pc98xx_dma_rd,	pc98xx_dma_wr},		// 21,23,25,27: dma bank
//	{0xcf1, 0x030, 2, 2, 2, NULL,		NULL},			// 30,32: rs232 on upd8251
	{0xcf1, 0x031, 2, 2, 2, pc98xx_sys_rd,	pc98xx_sys_wr},		// 31,33,35,37: system ports on upd8255
	{0xcf1, 0x040, 2, 2, 2, pc98xx_prn_rd,	pc98xx_prn_wr},		// 40,42,44,46: printer on upd8255
	{0xcf1, 0x041, 2, 2, 2, pc98xx_kbd_rd,	pc98xx_kbd_wr},		// 41,43: keyboard on upd8251
	{0xcf1, 0x050, 2, 2, 2, NULL,		pc98xx_nmi_wr},		// 50,52: NMI controller: wr:A1 = nmi on/off
	{0xcf1, 0x060, 2, 2, 2, pc98xx_gdc_rd,	pc98xx_gdc_wr},		// 60,62:text gdc, 64,66,68,6a,6c,6e: crtc
//	{0xcf1, 0x070, 2, 2, 2, NULL,		NULL},			// 70,72,74,76,78,7a,7c,7e: crtc, grcg
	{0xcf1, 0x071, 2, 2, 2, pc98xx_pit_rd,	pc98xx_pit_wr},		// 71,73,75,77: PIT on upd8253
//	{0x0fd, 0x080, 2, 2, 2, NULL,		NULL},			// 80,82: hard disk inerface
//	{0xff9, 0x188, 2, 2, 2, NULL,		NULL},			// 188,18a,18c,18e: ym2203
//	{0x0f9, 0x089, 2, 2, 2, NULL,		NULL},			// 89,8b,8d,8f: network interface
//	{0x0f1, 0x090, 2, 2, 2, NULL,		NULL},			// 90,92,94,96: hd fdd on upd765
//	{0x0fd, 0x099, 2, 2, 2, NULL,		NULL},			// 99,9b: gp-ib switch
	{0xcf1, 0x0a0, 2, 2, 2, pc98xx_gra_rd,	pc98xx_gra_wr},		// a0,a2,a4,a6,a8,aa,ac,ae: graphic video upd7220a
//	{0xcf1, 0x1a0, 2, 2, 2, NULL,		NULL},			// ecg
//	{0xff1, 0x9a0, 2, 2, 2, NULL,		NULL},			// graphic control
//	{0xcf1, 0x0a1, 2, 2, 2, NULL,		NULL},			// kanjirom
//	{0x0f0, 0x0b0, 2, 2, 2, NULL,		NULL},			// b0..bf: communication controller / rs232 expansion interface; 0xbe: fdd switcher
//	{0x0f9, 0x0c8, 2, 2, 2, NULL,		NULL},			// c8,ca,cc,ce: dd drive on upd765
//	{0x0f1, 0x0c1, 2, 2, 2, NULL,		NULL},			// gp-ib on upd7210
	{0xfff9,0x7fd9,2, 2, 2, pc98xx_mou_rd,	pc98xx_mou_wr},		// 7fd9/b/d/f: mouse controller on upd8255
//	{0xfffb,0x3fdb,2, 2, 2, NULL,		NULL},			// 3fdb/f: upd8253 (timer for beeper)
//	{0xffff,0xbfdb,2, 2, 2, NULL,		NULL},			// bfdb: mouse interrupt ?
//	{0xcf9, 0x0f0, 2, 2, 2, NULL,		NULL},			// f0,f2,f4,f6: cpu ?
//	{0xcf8, 0x0f8, 2, 2, 2, NULL,		NULL},			// f8..ff: ndp (x87)
	{0x000, 0x000, 2, 2, 2, pc98xx_dbg_rd,	pc98xx_dbg_wr}
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

void pc98xx_init(Computer* comp) {
	ppi_set_cb(comp->ppi, comp, pc98xx_ppia_rd, pc98xx_ppia_wr, pc98xx_ppib_rd, pc98xx_ppib_wr, pc98xx_ppic_rd, pc98xx_ppich_wr, pc98xx_ppic_rd, pc98xx_ppicl_wr);
}

sndPair pc98xx_vol(Computer* comp, sndVolume* vol) {
	sndPair v;
	v.left = 0;
	v.right = 0;
	return v;
}
