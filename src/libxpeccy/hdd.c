#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hdd.h"

// overall

void copyStringToBuffer(unsigned char* dst, const char* src, int len) {
	while (len > 1) {
		*(dst+1) = *(src);
		*(dst) = *(src+1);
		dst += 2;
		src += 2;
		len -= 2;
	}
}

// ATA device

ATADev* ataCreate(int tp) {
	ATADev* ata = (ATADev*)malloc(sizeof(ATADev));
	memset(ata,0x00,sizeof(ATADev));
	ata->type = tp;
	ata->pass.cyls = 1024;
	ata->pass.hds = 16;
	ata->pass.vol = 1;
	ata->pass.bps = 512 * ata->pass.vol;
	ata->pass.spt = 255;
	ata->pass.bpt = ata->pass.bps * ata->pass.spt;
	ata->pass.type = 1;
	memset(ata->pass.serial,' ',20);
	memcpy(ata->pass.serial,"IDDQD",strlen("IDDQD"));
	memset(ata->pass.mcver,' ',8);
	memset(ata->pass.model,' ',40);
	memcpy(ata->pass.model,"Xpeccy HDD image",strlen("Xpeccy HDD image"));
	ata->image = NULL;
	ata->file = NULL;
	ata->reg.state = 0;
	return ata;
}

void ataDestroy(ATADev* ata) {
	free(ata);
}

void ataReset(ATADev* ata) {
	ata->reg.state = HDF_DRDY | HDF_DSC;
	ata->reg.err = 0x00;
	ata->reg.count = 0x01;
	ata->reg.sec = 0x01;
	ata->reg.cyl = 0x0000;
	ata->reg.head = 0x00;
	ata->buf.mode = HDB_IDLE;
	ata->buf.pos = 0;
	ata->idle = 0;
	ata->sleep = 0;
	ata->standby = 0;
}

void ataClearBuf(ATADev* dev) {
	int i;
	for (i = 0; i < HDD_BUFSIZE; i++) {
		dev->buf.data[i] = 0x00;
	}
}

void ataRefresh(ATADev* dev) {
	if (dev->hasLBA) {
		dev->pass.spt = 255;
		dev->pass.hds = 16;
	} else {
		dev->maxlba = dev->pass.cyls * dev->pass.hds * dev->pass.spt;
	}
	dev->pass.bpt = dev->pass.bps * dev->pass.spt;
}

void ataSetSector(ATADev* dev, int nr) {
	dev->lba = nr;
	if (dev->hasLBA && (dev->reg.head & HDF_LBA)) {
		dev->reg.sec = nr & 0xff;
		dev->reg.cyl = (nr >> 8) & 0xffff;
		dev->reg.head &= 0xf0;
		dev->reg.head |= ((nr >> 24) & 0x0f);
	} else {
		if (nr < dev->maxlba) {
			dev->reg.cyl = nr / (dev->pass.hds * dev->pass.spt);
			int tmp = nr % (dev->pass.hds * dev->pass.spt);
			dev->reg.head = tmp / dev->pass.spt;
			dev->reg.sec = tmp % dev->pass.spt + 1;
		}
	}
}

void ataNextSector(ATADev* dev) {
	if (dev->lba < (dev->maxlba - 1)) {
		dev->lba++;
	}
	ataSetSector(dev,dev->lba);
}

void ataSetLBA(ATADev* dev) {
	if (dev->hasLBA && (dev->reg.head & HDF_LBA)) {
		dev->lba = dev->reg.sec | (dev->reg.cyl << 8) | ((dev->reg.head & 0x0f) << 24);		// LBA28
	} else {
		dev->lba = ((dev->reg.cyl * dev->pass.hds + (dev->reg.head & 0x0f)) * dev->pass.spt) + dev->reg.sec - 1;
	}
}

void ataReadSector(ATADev* dev) {
	long nps;
	ataSetLBA(dev);
#ifdef ISDEBUG
	printf("ataReadSector %X (hd:%.2X cyl:%.4X sec:%.2X)\n",dev->lba,dev->reg.head & 15, dev->reg.cyl, dev->reg.sec);
#endif
	if (dev->lba >= dev->maxlba) {					// sector not found
		dev->reg.state |= HDF_ERR;
		dev->reg.err |= (HDF_ABRT | HDF_IDNF);
	} else {
		if (dev->file) {
			nps = dev->lba * dev->pass.bps;
			fseek(dev->file,nps,SEEK_SET);			// if filesize < nps, there will be 0xFF in buf
			fread((char*)dev->buf.data,dev->pass.bps,1,dev->file);
		} else {
			ataClearBuf(dev);
		}
	}
}

