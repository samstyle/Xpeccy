#include <stdio.h>
#include <string.h>

#include "fdc.h"

struct DiskHW {
	const int id;
	void(*reset)(struct DiskIF*);
	int(*in)(struct DiskIF*,int,int*,int);
	int(*out)(struct DiskIF*,int,int,int);
	void(*sync)(struct DiskIF*,int);
	void(*irq)(struct DiskIF*,int);
	void(*term)(struct DiskIF*);
};

int fdcFlag = 0;

// dummy (none)

void dumReset(DiskIF* dif) {}
int dumIn(DiskIF* dif, int port, int* res,int dos) {return 0;}
int dumOut(DiskIF* dif, int port, int val,int dos) {return 0;}
void dumSync(DiskIF* dif, int ns) {}

// overall

void fdcSync(FDC* fdc, int ns) {
	if (fdc->plan == NULL) return;	// fdc does nothing
	fdc->wait -= ns;
	fdc->tns += ns;
	while ((fdc->wait < 0) && (fdc->plan != NULL)) {
		if (fdc->plan[fdc->pos] != NULL) {
			fdc->plan[fdc->pos](fdc);
		} else {
			fdc->plan = NULL;
		}
	}
}

void dhwSync(DiskIF* dif, int ns) {
	fdcSync(dif->fdc, ns);
}

void dhw_irq(int id, void* p) {
	DiskIF* dif = (DiskIF*)p;
	if (dif->fdc->dma || (dif->hw->irq && dif->inten)) {
		dif->hw->irq(dif, id);
	}
}

void fdc_set_hd(FDC* fdc, int hd) {
	fdc->hd = !!hd;
	fdc->bytedelay = hd ? 16000 : 32000;
	flp_set_hd(fdc->flop[0], hd);
	flp_set_hd(fdc->flop[1], hd);
	flp_set_hd(fdc->flop[2], hd);
	flp_set_hd(fdc->flop[3], hd);
}

// BDI (VG93)

void vgReset(FDC*);
unsigned char vgRead(FDC*, int);
void vgWrite(FDC*, int, unsigned char);
void vgSetMR(FDC*, int);

int bdiGetPort(int port) {
	int res = 0;
	if ((port & 0x9f) == 0x9f) {			// 1xxxxx11 : bdi system port
		res = BDI_SYS;
	} else {
		switch (port & 0xff) {			// 0xxxxx11 : vg93 registers
			case 0x1f: res = FDC_COM; break;	// 000xxx11
			case 0x3f: res = FDC_TRK; break;	// 001xxx11
			case 0x5f: res = FDC_SEC; break;	// 010xxx11
			case 0x7f: res = FDC_DATA; break;	// 011xxx11
		}
	}
	return res;
}

int bdiIn(DiskIF* dif, int port, int* res, int dos) {
	if (!dos) return 0;
	port = bdiGetPort(port);
//	printf("in BDI port %.2X\n",port);
	if (port == 0) {
		return 0;
	} else if (port == BDI_SYS) {
		*res = (dif->fdc->irq ? 0x80 : 0x00) | (dif->fdc->drq ? 0x40 : 0x00);
	} else {
		*res = vgRead(dif->fdc, port);
	}
	return 1;
}

int bdiOut(DiskIF* dif, int port, int val, int dos) {
	if (!dos) return 0;
	port = bdiGetPort(port);
//	printf("out BDI port %.2X,%.2X\n",port,val);
	if (port == 0) {
		return 0;
	} else if (port == BDI_SYS) {
		dif->fdc->flp = dif->fdc->flop[val & 3];	// select floppy
		vgSetMR(dif->fdc,(val & 0x04) ? 1 : 0);		// master reset
		dif->fdc->block = (val & 0x08) ? 1 : 0;
		dif->fdc->side = (val & 0x10) ? 0 : 1;		// side
		dif->fdc->mfm = (val & 0x40) ? 1 : 0;
	} else {
		vgWrite(dif->fdc, port, val);
	}
	return 1;
}

void bdiReset(DiskIF* dif) {
	vgReset(dif->fdc);
	bdiOut(dif, 0xff, 0x00, 1);
}

void bdiSync(DiskIF* dif, int ns) {
	fdcSync(dif->fdc, ns);
}

// +3DOS (uPD765)

void uReset(FDC*);
unsigned char uRead(FDC*, int);
void uWrite(FDC*, int, unsigned char);
void uTCount(FDC*);

int pdosGetPort(int p) {
	int port = -1;
	if ((p & 0xf002) == 0x2000) port = 0;		// A0 input of upd765
	if ((p & 0xf002) == 0x3000) port = 1;		// 0:status(r), 1:data(rw)
	return port;
}

