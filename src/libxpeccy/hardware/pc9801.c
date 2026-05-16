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

#include "../cpu/x86/i80286.h"
#include "../video/upd7220.h"

#define regCol	reg[49]		// current color (16bit mode)

#define regKBDm reg[50]		// mode
#define regKBDc reg[51]		// command
#define regKBDs	reg[52]		// status (oe,fe,pe)
#define regKBDd	reg[53]		// last transfered byte

#define fntSL	reg[55]		// kanjirom data position
#define fntSH	reg[56]
#define fntLN	reg[57]

//#define portA	reg[64] = DIPSW1
#define portB	reg[65]
#define portC	reg[66]
#define DIPSW1	reg[71]
#define DIPSW2	reg[72]
#define DIPSW3	reg[73]
#define MEMSW(_n) reg[73+(_n)]	// 73-76, 6 switches

#define flgNMI	flag[0]
#define flgKBDm flag[1]		// upd8251 mode (0:mode, 1:command)
#define flgVSync flag[2]	// VSync flip-flop: set @ vsync, reset on port 64 write. 0 means vsync interrupt allowed
#define flg16col flag[3]	// video: 16 colors mode

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
	Computer* comp = (Computer*)p;
	int r = comp->vid->ram[adr & 0x1ffff] & 0xff;
	switch (adr) {
		case 0xa3fe2:
		case 0xa3fe6:
		case 0xa3fea:
		case 0xa3ff2:
		case 0xa3ff6:
			printf("%X : rd SWx: %X = %.2X\n", comp->cpu->cs.base + comp->cpu->oldpc, adr, r);
			break;
		case 0xa3fee:
			r = 0x00;
			if (comp->ts->rom.data != NULL) r |= 8;		// sound rom
			break;
	}
	return r;
}

void pc98xx_vid_wr(int adr, int val, void* p) {
	Computer* comp = (Computer*)p;
	switch(adr) {
		case 0xa3fe2:
		case 0xa3fe6:
		case 0xa3fea:
		case 0xa3fee:
		case 0xa3ff2:
		case 0xa3ff6:
			printf("wr SWx: %X,%.2X\n", adr, val);
			break;
	}
	comp->vid->ram[adr & 0x1ffff] = val;
}

int pc98xx_vex_rd(int adr, void* p) {
	Computer* comp = p;
	return comp->vid->ram[(adr & 0x7fff) + MEM_128K];
}

void pc98xx_vex_wr(int adr, int val, void* p) {
	Computer* comp = p;
	comp->vid->ram[(adr & 0x7fff) + MEM_128K] = val;
}

// bios
int pc98xx_bios_rd(int adr, void* p) {
	Computer* comp = (Computer*)p;
	adr -= 0xe8000;			// rom size is 96K (rounded to 128K), without this e8000 will be at 8000 in rom
	return comp->mem->romData[adr & comp->mem->romMask];
}

// sound board bios (cc000-cffff)
int pc98xx_snd_rd(int adr, void* p) {
	Computer* comp = p;
	return tsReadRom(comp->ts, adr & 0x3fff);
}

// adr bus width 20 bit
// max.adr = FFFFF (1MB)
// page size = 4096 bytes

// TODO:
// d4000-d5fff	gp/ib interface rom
// d0000-d3fff	rs232 rom
// cc000-cffff	sound board rom

// TODO:memory switches (see 'pc98 programmers bible')
// A3FE2/E3FE2:default 0x48
// b6,7	: stop bits (01:1, 10:1.5, 11:2)
// b5	: parity value
// b4	: parity check on
// b2,3	: data bits (10:7, 11:8)
// b1	: transmission method (0:full duplex, 1:half duplex)
// b0	: X parameter on (?)

// A3FE6/E3FE6:default 05
// b7	: S parameter on (?)
// b6	: return key (0:cr, 1:cr+lf)
// b5	: CR code (cr/cr+lf)
// b0..4: bodrate (0000: none, others: 75*2^(n-1): 75,150,300,600... 9600)

