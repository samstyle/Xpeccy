#include <fstream>
#include <string.h>

#include "hdd.h"


// overall

void copyStringToBuffer(uint8_t* dst, std::string sst, int len) {
	sst.resize(len, ' ');
	const char* src = sst.c_str();
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
	ATADev* ata = new ATADev;
	ata->type = tp;
	ata->pass.cyls = 1024;
	ata->pass.hds = 16;
	ata->pass.vol = 1;
	ata->pass.bps = 512 * ata->pass.vol;
	ata->pass.spt = 255;
	ata->pass.bpt = ata->pass.bps * ata->pass.spt;
	ata->pass.type = 1;
	ata->pass.serial = "";
	ata->pass.mcver = "";
	ata->pass.model = "";
	ata->flags = 0x00;
	return ata;
}

void ataDestroy(ATADev* ata) {
	delete(ata);
}

void ataReset(ATADev* ata) {
	ata->reg.state = HDF_DRDY | HDF_DSC;
	ata->reg.err = 0x01;
	ata->reg.count = 0x01;
	ata->reg.sec = 0x01;
	ata->reg.cyl = 0x0000;
	ata->reg.head = 0x00;
	ata->buf.mode = HDB_IDLE;
	ata->buf.pos = 0;
	ata->flags &= ~(ATA_IDLE | ATA_SLEEP | ATA_STANDBY);
}

void ataClearBuf(ATADev* dev) {
	for (int i=0; i < HDD_BUFSIZE; i++) {
		dev->buf.data[i] = 0x00;
	}
}

void ataRefresh(ATADev* dev) {
	if (dev->flags & ATA_LBA) {
		dev->pass.spt = 255;
		dev->pass.hds = 16;
	} else {
		dev->maxlba = dev->pass.cyls * dev->pass.hds * dev->pass.spt;
	}
	dev->pass.bpt = dev->pass.bps * dev->pass.spt;
}

void ataSetSector(ATADev* dev, int nr) {
	dev->lba = nr;
	if ((dev->flags & ATA_LBA) && (dev->reg.head & 0x40)) {
		dev->reg.sec = nr & 0xff;
		dev->reg.cyl = (nr >> 8) & 0xffff;
		dev->reg.head &= 0xf0;
		dev->reg.head |= ((nr >> 24) & 0x0f);
	} else {
		if (nr < dev->maxlba) {
			dev->reg.sec = int(nr / dev->pass.spt) + 1;
			dev->reg.head &= 0xf0;
			dev->reg.head |= int((nr + 1 - dev->reg.sec) / (dev->pass.hds * dev->pass.spt)) / dev->pass.spt;
			dev->reg.cyl = (nr + 1 - dev->reg.sec - dev->pass.spt * (dev->reg.head & 0x0f)) / (dev->pass.hds * dev->pass.spt);
		}
	}
}

void ataNextSector(ATADev* dev) {
	if (dev->lba < (dev->maxlba - 1)) dev->lba++;
	ataSetSector(dev,dev->lba);
}

void ataSetLBA(ATADev* dev) {
	if ((dev->flags & ATA_LBA) && (dev->reg.head & 0x40)) {
		dev->lba = dev->reg.sec | (dev->reg.cyl << 8) | ((dev->reg.head & 0x0f) << 24);
	} else {
		if ((dev->reg.sec <= dev->pass.spt) && (dev->reg.cyl < dev->pass.cyls) && ((dev->reg.head & 15) < dev->pass.hds)) {
			dev->lba = ((dev->reg.cyl * dev->pass.hds + (dev->reg.head & 0x0f)) * dev->pass.spt) + dev->reg.sec - 1;
		} else {
			dev->lba = dev->maxlba + 1;
		}
	}
}

void ataReadSector(ATADev* dev) {
	ataSetLBA(dev);
	if (dev->lba > dev->maxlba) {			// sector not found
		dev->reg.state |= HDF_ERR;
		dev->reg.err |= (HDF_ABRT | HDF_IDNF);
	} else {
		std::ifstream file(dev->image.c_str(),std::ios::binary);
		if (!file.good()) {
			std::ofstream fzk(dev->image.c_str());
			fzk.close();
			file.open(dev->image.c_str(),std::ios::binary);
			if (!file.good()) {
				printf("Can't create HDD image file");
				dev->type = IDE_NONE;
			}
		} else {
			file.seekg(0,std::ios::end);
			size_t eps = file.tellg();
			size_t nps = dev->lba * dev->pass.bps;
			if (nps < eps) {
				file.seekg(dev->lba * dev->pass.bps);
				file.read((char*)&dev->buf.data[0],dev->pass.bps);
			} else {
				ataClearBuf(dev);
			}
			file.close();
		}
	}
}

