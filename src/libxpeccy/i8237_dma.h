#pragma once

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
	DMAChan ch[3];
	unsigned char cmr;	// mask
	unsigned char status;
	void* ptr;
} i8237DMA;

i8237DMA* dma_create(void*);
void dma_destroy(i8237DMA*);