// A3FEA/E3FEA:default 04
// b7	: DEL code (0:term-08,io-7F, 1:00/00)
// b6	: initial text mode screen color (0:white, 1:black)
// b5	: cpu clock? (10 / 8 or 16 MHz)
// b4	: cpu is v30
// b3	: cpu is x86
// b0..2: memory size (128/256/384/512/640/768 KB)

// A3FEE/E3FEE:default 00
// b7	: expansion rom on CE000-CFFFF (X:user unavailable)
// b6	: expansion rom on CA000-CBFFF (X:user unavailable)
// b5	: GP/IB rom @ D4000-D5FFF (0:off, 1:on)
// b4	: RS232/BRANCH4670 rom @ D0000-D3FFF
// b3	: sound rom @ CC000-CFFFF
// b2	: expansion rom @ C8000-C9FFF
// b0,1	: 00

// A3FF2/E3FF2:default 01
// b4..7: floppy disks
// b3	: screen hard copy (0:b/w, 1:color)
// b2	:
// b1	:
// b0	: printer present

// A3FF6:default 00
// b6,7	: unused
// b5	: phone control
// b4	: extend function control
// b3	: memory mode
// b0..2: unused

// TODO: highres mode memory map is different

void pc98xx_mem_map(Computer* comp) {
	memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_1M, pc98xx_ram_rd, pc98xx_ram_wr, comp);	// ram (1M)
	memSetBank(comp->mem, 0xa0, MEM_EXT, 0, MEM_128K, pc98xx_vid_rd, pc98xx_vid_wr, comp);	// video ram (128K)

	memSetBank(comp->mem, 0xc0, MEM_EXT, 0, MEM_128K, NULL, NULL, comp);			// expand roms (128K), reserved
	memSetBank(comp->mem, 0xcc, MEM_ROM, 1, MEM_16K, pc98xx_snd_rd, NULL, comp);		// sound.rom (16K)

	memSetBank(comp->mem, 0xe0, MEM_EXT, 0, MEM_32K, pc98xx_vex_rd, pc98xx_vex_wr, comp);	// video ram (32K)
	memSetBank(comp->mem, 0xe8, MEM_ROM, 0, MEM_64K, pc98xx_bios_rd, NULL, comp);		// bios (96K)
	memSetBank(comp->mem, 0xf8, MEM_ROM, 2, MEM_32K, pc98xx_bios_rd, NULL, comp);
}

void pc98xx_reset(Computer* comp) {
	cpu_reset(comp->cpu);
	comp->DIPSW1 = 0x01;
	comp->DIPSW2 = 0x03;
	comp->DIPSW3 = 0xa0;
	comp->portB = 0x00;	// b3:1=highres
	comp->portC = 0xff;
	comp->flgKBDm = 0;
	comp->flgVSync = 1;	// disabled
	comp->regKBDs = 0;
	pc98xx_mem_map(comp);
	kbd_set_type(comp->keyb, KBD_NEC98XX);
	vid_set_mode(comp->vid, VID_PC98XX);
	vid_set_resolution(comp->vid, 640, 400);
	comp->vid->linedbl = !(comp->portB & 8);
	dma_reset(comp->dma1);
	pit_reset(comp->pit);
	pic_reset(comp->mpic);
	pic_reset(comp->spic);
}

// DIPSW1
// b0	display clk - 1:24KHz, 0:15KHz (system port)
// b1	superimpose mode
// b2	digital display (printer port)
// b3	if on, switch floppies 1-2/3-4 (default: 1-2 internal, 3-4 external)
// b4,5	rs232 transfer mode (mouse port)
// b6	-
// b7	1:16col, 0:8col mode (printer port)

// DIPSW2
// b0	-
// b1	1:terminal mode, 0:basic mode
// b2	1:80 0:40 chars/line
// b3	1:25 0:20 char lines/screen
// b4	1:clear ram on reset
// b5	1:disable internal hdd
// b6	-
// b7	gdc dot clk: 1:5mhz, 0:2.5MHz	(640/320 dots/line)

// DIPSW3
// b0	1:auto detect fdd type (by default see b1)
// b1	1:fdd is dd, 0:fdd is hd
// b2	-
// b3	-
// b4	-
// b5	memory 1:512K, 0:640K (mouse port)
// b6	-
// b7	cpu 1:x86, 0:v30 (printer port)

