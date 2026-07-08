#include <stdio.h>
#include <string.h>

#include "fdc.h"

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
/*
	if (dif->fdc->flp->dwait > 0) {
		dif->fdc->flp->dwait -= ns;
		if (dif->fdc->flp->dwait < 0) {
			dif->fdc->flp->dwait = 0;
			if (dif->fdc->flp->door != dif->fdc->flp->insert) {
				dhw_irq(IRQ_FDD_RDY, dif);
				dif->fdc->flp->door = dif->fdc->flp->insert;
//				printf("insert:%i\tdoor:%i\n",dif->fdc->flp->insert,dif->fdc->flp->door);
			}
		}
	}
*/
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
			// b7: disk changing (set on interrupt, reset on reading)
			res = 0x7f;
			if (!dif->fdc->flp->door) {
				res |= 0x80;
			} else if (dif->flpch) {
				res |= 0x80;
				dif->flpch = 0;
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
			dif->flpch = 1;
			break;
	}
}

void dpc_term(DiskIF* dif) {
	uTCount(dif->fdc);
}

// pc98xx (upD765)

// TODO: b2,port = fdc select
// b0,1 = port number
int p98in(DiskIF* dif, int port, int* rptr, int dos) {
	int res = -1;
	switch(port & 3) {
		case 0: res = uRead(dif->fdc, 0); break;	// status
		case 1: res = uRead(dif->fdc, 1); break;	// data
		case 2: res = 0x40; break;			// b6=1, b3=fdd2/3type(0:1mb,1:640kb),b2=fdd0/1type
	}
	*rptr = res;
	return 1;
}

// b0,1 = port
// b2 = select 2nd fdc
int p98out(DiskIF* dif, int port, int val, int dos) {
	FDC* fdc = (port & 4) ? dif->fdc2 : dif->fdc;
	switch(port & 3) {
		case 1: uWrite(fdc, 1, val); break;			// com/param/data
		case 2:	if (!(val & 0x80)) uReset(fdc);			// control register: b7:rst, b6:rdy, b4:1
			break;
	}
	return 1;
}

void p98sync(DiskIF* dif, int ns) {
	fdcSync(dif->fdc, ns);
	fdcSync(dif->fdc2, ns);
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
	{DIF_NONE,dumReset,dumIn,dumOut,dumSync,NULL,NULL},
	{DIF_BDI,bdiReset,bdiIn,bdiOut,dhwSync,NULL,NULL},
	{DIF_P3DOS,pdosReset,pdosIn,pdosOut,pdosSync,NULL,NULL},		// upd765 (+3dos)
	{DIF_PC,pdosReset,dpcIn,dpcOut,pdosSync,dpc_irq,dpc_term},		// i8275 = upd765
	{DIF_PC98,pdosReset,p98in,p98out,p98sync,NULL,NULL},			// upd765 (pc98)
	{DIF_SMK512,bkdReset,bkdIn,bkdOut,dhwSync,NULL,NULL},
	{DIF_END,NULL,NULL,NULL,NULL,NULL,NULL}
};

DiskHW* findDHW(int id) {
	DiskHW* itm = dhwTab;
	while ((itm->id != id) && (itm->id != DIF_END))
		itm++;
	return (itm->id == DIF_END) ? NULL : itm;
}

void difSetHW(DiskIF* dif, int type) {
	dif->hw = findDHW(type);
	if (!dif->hw)
		dif->hw = findDHW(DIF_NONE);
	dif->type = dif->hw->id;
	dif->fdc->upd = (dif->hw->id == DIF_P3DOS) ? 1 : 0;	// difference between upd765 & i8275
}

FDC* fdc_create(cbirq cb, void* p) {
	FDC* fdc = malloc(sizeof(FDC));
	memset(fdc, 0x00, sizeof(FDC));
	fdc->wait = -1;
	fdc->plan = NULL;
	fdc->xptr = p;
	fdc->xirq = cb;
	fdc->debug = 0;
	fdc_set_hd(fdc, 0);
	return fdc;
}

void fdc_destroy(FDC* fdc) {
	fdc->flp = NULL;
	free(fdc);
}

void fdc_set_flps(FDC* fdc, Floppy* fa, Floppy* fb, Floppy* fc, Floppy* fd) {
	fdc->flop[0] = fa;
	fdc->flop[1] = fb;
	fdc->flop[2] = fc;
	fdc->flop[3] = fd;
}

void dif_align_flps(DiskIF* dif, FDC* fdc, int n0, int n1, int n2, int n3) {
	fdc->flop[0] = dif->flp[n0 & 3];
	fdc->flop[1] = dif->flp[n1 & 3];
	fdc->flop[2] = dif->flp[n2 & 3];
	fdc->flop[3] = dif->flp[n3 & 3];
	fdc->flp = fdc->flop[0];
}

DiskIF* difCreate(int type, cbirq cb, void* p) {
	DiskIF* dif = (DiskIF*)malloc(sizeof(DiskIF));
	dif->fdc = fdc_create(cb, p);
	dif->fdc2 = fdc_create(cb, p);
	for (int i = 0; i < 4; i++) {
		dif->flp[i] = flpCreate(i, dhw_irq, dif);
		dif->fdc->flop[i] = dif->flp[i];
		dif->fdc2->flop[i] = dif->flp[i];
	}
	dif->fdc->flp = dif->fdc->flop[0];
	difSetHW(dif, type);
	return dif;
}

void difDestroy(DiskIF* dif) {
	flpDestroy(dif->flp[0]);
	flpDestroy(dif->flp[1]);
	flpDestroy(dif->flp[2]);
	flpDestroy(dif->flp[3]);
	fdc_destroy(dif->fdc);
	fdc_destroy(dif->fdc2);
	free(dif);
}

void difReset(DiskIF* dif) {
	dif->inten = 1;
	dif->hw->reset(dif);
}

void difSync(DiskIF* dif, int ns) {
	dif->hw->sync(dif, ns);
	flp_sync(dif->fdc->flp, ns);
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
