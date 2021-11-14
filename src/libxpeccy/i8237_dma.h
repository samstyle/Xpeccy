#pragma once

enum {
	DMA_CR = 0,	// command register
	DMA_RR,		// request register
	CMA_CMR,	// channel mask register
	DMA_MR,		// mode reigster
	DMA_BTR,	// byte trigger reset
	DMA_RES,	// controller reset
	DMA_MRES,	// mask reset
	DMA_WAMR,	// write all masks register
	DMA_CH_BAR,	// channel bar/car (base address)
	DMA_CH_BWCR,	// cnannel bwcr/cwr (base counter)
	DMA_CH_PAR	// channel page register (b16+ of address)
};

#define DMA_SR		DMA_CR
#define DMA_CH_CAR	DMA_CH_BAR
#define DMA_CH_CWR	DMA_CH_BWCR

typedef struct {
	unsigned short bar;	// base address
	unsigned short car;	// current address
	unsigned char par;	// page address register (high 4/8 bits of address)
	unsigned short bwr;	// base counter
	unsigned short cwr;	// current counter
	unsigned char mode;
	int(*rd)(int, void*);
	void(*wr)(int, int, void*);
} DMAChan;

typedef struct {
	unsigned wrd:1;		// 16-bit dma, ignore btr
	unsigned btr:1;		// byte trigger. 0=low, 1=high
	DMAChan ch[3];
	unsigned char cmr;	// mask
	unsigned char status;
	void* ptr;
} i8237DMA;

i8237DMA* dma_create(void*, int);
void dma_destroy(i8237DMA*);

void dma_wr(i8237DMA*, int reg, int ch, int val);
int dma_rd(i8237DMA*, int reg, int ch);