void ataWriteSector(ATADev* dev) {
	ataSetLBA(dev);
	if (dev->lba > dev->maxlba) {			// sector not found
		dev->reg.state |= HDF_ERR;
		dev->reg.err |= (HDF_ABRT | HDF_IDNF);
	} else {
		std::ofstream file(dev->image.c_str(),std::ios::binary | std::ios::in | std::ios::out);
		if (!file.good()) {
			file.open(dev->image.c_str(),std::ios::binary | std::ios::out);
			if (!file.good()) {
				printf("Can't write to HDD image file");
				dev->type = IDE_NONE;
			}
		}
		file.seekp(dev->lba * dev->pass.bps);
		file.write((char*)&dev->buf.data[0],dev->pass.bps);
		file.close();
	}
}

void ataAbort(ATADev* dev) {
	dev->reg.state |= HDF_ERR;
	dev->reg.err |= HDF_ABRT;
}

void ataExec(ATADev* dev, uint8_t cm) {
	dev->reg.state &= ~HDF_ERR;
	dev->reg.err = 0x00;
	switch (dev->type) {
	case IDE_ATA:
		switch (cm) {
			case 0x00:			// NOP
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
			case 0x91:			// initialize drive parameters; TODO: pass.spt = reg.count; pass.heads = (reg.head & 15) + 1;
				break;
			case 0x94:			// standby immediate; TODO: if reg.count!=0 in idle/standby commands, HDD power off
			case 0xe0:
				dev->flags |= ATA_STANDBY;
				break;
			case 0x95:			// idle immediate
			case 0xe1:
				dev->flags |= ATA_IDLE;
				break;
			case 0x96:			// standby
			case 0xe2:
				dev->flags |= ATA_STANDBY;
				break;
			case 0x97:			// idle
			case 0xe3:
				dev->flags |= ATA_IDLE;
				break;
			case 0x98:			// check power mode
			case 0xe5:			// if drive is in, set sector count register to 0x00, if idle - to 0xff
				dev->reg.count = (dev->flags & ATA_IDLE) ? 0xff : 0x00;
				break;
			case 0x99:			// sleep
			case 0xe6:
				dev->flags |= ATA_SLEEP;
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
				dev->buf.data[99] = ((dev->flags & ATA_DMA) ? 0x01 : 0x00) | ((dev->flags & ATA_LBA) ? 0x02 : 0x00);	// lba/dma support
				dev->buf.pos = 0;
				dev->buf.mode = HDB_READ;
				dev->reg.state |= HDF_DRQ;
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

uint16_t ataIn(ATADev* dev,int prt) {
	uint16_t res = 0xffff;
	if ((dev->type != IDE_ATA) || (dev->image == "") || (dev->flags & ATA_SLEEP)) return res;
	switch (prt) {
		case HDD_DATA:
			if ((dev->buf.mode == HDB_READ) && (dev->reg.state & HDF_DRQ)) {
				res = dev->buf.data[dev->buf.pos] | (dev->buf.data[dev->buf.pos + 1] << 8);
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

void ataOut(ATADev* dev, int prt, uint16_t val) {
	if ((dev->type != IDE_ATA) || (dev->image == "") || (dev->flags & ATA_SLEEP)) return;
	switch (prt) {
		case HDD_DATA:
			if ((dev->buf.mode == HDB_WRITE) && (dev->reg.state & HDF_DRQ)) {
				dev->buf.data[dev->buf.pos++] = (val & 0xff);
				dev->buf.data[dev->buf.pos++] = ((val & 0xff00) >> 8);
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
	IDE* ide = new IDE;
	ide->type = tp;
	ide->master = ataCreate(IDE_NONE);
	ide->slave = ataCreate(IDE_NONE);
	ide->curDev = ide->master;
	return ide;
}

void ideDestroy(IDE* ide) {
	ataDestroy(ide->master);
	ataDestroy(ide->slave);
	delete(ide);
}

int ideGet(IDE* ide, int iface, int wut) {
	ATADev* ata = NULL;
	int res = 0;
	switch (iface) {
		case IDE_MASTER:
			ata = ide->master;
			break;
		case IDE_SLAVE:
			ata = ide->slave;
			break;
	}
	if (ata == NULL) {
		switch (wut) {
			case IDE_TYPE:
				res = ide->type;
				break;
		}
	} else {
		switch (wut) {
			case IDE_TYPE:
				res = ata->type;
				break;
			case IDE_FLAG:
				res = ata->flags;
				break;
			case IDE_MAXLBA:
				res = ata->maxlba;
				break;
		}
	}
	return res;
}

void ideSet(IDE* ide, int iface, int wut, int val) {
	ATADev* ata = NULL;
	switch (iface) {
		case IDE_MASTER:
			ata = ide->master;
			break;
		case IDE_SLAVE:
			ata = ide->slave;
			break;
	}
	if (ata == NULL) {
		switch (wut) {
			case IDE_TYPE:
				ide->type = val;
				break;
		}
	} else {
		switch (wut) {
			case IDE_TYPE:
				ata->type = val;
				break;
			case IDE_FLAG:
				ata->flags = val;
				break;
			case IDE_MAXLBA:
				ata->maxlba = val;
				break;
		}
	}
}

std::string ideGetPath(IDE* ide,int iface) {
	std::string res = "";
	switch(iface) {
		case IDE_MASTER:
			res = ide->master->image;
			break;
		case IDE_SLAVE:
			res = ide->slave->image;
			break;
	}
	return res;
}

void ideSetPath(IDE* ide,int iface,std::string path) {
	switch(iface) {
		case IDE_MASTER:
			ide->master->image = path;
			break;
		case IDE_SLAVE:
			ide->slave->image = path;
			break;
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
	pass.mcver = "";
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

bool ideIn(IDE* ide,uint16_t port,uint8_t* val,bool bdiActive) {
	bool res = false;
	bool ishdd = false;
	bool ishi = false;
	int prt = 0;
	switch (ide->type) {
		case IDE_NEMO:
		case IDE_NEMOA8:
			if (((port & 6) != 0) || bdiActive) return false;
			prt = (((port & 0xe0) >> 5) | (((port & 0x18) ^ 0x18) << 5) | 0x00f0);
			ishi = (port & ((ide->type == IDE_NEMO) ? 0x01 : 0x100));
			ishdd = true;
			res = true;
			break;
		case IDE_SMUC:
			if (((port & 0x18a3) != 0x18a2) || !bdiActive) return false;
			prt = ((port & 0x0700) >> 8) | 0x1f0;		// TODO: o, rly?
			if (ide->smucSys & 0x80) {
				if (prt == HDD_HEAD) prt = HDD_ASTATE;
			}
			res = true;					// catched smuc port
			ishi = (port == 0xd8be);
			ishdd = (ishi || ((port & 0xf8ff) == 0xf8be));		// ide port (hdd itself)
			switch (port) {
				case 0x5fba:		// version
					*val = 0x28;	// 1
					break;
				case 0x5fbe:		// revision
					*val = 0x40;	// 2
					break;
				case 0xffba:		// system
					*val = (ide->curDev->reg.state & HDF_BSY) ? 0x00 : 0x80;	// TODO: b7: IRQ
					break;
			}
			break;
	}
	if (ishdd) {
		if (ishi) {
			*val = ((ide->bus & 0xff00) >> 8);
		} else {
			ide->bus = ataIn(ide->curDev,prt);
			*val = (ide->bus & 0x00ff);
		}
	}
	return res;
}

bool ideOut(IDE* ide,uint16_t port,uint8_t val,bool bdiActive) {
	bool res = false;
	bool ishi = false;
	bool ishdd = false;
	int prt;
	switch (ide->type) {
		case IDE_NEMO:
		case IDE_NEMOA8:
			if (((port & 6) != 0) || bdiActive) return false;
			res = true;
			ishdd = true;
			prt = ((port & 0xe0) >> 5) | (((port & 0x18) ^ 0x18) << 5) | 0x00f0;
			if (prt == HDD_HEAD) ide->curDev = (prt & 0x08) ? ide->slave : ide->master;	// write to head reg: select MASTER/SLAVE
			ishi = (port & ((ide->type==IDE_NEMO) ? 0x01 : 0x100));
			break;
		case IDE_SMUC:
			if (((port & 0x18a3) != 0x18a2) || !bdiActive) return false;
			prt = ((port & 0x0700) >> 8) | 0x1f0;		// TODO: o, rly?
			ishi = ((port & 0x2000) == 0x0000);
			res = true;					// catched smuc port
			ishdd = ((port & 0xf8ff) == 0xf8be);		// ide port (hdd itself)
			switch (port) {
				case 0xffba:
					ide->smucSys = val;
					break;
			}
	}
	if (ishdd) {
		if (ishi) {
			ide->bus &= 0x00ff;
			ide->bus |= (val << 8);
		} else {
			ide->bus &= 0xff00;
			ide->bus |= val;
			ataOut(ide->curDev,prt,ide->bus);
		}
	}
	return res;
}

void ideReset(IDE* ide) {
	ataReset(ide->master);
	ataReset(ide->slave);
	ide->curDev = ide->master;
}
