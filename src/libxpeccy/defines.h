#pragma once

// interrupt types
enum {
	IRQ_VID_INT = 1,// video
	IRQ_VID_INT_E,
	IRQ_VID_VBLANK,
	IRQ_VID_LINE,	// tsconf
	IRQ_RZX_INT,
	IRQ_DMA,
	IRQ_CPU_SYNC,	// sync cpu ticks
	IRQ_FDC,	// ibm
	IRQ_HDD_PRI,
	IRQ_SLAVE_PIC,
	IRQ_MASTER_PIC,
	IRQ_KBD,	// ibm kbd || gbc buttons
	IRQ_RESET,
	IRQ_MOUSE,
	IRQ_PIT_CH0,
	IRQ_PIT_CH1,
	IRQ_APU,	// nes
};

typedef void(*cbirq)(int, void*);

// memory size
#define MEM_256	(1<<8)
#define MEM_512	(1<<9)
#define MEM_1K	(1<<10)
#define MEM_2K	(1<<11)
#define MEM_4K	(1<<12)
#define MEM_8K	(1<<13)
#define MEM_16K	(1<<14)
#define MEM_32K	(1<<15)
#define MEM_64K	(1<<16)
#define MEM_128K	(1<<17)
#define MEM_256K	(1<<18)
#define MEM_512K	(1<<19)
#define MEM_1M	(1<<20)
#define MEM_2M	(1<<21)
#define MEM_4M	(1<<22)