int pdosIn(DiskIF* dif, int port, int* res, int dos) {
	port = pdosGetPort(port);
	if (port < 0) return 0;
//	printf("in %.4X\n",port);
	*res = uRead(dif->fdc, port);
	return 1;
}

int pdosOut(DiskIF* dif, int port, int val, int dos) {
	port = pdosGetPort(port);
	if (port < 0) return 0;
	uWrite(dif->fdc, port, val);
	return 1;
}

void pdosReset(DiskIF* dif) {
//	dif->inten = 0;
//	dif->lirq = 0;
	uReset(dif->fdc);
}

// TODO: interrupt on flp ready signal changed (on a disk change)
void pdosSync(DiskIF* dif, int ns) {
	dhwSync(dif, ns);
	if (dif->fdc->flp->dwait > 0) {
		dif->fdc->flp->dwait -= ns;
		if (dif->fdc->flp->dwait < 0) {
			dif->fdc->flp->dwait = 0;
			dhw_irq(IRQ_FDD_RDY, dif);
//			dif->fdc->sr0 &= 0x1f;
//			dif->fdc->sr0 |= 0xc0;		// 110xxxxx: ready changed
//			dif->fdc->xirq(IRQ_FDC, dif->fdc->xptr);	// rdy 0->1
			dif->fdc->flp->door = dif->fdc->flp->insert;
			printf("insert:%i\tdoor:%i\n",dif->fdc->flp->insert,dif->fdc->flp->door);
		}
	}
}

// pc (i8275 = upd765)

int dpcIn(DiskIF* dif, int port, int* rptr, int dos) {
	int res = 0xff;
	switch (port & 7) {
		case 0:
			// b4:trk 0
			// b3:side
			// b2:index
			// b1:wr protect
			// b0:step dir
			res = 0;
			if (!dif->fdc->flp->trk == 0) res |= 0x10;
			if (dif->fdc->side) res |= 0x08;
			if (!dif->fdc->flp->index) res |= 0x04;
			if (!dif->fdc->flp->protect) res |= 0x02;
			if (dif->fdc->dir) res |= 0x01;
			break;
		case 1:
			// b5:drive (0:a, 1:b)
			// b4:wr
			// b3:rd
			// b2:write enable
			// b1:drv 1 motor enable
			// b0:drv 0 motor enable
			res = 0;
			if (dif->fdc->flp->id & 1) res |= 0x20;
			if (dif->fdc->flop[1]->motor) res |= 2;
			if (dif->fdc->flop[0]->motor) res |= 1;
			break;
		case 4:
		case 5: res = uRead(dif->fdc, port & 1);
			break;
		case 7:
			// b0: HD
			// b7: disk changing
			res = 0x7f;
			if (!dif->fdc->flp->door) {
				res |= 0x80;
			}
			break;
	}
	*rptr = res;
	return 1;
}

int dpcOut(DiskIF* dif, int port, int val, int dos) {
	// printf("i8275: out %.3X %.2X\n",port,val);
	switch (port & 7) {
		case 2:
			// b4..7 = motor drive 0..3 (if 1, motor on when drive selected)
			// b3 = enable int/dma
			// b2 = 0:fdc reset
			// b0,1 = drive select
			dif->fdc->flp = dif->fdc->flop[val & 3];
			if (val & (0x10 << (val & 3)))
				dif->fdc->flp->motor = 1;
			if (!(val & 4)) uReset(dif->fdc);
			dif->inten = (val & 8) ? 1 : 0;
			break;
		case 4:
		case 5: uWrite(dif->fdc, port & 1, val);
			break;
		case 6:
			/*03F6	r/w	FIXED disk controller data register
				 bit 7-4    reserved
				 bit 3 = 0  reduce write current
					 1  head select 3 enable
				 bit 2 = 1  disk reset enable
					 0  disk reset disable
				 bit 1 = 0  disk initialization enable
					 1  disk initialization disable
				 bit 0	    reserved*/
			break;
		case 7:
			// b0,1 = transfer rate: 00:500Kbit/s, 01:reserved,10:250Kbit/s, 11:reserved
			switch(val & 3) {
				case 0: dif->fdc->bytedelay = 16000; break;	// ns/byte
				case 2: dif->fdc->bytedelay = 32000; break;
			}
			break;
	}
	return 1;
}

void dpc_irq(DiskIF* dif, int id) {
	switch(id) {
		case IRQ_FDD_RDY:
			dif->fdc->sr0 &= 0x1f;
			dif->fdc->sr0 |= 0xc0;		// 110xxxxx: ready changed
			dif->fdc->xirq(IRQ_FDC, dif->fdc->xptr);
			break;
	}
}

