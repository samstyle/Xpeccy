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

void dma_ch_res(DMAChan* ch) {
	ch->bar = 0;
	ch->car = 0;
	ch->bwr = 0;
	ch->cwr = 0;
	ch->par = 0;
	ch->masked = 1;
}

void dma_reset(i8237DMA* dma) {
	dma->state = DMA_IDLE;
	dma->btr = 0;
	dma_ch_res(&dma->ch[0]);
	dma_ch_res(&dma->ch[1]);
	dma_ch_res(&dma->ch[2]);
	dma_ch_res(&dma->ch[3]);
}

void dma_set_chan(i8237DMA* dma, int ch, cbdmard cr, cbdmawr cw) {
	if (ch < 0) return;
	if (ch > 3) return;
	dma->ch[ch].rd = cr;
	dma->ch[ch].wr = cw;
}

void dma_transfer(i8237DMA* dma) {
	int i;
	int b;
	DMAChan* ch;
	for(i = 0; i < 4; i++) {
		ch = &dma->ch[i];
		if (!ch->masked && (ch->cwr > 0)) {
			b = ch->rd ? ch->rd((ch->par << 16) | ch->car, dma->ptr) : 0xff;
			if (ch->wr)
				ch->wr((ch->par << 16) | ch->car, b, dma->ptr);
			if (ch->mode & 0x10) {
				ch->car--;
			} else {
				ch->car++;
			}
			ch->cwr--;
			// TODO: if (ch->cwr == 0) ...
		}
	}
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
		case DMA_CR:
			break;
		case DMA_RR:
			break;
		case CMA_CMR:		// channel mask register
			dma->ch[val & 3].masked = (val & 4) ? 1 : 0;
			break;
		case DMA_MR:		// mode reigster
			dma->ch[val & 3].mode = val & 0xfc;
			break;
		case DMA_BTR:		// byte trigger reset
			dma->btr = 0;
			break;
		case DMA_RES:		// controller reset
			dma_reset(dma);
			break;
		case DMA_MRES:		// mask reset
			dma->ch[0].masked = 0;
			dma->ch[1].masked = 0;
			dma->ch[2].masked = 0;
			dma->ch[3].masked = 0;
			break;
		case DMA_WAMR:		// write all masks register
			dma->ch[0].masked = (val & 1) ? 1 : 0;
			dma->ch[1].masked = (val & 2) ? 1 : 0;
			dma->ch[2].masked = (val & 4) ? 1 : 0;
			dma->ch[3].masked = (val & 8) ? 1 : 0;
			break;
		case DMA_CH_BAR:
			dma->ch[ch].bar = dma_wr_reg(dma, dma->ch[ch].bar, val) & 0xffff;
			break;
		case DMA_CH_BWCR:
			dma->ch[ch].bwr = dma_wr_reg(dma, dma->ch[ch].bwr, val) & 0xffff;
			break;
		case DMA_CH_PAR:
			dma->ch[ch].par = val & (dma->wrd ? 0xff : 0x0f);
			break;
	}
}

int dma_rd_reg(i8237DMA* dma, unsigned short val) {
	int res;
	if (dma->wrd) {
		 res = val & 0xffff;
	} else if (dma->btr) {
		res = (val >> 8) & 0xff;
	} else {
		res = val & 0xff;
	}
	dma->btr = !dma->btr;
	return res;
}

int dma_rd(i8237DMA* dma, int reg, int ch) {
	int res = -1;
	switch (reg) {
		case DMA_SR:
			res = 0;
			if (dma->ch[0].cwr == 0) res |= 1;	// b0..3: transfer completed
			if (dma->ch[1].cwr == 0) res |= 2;
			if (dma->ch[2].cwr == 0) res |= 4;
			if (dma->ch[3].cwr == 0) res |= 8;
			break;
		case DMA_CH_BAR:
			res = dma_rd_reg(dma, dma->ch[ch].bar);
			break;
		case DMA_CH_BWCR:
			res = dma_rd_reg(dma, dma->ch[ch].bwr);
			break;
		case DMA_CH_PAR:
			res = dma->ch[ch].par & (dma->wrd ? 0xff : 0x0f);
			break;
	}
	return res;
}
