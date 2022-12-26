#include "i8237_dma.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

i8237DMA* dma_create(void* p, int w) {
	i8237DMA* dma = (i8237DMA*)malloc(sizeof(i8237DMA));
	if (dma) {
		memset(dma, 0, sizeof(i8237DMA));
		dma->ptr = p;
		dma->wrd = w;
		for (int i = 0; i < 4; i++) {
			dma->ch[i].wrd = w;
			dma->ch[i].blk = 0;
		}
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
	ch->hold = 0;
}

void dma_reset(i8237DMA* dma) {
	dma->state = DMA_IDLE;
	dma->btr = 0;
	dma->en = 1;
	dma_ch_res(&dma->ch[0]);
	dma_ch_res(&dma->ch[1]);
	dma_ch_res(&dma->ch[2]);
	dma_ch_res(&dma->ch[3]);
}

// set callbacks to read/wr device for one channel
void dma_set_chan(i8237DMA* dma, int ch, cbdmadrd cr, cbdmadwr cw) {
	ch &= 3;
	dma->ch[ch].rd = cr;
	dma->ch[ch].wr = cw;
}

// set callbacks for read/wr memory for all channels
void dma_set_cb(i8237DMA* dma, cbdmamrd cr, cbdmamwr cw) {
	for(int i = 0; i < 4; i++) {
		dma->ch[i].mrd = cr;
		dma->ch[i].mwr = cw;
	}
}

// mode b6,7: 00:by request, 01:single, 10:block, 11:cascade
// dma command reg: b0:mem-mem enable (ch0-ch1), b1:hold addres of ch0 (filling)
void dma_ch_count(DMAChan* ch) {
	if (!ch->hold)
		ch->car += (ch->mode & 0x20) ? -1 : 1;
	ch->cwr--;
	if (ch->cwr == -1) {	// counter is 1 less than bytes must be sended
		ch->masked = 1;
		if (ch->mode & 0x10) {	// auto restore
			ch->car = ch->bar;
			ch->cwr = ch->bwr;
		}
	}
}

// TODO: dma2 channels read/write by 2 bytes (ch->wrd == 1)
// TODO: mem->dev: channel reads data from mem to buffer and checks device is ready to get it (dec counter and clear buffer if it is)
void dma_ch_transfer(DMAChan* ch, void* ptr) {
	if (ch->masked) return;			// channel masked, no transfer
	int flag = 0;
	int b;
	switch((ch->mode >> 2) & 3) {
		case 0:		// verify. just read from dev, not write to mem?
			b = ch->rd ? ch->rd(ptr, &flag) : -1;
			break;
		case 1:		// dev->mem
			b = ch->rd ? ch->rd(ptr, &flag) : -1;
			if (flag && ch->mwr)
				ch->mwr((ch->par << 16) | ch->car, b, ch->wrd, ptr);
			break;
		case 2:		// mem->dev
			b = ch->mrd ? ch->mrd((ch->par << 16) | ch->car, ch->wrd, ptr) : -1;	// TODO: check dev is ready first?
			if (ch->wr)
				ch->wr(b, ptr, &flag);
			break;
		case 3:		// not allowed
			break;
	}
	if (flag)		// if transfer is successful
		dma_ch_count(ch);
}

void dma_transfer(i8237DMA* dma) {
	if (dma->en) {
		if (!dma->ch[0].blk) dma_ch_transfer(&dma->ch[0], dma->ptr);
		if (!dma->ch[1].blk) dma_ch_transfer(&dma->ch[1], dma->ptr);
		if (!dma->ch[2].blk) dma_ch_transfer(&dma->ch[2], dma->ptr);
		if (!dma->ch[3].blk) dma_ch_transfer(&dma->ch[3], dma->ptr);
	}
}

void dma_sync(i8237DMA* dma, int ns) {
	dma->ns += ns;
	while (dma->ns > 0) {
		dma->ns -= 200;		// 5MHz ~ 200ns/tick
		dma_transfer(dma);
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
			dma->ch[0].hold = !(~val & 3);		// mem-mem && hold address
			dma->en = !!(val & 4);
			printf("dma mem-mem: %i\n", val & 3);
			break;
		case DMA_RR:
			break;
		case DMA_CMR:		// channel mask register
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
			if (dma->wrd || !dma->btr) dma->ch[ch].car = dma->ch[ch].bar;
			break;
		case DMA_CH_BWCR:
			dma->ch[ch].bwr = dma_wr_reg(dma, dma->ch[ch].bwr, val) & 0xffff;
			if (dma->wrd) dma->ch[ch].bwr <<= 1;
			if (dma->wrd || !dma->btr) dma->ch[ch].cwr = dma->ch[ch].bwr;
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
			if (dma->ch[0].cwr == -1) res |= 1;	// b0..3: transfer completed
			if (dma->ch[1].cwr == -1) res |= 2;
			if (dma->ch[2].cwr == -1) res |= 4;
			if (dma->ch[3].cwr == -1) res |= 8;
			break;
		case DMA_CH_BAR:
			res = dma_rd_reg(dma, dma->ch[ch].bar);
			break;
		case DMA_CH_BWCR:
			res = dma_rd_reg(dma, dma->ch[ch].bwr);
			if (dma->wrd) res >>= 1;
			break;
		case DMA_CH_PAR:
			res = dma->ch[ch].par & (dma->wrd ? 0xff : 0x0f);
			break;
	}
	return res;
}