void ataWriteSector(ATADev* dev) {
	ataSetLBA(dev);
	if (dev->lba >= dev->maxlba) {			// sector not found
		dev->reg.state |= HDF_ERR;
		dev->reg.err |= (HDF_ABRT | HDF_IDNF);
	} else {
		if (dev->file) {
			fseek(dev->file, dev->lba * dev->pass.bps, SEEK_SET);
			fwrite((char*)dev->buf.data, dev->pass.bps, 1, dev->file);
		}
	}
}

void ataAbort(ATADev* dev) {
	dev->reg.state |= HDF_ERR;
	dev->reg.err |= HDF_ABRT;
}

void ataExec(ATADev* dev, unsigned char cm) {
	dev->reg.state = HDF_DRDY | HDF_DSC;
	dev->reg.err = 0x00;
	switch (dev->type) {
	case IDE_ATA:
#ifdef ISDEBUG
	printf("ATA exec %.2X\n",cm);
#endif
		switch (cm) {
			case 0x00:			// NOP
				break;
			case 0x08:			// device reset
				break;
			case 0x20:			// read sectors (w/retry)
			case 0x21:			// read sectors (w/o retry)
				ataReadSector(dev);
				dev->buf.pos = 0;
				dev->buf.mode = HDB_READ;
				dev->reg.state |= HDF_DRQ;
				break;
			case 0x22:			// read long (w/retry) TODO: read sector & ECC
			case 0x23:			// read long (w/o retry)
				ataAbort(dev);
				break;
			case 0x30:			// write sectors (w/retry)
			case 0x31:			// write sectors (w/o retry)
			case 0x3c:			// write verify
				dev->buf.pos = 0;
				dev->buf.mode = HDB_WRITE;
				dev->reg.state |= HDF_DRQ;
				break;
			case 0x32:			// write long (w/retry)
			case 0x33:			// write long (w/o retry)
				ataAbort(dev);
				break;
			case 0x40:			// verify sectors (w/retry)	TODO: is it just read sectors until error or count==0 w/o send buffer to host?
			case 0x41:			// verify sectors (w/o retry)
				do {
					ataReadSector(dev);
					if (dev->reg.state & HDF_BSY) break;
					dev->reg.count--;
				} while (dev->reg.count != 0);
				break;
			case 0x50:			// format track; TODO: cyl = track; count = spt; drq=1; wait for buffer write, ignore(?) buffer; format track;
				break;
			case 0x90:			// execute drive diagnostic; FIXME: both drives must do this
				dev->reg.err = 0x01;
				break;
			case 0x91:			// initialize drive parameters; pass.spt = reg.count; pass.heads = (reg.head & 15) + 1;
				dev->pass.spt = dev->reg.count;
				dev->pass.hds = (dev->reg.head & 0x0f) + 1;
				break;
			case 0x94:			// standby immediate; TODO: if reg.count!=0 in idle/standby commands, HDD power off
			case 0xe0:
				dev->standby = 1;
				break;
			case 0x95:			// idle immediate
			case 0xe1:
				dev->idle = 1;
				break;
			case 0x96:			// standby
			case 0xe2:
				dev->standby = 1;
				break;
			case 0x97:			// idle
			case 0xe3:
				dev->idle = 1;
				break;
			case 0x98:			// check power mode
			case 0xe5:			// if drive is in, set sector count register to 0x00, if idle - to 0xff
				dev->reg.count = dev->idle ? 0xff : 0x00;
				break;
			case 0x99:			// sleep
			case 0xe6:
				dev->sleep = 1;
				break;
			case 0x9a:			// vendor unique
			case 0xc0:
			case 0xc1:
			case 0xc2:
			case 0xc3:
				ataAbort(dev);
				break;
			case 0xc4:			// read multiple; NOTE: doesn't support for 1-sector buffer
				ataAbort(dev);
				break;
			case 0xc5:			// write multiple; NOTE: doesn't support for 1-sector buffer
				ataAbort(dev);
				break;
			case 0xc6:			// set multiple mode; NOTE: only 1-sector reading supported
				ataAbort(dev);
				break;
			case 0xc8:			// read DMA (w/retry)
			case 0xc9:			// read DMA (w/o retry)
				ataAbort(dev);
				break;
			case 0xca:			// write DMA (w/retry)
			case 0xcb:			// write DMA (w/o retry)
				ataAbort(dev);
				break;
			case 0xdb:			// acknowledge media chge
				dev->reg.state |= HDF_ERR;	// NOTE: HDD isn't removable, return abort error
				dev->reg.err |= HDF_ABRT;
				break;
			case 0xdc:			// boot - post-boot; TODO: do nothing?
				break;
			case 0xdd:			// boot - pre-boot; TODO: do nothing?
				break;
			case 0xde:			// door lock
				break;
			case 0xdf:			// door unlock
				break;
			case 0xe4:			// read buffer
				dev->buf.pos = 0;
				dev->buf.mode = HDB_READ;
				dev->reg.state |= HDF_DRQ;
				break;
			case 0xe8:
				dev->buf.pos = 0;		// write buffer
				dev->buf.mode = HDB_WRITE;
				dev->reg.state |= HDF_DRQ;
				break;
			case 0xe9:			// write same
				ataAbort(dev);
				break;
			case 0xec:			// identify drive
				ataClearBuf(dev);
				dev->buf.data[0] = 0x04;
				dev->buf.data[1] = 0x00;							// main word
				dev->buf.data[2] = dev->pass.cyls & 0xff;
				dev->buf.data[3] = ((dev->pass.cyls & 0xff00) >> 8);		// cylinders
				dev->buf.data[6] = dev->pass.hds & 0xff;
				dev->buf.data[7] = ((dev->pass.hds & 0xff00) >> 8);		// heads
				dev->buf.data[8] = dev->pass.bpt & 0xff;
				dev->buf.data[9] = ((dev->pass.bpt & 0xff00) >> 8);		// bytes per track
				dev->buf.data[10] = dev->pass.bps & 0xff;
				dev->buf.data[11] = ((dev->pass.bps & 0xff00) >> 8);		// bytes per sector
				dev->buf.data[12] = dev->pass.spt & 0xff;
				dev->buf.data[13] = ((dev->pass.spt & 0xff00) >> 8);		// sector per track
				copyStringToBuffer(&dev->buf.data[20],dev->pass.serial,20);	// serial (20 bytes)
				dev->buf.data[40] = dev->pass.type & 0xff;
				dev->buf.data[41] = ((dev->pass.type & 0xff00) >> 8);		// buffer type
				dev->buf.data[42] = dev->pass.vol & 0xff;
				dev->buf.data[43] = ((dev->pass.vol & 0xff00) >> 8);		// buffer size
				copyStringToBuffer(&dev->buf.data[46],dev->pass.mcver,8);	// microcode version (8 bytes)
				copyStringToBuffer(&dev->buf.data[54],dev->pass.model,10);	// model (40 bytes)
				dev->buf.data[99] = (dev->hasDMA ? 0x01 : 0x00) | (dev->hasLBA ? 0x02 : 0x00);	// lba/dma support
				dev->buf.pos = 0;
				dev->buf.mode = HDB_READ;
				dev->reg.state |= HDF_DRQ;
				//printf("request hdd info\n");
				break;
			case 0xef:			// set features
				break;
			default:
				switch (cm & 0xf0) {
					case 0x10:			// 0x1x: recalibrate
						dev->reg.cyl = 0x0000;
						break;
					case 0x70:			// seek; TODO: if cylinder/head is out of range - must be an error?
						break;
					case 0x80:			// vendor unique
					case 0xf0:
						ataAbort(dev);
						break;
					default:
						ataAbort(dev);
						printf("HDD exec: command %.2X isn't emulated\n",cm);
						break;
				}
				break;
		}
		break;
	}
}

