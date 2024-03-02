#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "nvram.h"

nvRam* nvCreate() {
	nvRam* nv = (nvRam*)malloc(sizeof(nvRam));
	if (nv == NULL) return NULL;
	memset(nv,0x00,sizeof(nvRam));
	nv->mode = NV_IDLE;
	nv->sdc = 1;
	nv->sda = 1;
	return nv;
}

void nvDestroy(nvRam* nv) {
	free(nv);
}

void nvWr(nvRam* nv,int sda, int sdc, int wp) {
//	if (nv->flag & NV_TX) printf("%i:%i\n",sdc ? 1 : 0, sda ? 1 : 0);
	if (!nv->sdc && sdc) {			// sdc 0->1
		nv->stable = 1;
		if (nv->tx && !nv->ack) {	// if transmit but not ack read byte to nv->data
			if (nv->bitcount == 0) {
				nv->data = nv->mem[nv->adr & 0x7ff];
				nv->adr++;
				nv->bitcount = 8;
//				printf("RD %.2X\n",nv->data);
			}
		}
	} else if (nv->sdc && sdc) {		// sdc 1->1 : control START/STOP/stable
		if (nv->sda && !sda) {		// sda 1->0 : START
//			printf("START\n");
			nv->stable = 0;
			nv->tx = 0;
			nv->rx = 1;
			nv->mode = NV_COM;	// 1st byte is command
			nv->bitcount = 8;
		} else if (!nv->sda && sda) {	// sda 0->1 : STOP, write buffer to mem
//			printf("STOP\n");
			if ((nv->mode == NV_WRITE) && !wp) {
				nv->bufpos &= 0x0f;		// TODO : reset to 0 or limited by F?
				for (int i = 0; i < nv->bufpos; i++) {
					nv->mem[nv->adr] = nv->buf[i];
					if ((nv->adr & 0xff) == 0xff)
						nv->adr &= 0x700;	// see LC16 manual : adr is cycled inside a page
					else
						nv->adr++;
				}
			}
			nv->mode = NV_IDLE;
			nv->stable = 0;
			nv->rx = 0;
			nv->tx = 0;
		}
	} else if (nv->sdc && !sdc) {		// sdc 1->0 : back front.
		if (nv->ack) {
			nv->ack = 0;
		} else {
			if (nv->tx) {
				nv->data <<= 1;
			} else if (nv->rx && nv->stable) {
				nv->data <<= 1;
				if (nv->sda) nv->data |= 1;
				nv->bitcount--;
//				printf("wr bit %i\n",nv->sda ? 1 : 0);
				if (nv->bitcount == 0) {
//					printf("WR %.2X\n",nv->data);
					switch (nv->mode) {
						case NV_COM:
							if ((nv->data & 0xf0) == 0xa0) {
								if (nv->data & 1) {			// rd
									nv->bitcount = 0;
									nv->rx = 0;
									nv->tx = 1;
								} else {				// wr
									nv->adr &= 0x0ff;
									nv->adr |= ((nv->data & 0x0e) << 7);	// hi 3 bits of adr
									nv->bitcount = 8;
									nv->mode = NV_ADR;
								}
								nv->ack = 1;				// ack after com
							} else {
								nv->mode = NV_IDLE;			// 1010 not detected
								nv->stable = 0;
								nv->ack = 0;
								nv->rx = 0;
								nv->tx = 0;
							}
							break;
						case NV_ADR:
							nv->adr &= 0x700;		// low 8 bits of addr
							nv->adr |= (nv->data & 0xff);
							nv->bitcount = 8;
							nv->mode = NV_WRITE;
							nv->ack = 1;
							nv->bufpos = 0;
							break;
						case NV_WRITE:
							nv->buf[nv->bufpos & 0x0f] = nv->data;
							nv->bufpos++;
							nv->ack = 1;
							break;
					}
				}
			}
		}
	}
	nv->sda = sda;
	nv->sdc = sdc;
}

int nvRd(nvRam* nv) {
	int res = 0;
	if (nv->ack || (nv->mode == NV_IDLE)) {
		res = 0;
	} else if (nv->tx) {
		res = (nv->data & 0x80) ? 1 : 0;
	}
	return res;
}