// uPD8255 - misc data (rs232,soft reset,misc flags) (31,33,35,37)
// portA - system switch, DIPSW2 full (ro) - see above
// portB (ro)	b0:CDAT (upd4990 data out)
//		b1:EMCK (ext.ram parity error)
//		b2:IMCK (int.ram parity error)
//		b3:CRTT (1:24KHz, 0:15KHz) - display hires, DIPSW1-0
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

int pc98xx_ppia_rd(void* comp) {return ((Computer*)comp)->DIPSW2;}
int pc98xx_ppib_rd(void* ptr) {
	Computer* comp = (Computer*)ptr;
	int res = comp->portB & 0xfe;
	if (comp->rtc->data) res |= 1;
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

#if 1

int uart_kbd_rd(void* p) {
	int res = kbd_rd((Keyboard*)p, 0);
	return res;
}

void uart_kbd_wr(int data, void* p) {
	kbd_wr((Keyboard*)p, 0, data);
}

int pc98xx_kbd_rd(Computer* comp, int adr) {
	return uart_rd(comp->uart, adr >> 1);
}

void pc98xx_kbd_wr(Computer* comp, int adr, int data) {
	uart_wr(comp->uart, adr >> 1, data);
}

#else

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

#endif
// uPD7220 - video(txt)
// text GDC
// == 60, 62 - upd7220 rd/wr
// 60:0	rd	b7:light pen detect
//		b6:hblank
//		b5:vsync
//		b4:dma execution in process
//		b3:drawing in process
//		b2:fifo buffer full
//		b1:fifo buffer empty
//		b0:data ready
// 60:0	wr	gdc parameter write
// 62:1	rd	gdc data read
// 62:1	wr	gdc command write

// 64:2	wr	crt interrupt reset
// 66:3		?
// 68:4 rd	[PC-H98, PC-9821, PC-9801BA2･BS2･BX2･BA3･BX3･BX4･NS/A]
//		b7: KAC mode (0:code access, 1:dot access)
//		b6: NVMW permit (0:write disabled, 1:write enabled)
//		b3: column width (0:80 symbols, 1:40 symbols)
//		b2: graph mode (0:color, 1:mono)
//		b0: atr4 (0:vertical line, 1:simple graf)
// 68:4	wr	mode flip-flop
//		b0:flag
//		b1,2,3:param.nr:
//		000:atr4 (1:graph / 0:vertical line) : b4 of text attribute
//		001:graph mode (1:mono / 0:color)
//		010:column size (1:40 / 0:80 chars)
//		011:font size (1:7x13 (400-line mode) / 0:6x8 (200-line mode))
//		100:88 graph mode (1:200 lines / 0:crt lines)
//		101:kanji access (1:dot mode / 0:code mode). dot mode: stop output chars, can read kanji data all time; code mode: display chars, allow kanji data access on VSync only
//		110:static memory (1:enable / 0:disable)	allow writing to text area 3FE2~3FFE (A000: @normal mode, E000: @highres)
//		111:allow display (1:enable / 0:disable)	text & graphic
// 6a:5	wr	mode flip-flop 2
//		b0:flag
//		b1-7:param nr:
//		0000000:color select (1:16col / 0:8col)
//		0000010:enhanced mode (1:enhanced / 0:compatible)
//		others:reserved
// 6c:6	wr	border color: b4,5,6 = b,r,g
// 6e:7		?

int pc98xx_gdc_rd(Computer* comp, int adr) {
	int res = -1;
	adr = (adr >> 1) & 7;
	switch (adr) {
		case 0:
		case 1: res = upd7220_rd(comp->vid, comp->vid->txt7220, adr); break;
		case 2: break;
		case 3: break;
		// 68: read
		case 4: break;
		case 5: break;
		case 6: break;
		case 7: break;
	}
	return res;
}

void pc98xx_gdc_wr(Computer* comp, int adr, int val) {
	adr = (adr >> 1) & 7;
	switch(adr) {
		case 0:
		case 1: upd7220_wr(comp->vid, comp->vid->txt7220, adr, val); break;
		// 64: wr any value will reset vsync int flip-flop, int0A will occur @ next vsync start
		case 2: comp->flgVSync = 0; break;
		// 66: text mode picture brightness (?)
		case 3: break;
		// 68: mode flip-flop 1
		case 4: switch((val >> 1) & 7) {
				case 0: break;						// atr sel
				case 1: break;						// graphic mode
				case 2: break;						// column width
				case 3: break;						// font sel
				case 4: printf("40/80 chars: %i\n", val & 1);
					comp->vid->vga.cpl = (val & 1) ? 40 : 80;
					break;// grp mode (chars / line)
				case 5: break;						// kac mode
				case 6: break;						// nvmw permit
				case 7: comp->vid->nogfx = !(val & 1); break;		// enable display
			}
			break;
		// 6a: mode flip-flop 2
		case 5: switch ((val >> 1) & 0x7f) {
				case 0: comp->flg16col = val & 1; break;	// color mode. 1:16col, 0:8col
				case 2: break;					// grcg / ecg
				// there are others [undocumented] flags on different pc98xx models
			}
			break;
		// 6c: border color
		case 6: comp->vid->brdcol = (val >> 4) & 7;break;
		// 6e: monitor refresh rate (higher models)
		case 7: break;
	}
}

// upd52611 - crt controller for text mode
// 70:0	wr	Line counter initial value (0)
// 72:1	wr	Character bodyface line number (15)
// 74:2	wr	Character Lines Number (15) - char height
// 76:3	wr	Smooth Scroll Lines Count
// 78:4	wr	Scroll Area Above Position Lines Number
// 7a:5	wr	Scroll Area Lines Number

// 7c:6	wr	Fast Graphics Mode Register:
//		b0..3: p0..3 disable
//		b6:rmw mode
//		b7:cg mode
// 7e:7	wr	Tile Register 0-3

int pc98xx_crt_rd(Computer* comp, int adr) {
	int res = -1;
	adr = (adr >> 1) & 7;
	switch (adr) {
		case 0: break;
		case 1: break;
		case 2: break;
		case 3: break;
		case 4: break;
		case 5: break;
		case 6: break;
		case 7: break;
	}
	return res;
}

void pc98xx_crt_wr(Computer* comp, int adr, int val) {
	adr = (adr >> 1) & 7;
	switch (adr) {
		case 0: comp->vid->regCharBegin = val & 0xff; break;
		case 1: comp->vid->regCharEnd = val & 0xff; break;
		case 2: comp->vid->regCharHeight = val & 0xff; break;
		case 3: break;
		case 4: break;
		case 5: break;
		case 6: break;
		case 7: break;
	}
}

// uPD7220 - video(gra)
// a0 rd:gdc status	wr:gdc par
// a2 rd:gdc data	wr:gdc com
// a4 wr:displaying access mode - b0:plane 1/0
// a6 wr:drawing access mode - b0:plane 1/0

// a8,aa,ac,ae: palette registers
// 8bit mode: a8 - cols 3,7 (0bgr.0bgr), aa - cols 2,6, ac - cols 1,5, ae - cols 0,4
// 16bit mode: a8 bit 0..3 = set col number, aa,ac,ae = G,R,B level (bit 2..0) for selected color

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

xColor col_conv(int val) {
	xColor col;
	col.b = (val & 0x04) ? 0xa0 : 0x00;
	col.g = (val & 0x02) ? 0xa0 : 0x00;
	col.r = (val & 0x01) ? 0xa0 : 0x00;
	return col;
}

void pc98xx_gra_wr(Computer* comp, int adr, int val) {
	adr = (adr >> 1) & 7;
	switch(adr) {
		case 0:
		case 1: upd7220_wr(comp->vid, comp->vid->grf7220, adr, val); break;
		case 2: break;
		case 3: break;
		// palette
		case 4: if (comp->flg16col) {
				comp->regCol = val & 15;
			} else {
				vid_set_col(comp->vid, 7, col_conv(val));
				vid_set_col(comp->vid, 3, col_conv(val >> 4));
			}
			break;
		case 5: if (comp->flg16col) {
				vid_set_green(comp->vid, comp->regCol & 0x0f, (val & 0x0f) << 4);
			} else {
				vid_set_col(comp->vid, 6, col_conv(val));
				vid_set_col(comp->vid, 2, col_conv(val >> 4));
			}
			break;
		case 6: if (comp->flg16col) {
				vid_set_blue(comp->vid, comp->regCol & 0x0f, (val & 0x0f) << 4);
			} else {
				vid_set_col(comp->vid, 5, col_conv(val));
				vid_set_col(comp->vid, 1, col_conv(val >> 4));
			}
			break;
		case 7: if (comp->flg16col) {
				vid_set_red(comp->vid, comp->regCol & 0x0f, (val & 0x0f) << 4);
			} else {
				vid_set_col(comp->vid, 4, col_conv(val));
				vid_set_col(comp->vid, 0, col_conv(val >> 4));
			}
			break;
	}
}

// kanji font
// A1	wr	symbol code 2nd byte
// A3	wr	symbol code 1st byte
// A5	wr	b0..4:line, b5:1=left byte,0=right byte
// A7	--
// A9	rd/wr	symbol data in position chosed by a1,a3,a5
// AB	--
// AD	--
// AF	--

int pc98xx_fnt_rd(Computer* comp, int adr) {
	int res = -1;
	adr = (adr >> 1) & 7;
	if (adr == 4) {
		adr = (comp->fntSL << 5) | (comp->fntSH << 13);	// 32 bytes/char
		adr |= ((comp->fntLN & 0x0f) << 1);		// rc4 is ignored, 16 lines only
		if (!(comp->fntLN & 0x20)) adr++;
		res = vid_fnt_rd(comp->vid, adr);
	}
	return res;
}

void pc98xx_fnt_wr(Computer* comp, int adr, int val) {
	adr = (adr >> 1) & 7;
	switch (adr) {
		case 0: comp->fntSH = val & 0xff; break;
		case 1: comp->fntSL = val & 0xff; break;
		case 2: comp->fntLN = val & 0x3f; break;
		case 4:
			adr = (comp->fntSL << 5) | (comp->fntSH << 13);
			adr |= ((comp->fntLN & 0x0f) << 1);
			if (!(comp->fntLN & 0x20)) adr++;
			vid_fnt_wr(comp->vid, adr, val);
			break;
	}
}

// uPD765 - FDC x 2
// 80,82: 5" flop
// 90,92,94,96: 1MB flop
// uPD8251 - RS232

// 51,53,55,57 - 320KB FDD on 8255
int pc98xx_f55_rd(Computer* comp, int adr) {
	return -1;
}

void pc98xx_f55_wr(Computer* comp, int adr, int val) {

}

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

// uPD8251 (uart) - rs232

int pc98xx_rs_rd(Computer* comp, int adr) {
	return -1;
}

void pc98xx_rs_wr(Computer* comp, int adr, int val) {

}

// uPD8255 - printer
// 40 wr data
// 42 rd 1.0.mod.sw1-3.sw1-8.!bsy.sw3-7.0
//	mod: system clock 0:10MHz, 1:8MHz - how?
//	sw1-3: printer display on/off
//	sw1-8: color depth. 1:16col, 0:8col
//	sw3-7: 1 for V30, 0 for others
// 44 rw pstb.0.0.0.0.0.0.0
// 46 rd 82
// 46 wr 8255 control word/mode
//	b7:!pstb

int pc98xx_prn_rd(Computer* comp, int adr) {
	int res = 0;
	switch ((adr >> 1) & 3) {
		case 0: break;
		case 1: res = 0xa4;
			if (comp->DIPSW1 & 0x08) res |= 0x10;
			if (comp->DIPSW1 & 0x80) res |= 0x08;
			if (comp->DIPSW3 & 0x80) res |= 0x02;
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

// c0,c2,c4,c6

int pc98xx_c0_rd(Computer* comp, int adr) {
	return -1;
}

void pc98xx_c0_wr(Computer* comp, int adr, int val) {

}

// uPD7261 - HDD controller

void pc98xx_hdd_wr(Computer* comp, int adr, int val) {

}

int pc98xx_hdd_rd(Computer* comp, int adr) {
	return -1;
}

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
// 439 - DMA access [PC98XA on i80286]
// 43d - switch rom pages ???
// 43f - misc memory managment
// 461 - ram window maping. [0x80000-0x9ffff - address bits 23-17 replaced with value bits 7-1]
// 467 wr: b1 = highres mode [undocumented]
// NOTE: check this:
// 000..3FF is copies of 00..FF
// 400..FFF is copies of 000..3FF for ext.devices
// 1000..FFFF is copies of 000..FFF
xPort pc98xx_io_map[] = {
	{0xcf1, 0x000, 2, 2, 2, pc98xx_pic_rd,	pc98xx_pic_wr},		// 00,02,08,0a PIC, b3:slave, b1:A0
	{0xce1, 0x001, 2, 2, 2, pc98xx_dma_rd,	pc98xx_dma_wr},		// upd8237, dma controller
	{0xcf1, 0x020, 2, 2, 2, NULL,		pc98xx_rtc_wr},		// 20: upd4990,rtc
	{0xcf1, 0x021, 2, 2, 2, pc98xx_dma_rd,	pc98xx_dma_wr},		// 21,23,25,27: dma bank
	{0xcf1, 0x030, 2, 2, 2, pc98xx_rs_rd,	pc98xx_rs_wr},		// 30,32: rs232 on upd8251
	{0xcf1, 0x031, 2, 2, 2, pc98xx_sys_rd,	pc98xx_sys_wr},		// 31,33,35,37: system ports on upd8255
	{0xcf1, 0x040, 2, 2, 2, pc98xx_prn_rd,	pc98xx_prn_wr},		// 40,42,44,46: printer on upd8255
	{0xcf1, 0x041, 2, 2, 2, pc98xx_kbd_rd,	pc98xx_kbd_wr},		// 41,43: keyboard on upd8251
	{0xcf1, 0x050, 2, 2, 2, NULL,		pc98xx_nmi_wr},		// 50,52: NMI controller: wr:A1 = nmi on/off
	{0xcf1, 0x051, 2, 2, 2, pc98xx_f55_rd,	pc98xx_f55_wr},		// 51,53,55,57: 320KB FDD controller on 8255
	{0xcf1, 0x060, 2, 2, 2, pc98xx_gdc_rd,	pc98xx_gdc_wr},		// 60,62:text gdc, 64,66,68,6a,6c,6e: crtc
	{0xcf1, 0x070, 2, 2, 2, pc98xx_crt_rd,	pc98xx_crt_wr},		// 70,72,74,76,78,7a,7c,7e: crtc, grcg
	{0xcf1, 0x071, 2, 2, 2, pc98xx_pit_rd,	pc98xx_pit_wr},		// 71,73,75,77: PIT on upd8253
	{0x0fd, 0x080, 2, 2, 2, pc98xx_hdd_rd,	pc98xx_hdd_wr},		// 80,82: hard disk inerface
//	{0xff9, 0x188, 2, 2, 2, NULL,		NULL},			// 188,18a,18c,18e: ym2203
//	{0x0f9, 0x089, 2, 2, 2, NULL,		NULL},			// 89,8b,8d,8f: network interface
//	{0x0f1, 0x090, 2, 2, 2, NULL,		NULL},			// 90,92,94,96: hd fdd on upd765
//	{0x0fd, 0x099, 2, 2, 2, NULL,		NULL},			// 99,9b: gp-ib switch
	{0xcf1, 0x0a0, 2, 2, 2, pc98xx_gra_rd,	pc98xx_gra_wr},		// a0,a2,a4,a6,a8,aa,ac,ae: graphic video upd7220a
//	{0xcf1, 0x1a0, 2, 2, 2, NULL,		NULL},			// ecg
//	{0xff1, 0x9a0, 2, 2, 2, NULL,		NULL},			// graphic control
	{0xcf1, 0x0a1, 2, 2, 2, pc98xx_fnt_rd,	pc98xx_fnt_wr},		// a1,a3,a5,a9:kanjirom
//	{0x0f0, 0x0b0, 2, 2, 2, NULL,		NULL},			// b0..bf: communication controller / rs232 expansion interface; 0xbe: fdd switcher
	{0x0f9, 0x0c0, 2, 2, 2, pc98xx_c0_rd,	pc98xx_c0_wr},		// c0,c2,c4,c6,c8: oda printer board on 8255 [reserved]
//	{0x0f9, 0x0c8, 2, 2, 2, NULL,		NULL},			// c8,ca,cc,ce: dd fdd on upd765
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

// cpu: get int vector from pic
int pc98xx_ack(Computer* comp) {
	int t = pic_ack(comp->mpic);
	if (t < 0) t = pic_ack(comp->spic);
	return t;
}

void pc98xx_irq(Computer* comp, int id) {
	switch(id) {
		case IRQ_PIT_CH0:		// ch0: interval (master pic int0 -> cpu int8)
			pic_int(comp->mpic, 0);
			break;
		case IRQ_PIT_CH1: break;	// ch1: speaker NOTE:9801/E/F/M for memory refresh
		case IRQ_PIT_CH2: break;	// ch2: rs232 timer
		case IRQ_MASTER_PIC:
			comp->cpu->intrq |= I286_INT; break;
			break;
		case IRQ_SLAVE_PIC:		// slave pic is connected to int7 of master pic
			pic_int(comp->mpic, 7);
			break;
		case IRQ_VID_VBLANK:		// VBlank: if !flgVSync, master PIC interrupt #2
			if (!comp->flgVSync) {
				pic_int(comp->mpic, 2);
				comp->flgVSync = 1;
			}
			break;
		case IRQ_KBD_DATA:
		case IRQ_KBD_ACK:		// kbd data is ready
			uart_ready(comp->uart);
			break;
		case IRQ_UART_0:
			pic_int(comp->mpic, 1);
			break;
	}
}

void pc98xx_sync(Computer* comp, int ns) {
	pit_sync(comp->pit, ns);		// timers
	uart_sync(comp->uart, ns);		// kbd
}

// kbd uart: 19200bps = 2400Bps
void pc98xx_init(Computer* comp) {
	// 8255, system ports
	ppi_set_cb(comp->ppi, comp, pc98xx_ppia_rd, pc98xx_ppia_wr, pc98xx_ppib_rd, pc98xx_ppib_wr, pc98xx_ppic_rd, pc98xx_ppich_wr, pc98xx_ppic_rd, pc98xx_ppicl_wr);
	// keyboard uart
	uart_set_type(comp->uart, UPD_8251);
	uart_set_rate(comp->uart, 100);
	uart_set_irq(comp->uart, IRQ_UART_0);
	uart_set_dev(comp->uart, uart_kbd_rd, uart_kbd_wr, comp->keyb);
	// keyboard
	kbd_set_type(comp->keyb, KBD_NEC98XX);
}

sndPair pc98xx_vol(Computer* comp, sndVolume* vol) {
	sndPair v;
	v.left = 0;
	v.right = 0;
	return v;
}

// TODO: need to avoid xt_press/xt_release in MainWin::keyPressEvent/keyReleaseEvent

void pc98xx_keyp(Computer* comp, keyEntry* kent) {
	kbd_press(comp->keyb, kent);
}

void pc98xx_keyr(Computer* comp, keyEntry* kent) {
	kbd_release(comp->keyb, kent);
}

static vLayout pc98xxLay = {{720,412},{0,0},{80,12},{640,400},{0,0},1};		// check

HardWare p98_hw_core = {HW_PC9801,HWG_PC98XX,"PC-9801","NEC PC9801 (in progress)",16,MEM_1M,1.0,&pc98xxLay,20,NULL,
			pc98xx_init,pc98xx_mem_map,pc98xx_iowr,pc98xx_iord,pc98xx_mrd,pc98xx_mwr,pc98xx_irq,pc98xx_ack,pc98xx_reset,pc98xx_sync,pc98xx_keyp,pc98xx_keyr,pc98xx_vol};