unsigned short ataRd(ATADev* dev,int prt) {
	unsigned short res = 0xffff;
	if ((dev->type != IDE_ATA) || (dev->image == NULL) || dev->sleep) return res;
	switch (prt) {
		case HDD_DATA:
			if ((dev->buf.mode == HDB_READ) && (dev->reg.state & HDF_DRQ)) {
				res = dev->buf.data[dev->buf.pos] | (dev->buf.data[dev->buf.pos + 1] << 8);	// low-hi
				dev->buf.pos += 2;
				if (dev->buf.pos >= HDD_BUFSIZE) {
					dev->buf.pos = 0;
					if ((dev->reg.com & 0xf0) == 0x20) {
						dev->reg.count--;
						if (dev->reg.count == 0) {
							dev->buf.mode = HDB_IDLE;
							dev->reg.state &= ~HDF_DRQ;
						} else {
							ataNextSector(dev);
							ataReadSector(dev);
						}
					} else {
						dev->buf.mode = HDB_IDLE;
						dev->reg.state &= ~HDF_DRQ;
					}
				}
			}
			break;
		case HDD_COUNT:
			res = dev->reg.count;
			break;
		case HDD_SECTOR:
			res = dev->reg.sec;
			break;
		case HDD_CYL_LOW:
			res = dev->reg.cyl & 0xff;
			break;
		case HDD_CYL_HI:
			res = ((dev->reg.cyl & 0xff00) >> 8);
			break;
		case HDD_HEAD:
			res = dev->reg.head;
			break;
		case HDD_ERROR:
			res = dev->reg.err;
			break;
		case HDD_STATE:
		case HDD_ASTATE:
			res = dev->reg.state;
			break;
		default:
			printf("HDD in: port %.3X isn't emulated\n",prt);
//			throw(0);
	}
	return res;
}