void dpc_term(DiskIF* dif) {
	uTCount(dif->fdc);
}

// bk

void vp1_reset(FDC*);
unsigned short vp1_rd(FDC*, int);
void vp1_wr(FDC*, int, unsigned short);

void bkdReset(DiskIF* dif) {
	vp1_reset(dif->fdc);
}

// rd doesn't using *res as result, it will return full 16-bit value
int bkdIn(DiskIF* dif, int port, int* res, int dos) {
	return vp1_rd(dif->fdc, port & 1) & 0xffff;
}

// wr will use *dos* argument as 16-bit value to write
int bkdOut(DiskIF* dif, int port, int val, int dos) {
	vp1_wr(dif->fdc, port & 1, dos & 0xffff);
	return 1;
}

// common

static DiskHW dhwTab[] = {
	{DIF_NONE,&dumReset,&dumIn,&dumOut,&dumSync,NULL,NULL},
	{DIF_BDI,&bdiReset,&bdiIn,&bdiOut,&dhwSync,NULL,NULL},
	{DIF_P3DOS,&pdosReset,&pdosIn,&pdosOut,&pdosSync,NULL,NULL},	// upd765 (+3dos)
	{DIF_PC,&pdosReset,&dpcIn,&dpcOut,&pdosSync,dpc_irq,dpc_term},		// i8275 = upd765
	{DIF_SMK512,&bkdReset,&bkdIn,&bkdOut,&dhwSync,NULL,NULL},
	{DIF_END,NULL,NULL,NULL,NULL,NULL,NULL}
};

DiskHW* findDHW(int id) {
	DiskHW* dif = NULL;
	int idx = 0;
	while (dhwTab[idx].id != DIF_END) {
		if (dhwTab[idx].id == id) {
			dif = &dhwTab[idx];
			break;
		}
		idx++;
	}
	return dif;
}

void difSetHW(DiskIF* dif, int type) {
	dif->hw = findDHW(type);
	if (!dif->hw)
		dif->hw = findDHW(DIF_NONE);
	dif->type = dif->hw->id;
	dif->fdc->upd = (dif->hw->id == DIF_P3DOS) ? 1 : 0;	// difference between upd765 & i8275
}

DiskIF* difCreate(int type, cbirq cb, void* p) {
	DiskIF* dif = (DiskIF*)malloc(sizeof(DiskIF));
	dif->fdc = (FDC*)malloc(sizeof(FDC));
	memset(dif->fdc,0x00,sizeof(FDC));
	dif->fdc->wait = -1;
	dif->fdc->plan = NULL;
	dif->fdc->flop[0] = flpCreate(0, dhw_irq, dif);
	dif->fdc->flop[1] = flpCreate(1, dhw_irq, dif);
	dif->fdc->flop[2] = flpCreate(2, dhw_irq, dif);
	dif->fdc->flop[3] = flpCreate(3, dhw_irq, dif);
	dif->fdc->flp = dif->fdc->flop[0];
	dif->fdc->xptr = p;
	dif->fdc->xirq = cb;
	dif->fdc->debug = 0;
	fdc_set_hd(dif->fdc, 0);
	difSetHW(dif, type);
	return dif;
}

void difDestroy(DiskIF* dif) {
	flpDestroy(dif->fdc->flop[0]);
	flpDestroy(dif->fdc->flop[1]);
	flpDestroy(dif->fdc->flop[2]);
	flpDestroy(dif->fdc->flop[3]);
	dif->fdc->flp = NULL;
	free(dif->fdc);
	free(dif);
}

void difReset(DiskIF* dif) {
	dif->inten = 1;
	dif->hw->reset(dif);
}

void difSync(DiskIF* dif, int ns) {
/*
	if (dif->fdc->flp->dwait > 0) {
		dif->fdc->flp->dwait -= ns;
		if (dif->fdc->flp->dwait < 0) {
			dif->fdc->flp->dwait = 0;
			dif->fdc->xirq(IRQ_FDD_RDY, dif->fdc->xptr);
			dif->fdc->flp->door = dif->fdc->flp->insert;
			printf("insert:%i\tdoor:%i\n",dif->fdc->flp->insert,dif->fdc->flp->door);
		}
	}
*/
	dif->hw->sync(dif, ns);
}

int difOut(DiskIF* dif, int port, int val, int dos) {
	return dif->hw->out(dif,port,val,dos);
}

int difIn(DiskIF* dif, int port, int* res, int dos) {
	return dif->hw->in(dif,port,res,dos);
}

void difTerminal(DiskIF* dif) {
	if (dif->hw->term)
		dif->hw->term(dif);
}
