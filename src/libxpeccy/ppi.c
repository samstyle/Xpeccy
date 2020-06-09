#include "ppi.h"

#include <stdlib.h>
#include <string.h>

PPI* ppi_create() {
	PPI* ppi = (PPI*)malloc(sizeof(PPI));
	if (ppi) {
		memset(ppi, 0x00, sizeof(PPI));
		ppi_reset(ppi);
	}
	return ppi;
}

void ppi_delete(PPI* ppi) {
	if (ppi)
		free(ppi);
}

void ppi_chan_reset(ppiChan* ch) {
	ch->dir = PPI_OFF;
	ch->val = 0xff;
}

void ppi_reset(PPI* ppi) {
	ppi_chan_reset(&ppi->a);
	ppi_chan_reset(&ppi->b);
	ppi_chan_reset(&ppi->ch);
	ppi_chan_reset(&ppi->cl);
	ppi->ctrl = 0xff;
}

int ppi_rd(PPI* ppi, int adr) {
	int res = -1;
	int tmp;
	switch(adr & 3) {
		case 0:
			if (ppi->a.dir == PPI_IN)
				res = ppi->a.rd ? ppi->a.rd(ppi->ptr) : 0xff;
			break;
		case 1:
			if (ppi->b.dir == PPI_IN)
				res = ppi->b.rd ? ppi->b.rd(ppi->ptr) : 0xff;
			break;
		case 2:
			res = (ppi->ch.dir == PPI_IN) ? (ppi->ch.rd ? ppi->ch.rd(ppi->ptr) & 0xf0 : 0xf0) : 0xf0;
			tmp = (ppi->cl.dir == PPI_IN) ? (ppi->cl.rd ? ppi->cl.rd(ppi->ptr) & 0x0f : 0x0f) : 0x0f;
			res |= tmp;
			break;
	}
	return res;
}

void ppi_wr(PPI* ppi, int adr, int val) {
	int mask;
	switch(adr & 3) {
		case 0: if (ppi->a.wr && (ppi->a.dir == PPI_OUT)) ppi->a.wr(val, ppi->ptr);
			break;
		case 1: if (ppi->b.wr && (ppi->b.dir == PPI_OUT)) ppi->b.wr(val, ppi->ptr);
			break;
		case 2: if (ppi->ch.wr && (ppi->ch.dir == PPI_OUT)) ppi->ch.wr(val & 0xf0, ppi->ptr);
			if (ppi->cl.wr && (ppi->cl.dir == PPI_OUT)) ppi->cl.wr(val & 0x0f, ppi->ptr);
			break;
		case 3:
			if (val & 0x80) {
				ppi->ctrl = val & 0xff;
				ppi->a.dir = (val & 0x10) ? PPI_IN : PPI_OUT;
				ppi->b.dir = (val & 0x02) ? PPI_IN : PPI_OUT;
				ppi->cl.dir = (val & 0x01) ? PPI_IN : PPI_OUT;
				ppi->ch.dir = (val & 0x08) ? PPI_IN : PPI_OUT;
				ppi->a.mode = (val & 0x40) ? 2 : ((val & 0x20) ? 1 : 0);
				ppi->ch.mode = ppi->a.mode;
				ppi->b.mode = (val & 4) ? 1 : 0;
				ppi->cl.mode = ppi->b.mode;
				ppi->a.val = 0;
				ppi->b.val = 0;
				ppi->cl.val = 0;
			} else {
				mask = (1 << ((val & 0xe0) >> 1));	// bit mask
				if (val & 1) {		// set
					ppi_wr(ppi, 2, (ppi->cl.val | ppi->ch.val) | mask);
				} else {
					ppi_wr(ppi, 2, (ppi->cl.val | ppi->ch.val) & ~mask);
				}
			}
			break;
	}
}