void ataWr(ATADev* dev, int prt, unsigned short val) {
	if ((dev->type != IDE_ATA) || (dev->image == NULL) || dev->sleep) return;
	switch (prt) {
		case HDD_DATA:
			if ((dev->buf.mode == HDB_WRITE) && (dev->reg.state & HDF_DRQ)) {
				dev->buf.data[dev->buf.pos++] = (val & 0xff);			// low
				dev->buf.data[dev->buf.pos++] = ((val & 0xff00) >> 8);		// hi
				if (dev->buf.pos >= HDD_BUFSIZE) {
					dev->buf.pos = 0;
					if ((dev->reg.com & 0xf0) == 0x30) {
						ataWriteSector(dev);
						dev->reg.count--;
						if (dev->reg.count == 0) {
							dev->buf.mode = HDB_IDLE;
							dev->reg.state &= ~HDF_DRQ;
						} else {
							ataNextSector(dev);
						}
					} else {
						dev->buf.mode = HDB_IDLE;
						dev->reg.state &= ~HDF_DRQ;
					}
				}
			}
			break;
		case HDD_COUNT:
			dev->reg.count = val & 0xff;
			break;
		case HDD_SECTOR:
			dev->reg.sec = val & 0xff;
			break;
		case HDD_CYL_LOW:
			dev->reg.cyl &= 0xff00;
			dev->reg.cyl |= (val & 0xff);
			break;
		case HDD_CYL_HI:
			dev->reg.cyl &= 0x00ff;
			dev->reg.cyl |= ((val & 0xff) << 8);
			break;
		case HDD_HEAD:
			dev->reg.head = val & 0xff;
			break;
		case HDD_COM:
			dev->reg.com = val & 0xff;
			ataExec(dev,dev->reg.com);
			break;
		case HDD_ASTATE:
			if (val & 0x04) ataReset(dev);
			break;
		case HDD_FEAT:
			break;
		default:
			printf("HDD out: port %.3X isn't emulated\n",prt);
//			throw(0);
	}
}

// IDE interface

IDE* ideCreate(int tp) {
	IDE* ide = (IDE*)malloc(sizeof(IDE));
	ide->type = tp;
	ide->master = ataCreate(IDE_NONE);
	ide->slave = ataCreate(IDE_NONE);
	ide->curDev = ide->master;
	ide->smuc.fdd = 0xc0;
	ide->smuc.sys = 0x00;
	ide->smuc.nv = nvCreate();
	return ide;
}

void ideDestroy(IDE* ide) {
	ideCloseFiles(ide);
	ataDestroy(ide->master);
	ataDestroy(ide->slave);
	free(ide);
}

void ideSetImage(IDE *ide, int wut, const char *name) {
	ATADev* dev = NULL;
	if (wut == IDE_MASTER)
		dev = ide->master;
	else if (wut == IDE_SLAVE)
		dev = ide->slave;
	if (dev == NULL) return;
	if (dev->file) fclose(dev->file);
	if (strlen(name) == 0) {
		free(dev->image);
		dev->image = NULL;
		dev->file = NULL;
	} else {
		dev->image = realloc(dev->image,strlen(name) + 1);
		strcpy(dev->image,name);
		dev->file = fopen(dev->image,"rb+");
	}
}

ATAPassport ideGetPassport(IDE* ide, int iface) {
	ATAPassport res;
	switch(iface) {
		case IDE_MASTER:
			res = ide->master->pass;
			break;
		case IDE_SLAVE:
			res = ide->slave->pass;
			break;
	}
	return res;
}

