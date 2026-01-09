#include "i8255_ppi.h"

#include <stdlib.h>
#include <string.h>

int ppi_no_rd(void* p) {return -1;}
void ppi_no_wr(int v, void* p) {}

PPI* ppi_create() {
	PPI* ppi = (PPI*)malloc(sizeof(PPI));
	if (ppi) {
		memset(ppi, 0x00, sizeof(PPI));
		ppi_set_cb(ppi, NULL, ppi_no_rd, ppi_no_wr, ppi_no_rd, ppi_no_wr, ppi_no_rd, ppi_no_wr, ppi_no_rd, ppi_no_wr);
		ppi_reset(ppi);
	}
	return ppi;
}

void ppi_destroy(PPI* ppi) {
	if (ppi)
		free(ppi);
}

void ppi_set_cb(PPI* ppi, void* p, cbppird ar, cbppiwr aw, cbppird br, cbppiwr bw, cbppird chr, cbppiwr chw, cbppird clr, cbppiwr clw) {
	ppi->ptr = p;
	ppi->a.rd = ar;
	ppi->a.wr = aw;
	ppi->b.rd = br;
	ppi->b.wr = bw;
	ppi->ch.rd = chr;
	ppi->ch.wr = chw;
	ppi->cl.rd = clr;
	ppi->cl.wr = clw;
}

void ppi_chan_reset(ppiChan* ch) {
	ch->dir = PPI_IN;
	ch->val = 0x00;
}

void ppi_reset(PPI* ppi) {
	ppi_chan_reset(&ppi->a);
	ppi_chan_reset(&ppi->b);
	ppi_chan_reset(&ppi->ch);
	ppi_chan_reset(&ppi->cl);
	ppi_wr(ppi, 3, 0x9b);
}

int ppi_rd(PPI* ppi, int adr) {
	int res = -1;
	int tmp;
	switch(adr & 3) {
		case 0:
			res = (ppi->a.dir == PPI_IN) ? (ppi->a.rd ? ppi->a.rd(ppi->ptr) : 0xff) : ppi->a.val;
			break;
		case 1:
			res = (ppi->b.dir == PPI_IN) ? (ppi->b.rd ? ppi->b.rd(ppi->ptr) : 0xff) : ppi->b.val;
			break;
		case 2:
			res = ((ppi->ch.dir == PPI_IN) ? (ppi->ch.rd ? ppi->ch.rd(ppi->ptr) : 0xff) : ppi->ch.val) & 0xf0;
			tmp = ((ppi->cl.dir == PPI_IN) ? (ppi->cl.rd ? ppi->cl.rd(ppi->ptr) : 0xff) : ppi->cl.val) & 0x0f;
			res |= tmp;
			break;
	}
	return res;
}

void ppi_wr(PPI* ppi, int adr, int val) {
	int mask;
	switch(adr & 3) {
		case 0: if (ppi->a.dir == PPI_OUT) {
				ppi->a.val = val;
				if (ppi->a.wr)
					ppi->a.wr(val, ppi->ptr);
			}
			break;
		case 1: if (ppi->b.dir == PPI_OUT) {
				ppi->b.val = val;
				if (ppi->b.wr)
					ppi->b.wr(val, ppi->ptr);
			}
			break;
		case 2: if (ppi->ch.dir == PPI_OUT) {
				ppi->ch.val = val & 0xf0;
				if (ppi->ch.wr)
					ppi->ch.wr(val & 0xf0, ppi->ptr);
			}
			if (ppi->cl.dir == PPI_OUT) {
				ppi->cl.val = val & 0x0f;
				if (ppi->cl.wr)
					ppi->cl.wr(val & 0x0f, ppi->ptr);
			}
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
				ppi_wr(ppi, 0, ppi->a.val);
				ppi_wr(ppi, 1, ppi->b.val);
				ppi_wr(ppi, 2, (ppi->ch.val & 0xf0) | (ppi->cl.val & 0x0f));
			} else {
				mask = (1 << ((val >> 1) & 7));	// bit mask
				if (val & 1) {		// set
					ppi_wr(ppi, 2, (ppi->cl.val | ppi->ch.val) | mask);
				} else {
					ppi_wr(ppi, 2, (ppi->cl.val | ppi->ch.val) & ~mask);
				}
			}
			break;
	}
}
