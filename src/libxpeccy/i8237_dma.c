#include "i8237_dma.h"

#include <stdlib.h>
#include <string.h>

i8237DMA* dma_create(void* p, int w) {
	i8237DMA* dma = (i8237DMA*)malloc(sizeof(i8237DMA));
	if (dma) {
		memset(dma, 0, sizeof(i8237DMA));
		dma->ptr = p;
		dma->wrd = w;
	}
	return dma;
}

void dma_destroy(i8237DMA* dma) {
	if (dma) free(dma);
}

int dma_wr_reg(i8237DMA* dma, int oldval, int val) {
	if (dma->wrd) {
		oldval = val & 0xffff;
	} else if (dma->btr) {
		oldval = (oldval & 0xff) | ((val << 8) & 0xff00);
	} else {
		oldval = (oldval & 0xff00) | (val & 0xff);
	}
	dma->btr = !dma->btr;
	return oldval;
}

void dma_wr(i8237DMA* dma, int reg, int ch, int val) {
	ch &= 3;
	switch (reg) {
		case DMA_CH_BAR:
			dma->ch[ch].bar = dma_wr_reg(dma, dma->ch[ch].bar, val) & 0xffff;
			break;
		case DMA_CH_BWCR:
			dma->ch[ch].bwr = dma_wr_reg(dma, dma->ch[ch].bwr, val) & 0xffff;
			break;
		case DMA_CH_PAR:
			dma->ch[ch].par = val & (dma->wrd ? 0xff : 0x0f);
	}
}

int dma_rd(i8237DMA* dma, int reg, int ch) {
	int res = -1;
	return res;
}