void ideSetPassport(IDE* ide, int iface, ATAPassport pass) {
	pass.vol = 1;
	pass.bps = 512 * pass.vol;
	pass.bpt = pass.bps * pass.spt;
	pass.type = 1;
	memset(pass.mcver,' ',8);
//	pass.mcver = "";
	switch(iface) {
		case IDE_MASTER:
			ide->master->pass = pass;
			ataRefresh(ide->master);
			break;
		case IDE_SLAVE:
			ide->slave->pass = pass;
			ataRefresh(ide->slave);
			break;
	}
}

// IDE controller

// SMUC: dos, a0=0,a1=a5=a7=a11=a12=1	xxx1 1xxx 1x1x xx10

typedef struct {
	unsigned iorq:1;
	unsigned high:1;
	unsigned hdd:1;
	int port;
} ataAddr;

ataAddr ideDecoder(IDE* ide, int port, int dosen, int wr) {
	ataAddr res;
	res.port = 0xff;
	res.iorq = 0;
	res.hdd = 0;
	res.high = 0;
	switch (ide->type) {
		case IDE_ATM:
			res.iorq = (((port & 0x001f) == 0x000f) && dosen) ? 1 : 0;
			res.hdd = 1;
			res.high = ((port & 0x1ff) == 0x10f) ? 1 : 0;
			res.port = (port & 0xe0) >> 5;
			break;
		case IDE_NEMO_EVO:
			res.iorq = (port & 6) ? 0 : 1;
			res.hdd = 1;
			res.high = ((port & 0xe1) == 0x01) ? 1 : 0;
			res.port = (port & 0xe0) >> 5;
			break;
		case IDE_NEMO:
		case IDE_NEMOA8:
			res.iorq = (dosen || (port & 6)) ? 0 : 1;
			res.hdd = 1;
			if (ide->type == IDE_NEMO) res.high = ((port & 0xe1) == 0x01) ? 1 : 0;
			if (ide->type == IDE_NEMOA8) res.high = ((port & 0x1e0) == 0x100) ? 1 : 0;
			res.port = (port & 0xe0) >> 5;		//  | (((port & 0x18) ^ 0x18) << 5) | 0x00f0);
			break;
		case IDE_SMUC:
			res.iorq = (((port & 0x18a3) == 0x18a2) && dosen) ? 1 : 0;
			res.hdd = ((port & 0xf8ff) == 0xf8be) ? 1 : 0;
			if (port == 0xd8be) {
				res.hdd = 1;
				res.high = 1;
			}
			res.port = (port & 0x700) >> 8;
			break;
		case IDE_PROFI:
			if (wr) port ^= 0x20;	// wr: eb -> cb; wr 0eb<->0cb; now rd/wr 0EB is data high
			res.iorq = (((port & 0x00ff) == 0x00cb) || ((port & 0x7ff) == 0xeb)) ? 1 : 0;
			if (port == 0x06ab) {
				res.hdd = 0;
				res.iorq = 1;
			} else {
				res.hdd = 1;
			}
			res.port = (port & 0x700) >> 8;
			res.high = ((port & 0x7ff) == 0xeb) ? 1 : 0;
			break;
		case IDE_SMK:
			res.iorq = ((port & 0xfff0) == 0xffe0) ? 1 : 0;
			res.hdd = 1;
			res.port = ((port >> 1) & 7) ^ 7;
			res.high = 0;
			if (port & 1) res.port |= 0x10;	// 0x16, 0x17
			if (res.port == 0x10) {
				res.high = 1;
				res.port = HDD_DATA;
			}
			break;
	}
	return res;
}

int ideIn(IDE* ide, int port, int* val, int dosen) {
	ataAddr adr = ideDecoder(ide,port,dosen,0);
	if (!adr.iorq) return 0;
	if (adr.hdd) {
		if (ide->type == IDE_NEMO_EVO) {
			if (adr.port == 0) {
				if (adr.high) {
					ide->hiTrig = 0;				// 11 : high, next 10 is low
				} else {
					if (ide->hiTrig) adr.high = 1;			// 10 : high byte
					ide->hiTrig ^= 1;				// switch trigger
				}
			} else {
				ide->hiTrig = 0;		// non-data ports : next 10 is low
			}
		}
		if (adr.high) {
			*val = ((ide->bus & 0xff00) >> 8);
		} else {
			ide->bus = ataRd(ide->curDev,adr.port);
			*val = (ide->bus & 0x00ff);
		}
	} else {
		if (ide->type == IDE_SMUC) {
			switch (port) {
				case 0x5fba:		// version
					*val = 0x28;	// 1
					break;
				case 0x5fbe:		// revision
					*val = 0x40;	// 2
					break;
				case 0xffba:		// system
					*val = (nvRd(ide->smuc.nv) ? 0x40 : 0x00);	// TODO: b7: INTRQ from HDD/CF, b6:SDA?
					break;
				case 0x7fba:		// virtual fdd
					*val = ide->smuc.fdd | 0x3f;
					break;
				case 0x7ebe:		// pic (not used)
				case 0x7fbe:
					*val = 0xff;
					break;
				case 0xdfba:		// cmos
					*val = (ide->smuc.sys & 0x80) ? 0xff : ide->smuc.cmos->data[ide->smuc.cmos->adr];
					break;
			}
		}
	}
	return 1;
}

