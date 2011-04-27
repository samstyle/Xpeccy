#include <fstream>
#include <string.h>

#include "hdd.h"
#include "spectrum.h"

extern ZXComp* zx;

// IDE controller

IDE::IDE() {
	cur = &master;
}

bool IDE::in(uint16_t port,uint8_t* val) {
	bool res = false;
	bool ishi;
	int prt = 0;
	switch (iface) {
		case IDE_NEMO:
		case IDE_NEMOA8:
			if (((port & 6) != 0) || zx->bdi->active) return false;
			prt = (((port & 0xe0) >> 5) | (((port & 0x18) ^ 0x18) << 5) | 0x00f0);
			res = true;
			ishi = (port & ((iface==IDE_NEMO) ? 0x01 : 0x80));
			break;
	}
	if (res) {
		if (ishi) {
			*val = ((bus & 0xff00) >> 8);
		} else {
			bus = cur->in(prt);
			*val = (bus & 0x00ff);
		}
//		printf("ATA in\t%.4X (%.4X) = %.2X %.2X\n",prt,port,bus.hi,bus.low);
	}
	return res;
}

bool IDE::out(uint16_t port,uint8_t val) {
	bool res = false;
	bool ishi;
	int prt;
	switch (iface) {
		case IDE_NEMO:
		case IDE_NEMOA8:
			if (((port & 6) != 0) || zx->bdi->active) return false;
			res = true;
			prt = ((port & 0xe0) >> 5) | (((port & 0x18) ^ 0x18) << 5) | 0x00f0;
			if (prt == HDD_HEAD) cur = (prt & 0x08) ? &slave : &master;	// write to head reg: select MASTER/SLAVE
			ishi = (port & ((iface==IDE_NEMO) ? 0x01 : 0x80));
			break;
	}
	if (res) {
		if (ishi) {
			bus &= 0x00ff;
			bus |= (val << 8);
		} else {
			bus &= 0xff00;
			bus |= val;
			cur->out(prt,bus);
		}
//		printf("ATA out\t%.4X (%.4X) = %.2X %.2X\n",prt,port,bus.hi,bus.low);
	}
	return res;
}

void IDE::reset() {
	master.reset();
	slave.reset();
	cur = &master;
}

// ATA(PI) devices

ATADev::ATADev() {
}

void ATADev::reset() {
	reg.state = HDF_DRDY | HDF_DSC;
	reg.err = 0x01;
	buf.mode = HDB_IDLE;
	buf.pos = 0;
}

// set REG value to bus (16bit for data, low for other)
uint16_t ATADev::in(int32_t prt) {
	uint16_t res = 0xffff;
	if (reg.state & HDF_BSY) {
		if ((prt == HDD_DATA) && (buf.mode == HDB_READ)) {
			res = (buf.data[buf.pos] << 8) | buf.data[buf.pos+1];
			buf.pos += 2;
			if (buf.pos >= HDD_BUFSIZE) {
				buf.pos = 0;
				reg.count--;
				if (reg.count == 0) {
					buf.mode = HDB_IDLE;
					reg.state &= ~(HDF_BSY | HDF_DRQ);
				} else {
					readSector();
				}
			}
		}
	} else {
		switch (prt) {
			case HDD_COUNT:
				res = reg.count;
				break;
			case HDD_SECTOR:
				res = reg.sec;
				break;
			case HDD_CYL_LOW:
				res = reg.cyl & 0xff;
				break;
			case HDD_CYL_HI:
				res = ((reg.cyl & 0xff00) >> 8);
				break;
			case HDD_HEAD:
				res = reg.head;
				break;
			case HDD_ERROR:
				res = reg.err;
				break;
			case HDD_STATE:
			case HDD_ASTATE:
				res = reg.state;
				break;
			default:
				printf("HDD in: port %.3X isn't emulated\n",prt);
				throw(0);
		}
	}
	return res;
}

void ATADev::out(int32_t prt, uint16_t val) {
	if (reg.state & HDF_BSY) {
		if ((prt == HDD_DATA) && (buf.mode == HDB_WRITE)) {
			buf.data[buf.pos++] = ((val & 0xff00) >> 8);
			buf.data[buf.pos++] = (val & 0xff);
			if (buf.pos >= HDD_BUFSIZE) {
				buf.pos = 0;
				writeSector();
				reg.count--;
				if (reg.count == 0) {
					buf.mode = HDB_IDLE;
					reg.state &= ~(HDF_BSY | HDF_DRQ);
				}
			}
		}
	} else {
		switch (prt) {
			case HDD_COUNT:
				reg.count = val & 0xff;
				break;
			case HDD_SECTOR:
				reg.sec = val & 0xff;
				break;
			case HDD_CYL_LOW:
				reg.cyl &= 0xff00;
				reg.cyl |= (val & 0xff);
				break;
			case HDD_CYL_HI:
				reg.cyl &= 0x00ff;
				reg.cyl |= ((val & 0xff) << 8);
				break;
			case HDD_HEAD:
				reg.head = val & 0xff;
				break;
			case HDD_COM:
				reg.com = val & 0xff;
				exec (reg.com);
				break;
			default:
				printf("HDD out: port %.3X isn't emulated\n",prt);
				throw(0);
		}
	}
}

void ATADev::readSector() {
}

void ATADev::writeSector() {
}

void ATADev::exec(uint8_t cm) {
	reg.state |= HDF_BSY;
	switch (cm) {
		default:
			reg.state &= ~HDF_BSY;
			reg.state |= HDF_ERR;
			reg.err |= HDF_ABRT;
			printf("HDD exec: command %.2X isn't emulated\n",cm);
			break;
	}
}