int ideOut(IDE* ide, int port, int val,int dosen) {
	ataAddr adr = ideDecoder(ide,port,dosen,1);
	if (!adr.iorq) return 0;
	if (adr.hdd) {
		if (adr.port == HDD_HEAD)
			ide->curDev = (val & HDF_DRV) ? ide->slave : ide->master;	// write to head reg: select MASTER/SLAVE
		if (ide->type == IDE_NEMO_EVO) {
			if (adr.port == HDD_DATA) {
				if (adr.high) {
					ide->bus &= 0x00ff;
					ide->bus |= (val << 8);
					ide->hiTrig = 2;		// 11 : high, next 10 is low
				} else {
					if (ide->hiTrig == 0) {		// 10 : low
						ide->bus &= 0xff00;
						ide->bus |= val;
						ide->hiTrig = 1;
					} else if (ide->hiTrig == 1) {	// 10 : high + wr
						ide->bus &= 0x00ff;
						ide->bus |= (val << 8);
						ataWr(ide->curDev,0,ide->bus);
						ide->hiTrig = 0;
					} else if (ide->hiTrig == 2) {	// 10 : low + wr (after 11)
						ide->bus &= 0xff00;
						ide->bus |= val;
						ataWr(ide->curDev,0,ide->bus);
						ide->hiTrig = 0;
					}
				}
			} else {
				ide->hiTrig = 0;		// non-data ports : next 10 is low
				ataWr(ide->curDev,adr.port,val);
			}
		} else if (ide->type == IDE_SMK) {
			if (adr.port == HDD_DATA) {
				if (adr.high) {
					ide->bus &= 0x00ff;
					ide->bus |= ((val << 8) & 0xff);
					ataWr(ide->curDev,adr.port,ide->bus);
				} else {
					ide->bus &= 0xff00;
					ide->bus |= (val & 0xff);
				}
			} else {
				ide->bus &= 0xff00;
				ide->bus |= (val & 0xff);
				ataWr(ide->curDev,adr.port,ide->bus);
			}
		} else {
			if (adr.high) {
				ide->bus &= 0x00ff;
				ide->bus |= (val << 8);
			} else {
				ide->bus &= 0xff00;
				ide->bus |= val;
				ataWr(ide->curDev,adr.port,ide->bus);
			}
		}
	} else {
		if (ide->type == IDE_SMUC) {
			switch (port) {
				case 0xffba:			// system
					ide->smuc.sys = val;
					nvWr(ide->smuc.nv, val & 0x10, val & 0x40, val & 0x20);		// nv,sda,scl,wp
					break;
				case 0x7fba:			// virtual fdd
					ide->smuc.fdd = val & 0xc0;
					break;
				case 0xdfba:			// cmos
					if (ide->smuc.sys & 0x80) {
						ide->smuc.cmos->data[ide->smuc.cmos->adr] = val;
					} else {
						ide->smuc.cmos->adr = val;
					}
					break;
			}
		}
	}
	return 1;
}

void ideReset(IDE* ide) {
	ataReset(ide->master);
	ataReset(ide->slave);
	ide->curDev = ide->master;
}

void ideOpenFiles(IDE* ide) {
	if (ide->master->image) ide->master->file = fopen(ide->master->image,"rb+");		// NULL when file doesn't exist
	if (ide->slave->image) ide->slave->file = fopen(ide->slave->image,"rb+");
}

void ideCloseFiles(IDE* ide) {

	if (ide->master->file) fclose(ide->master->file);
	ide->master->file = NULL;
	if (ide->slave->file) fclose(ide->slave->file);
	ide->slave->file = NULL;
}
