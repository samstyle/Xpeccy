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
		if (ishi) {*val = bus.hi;} else {cur->in(prt); *val = bus.low;}
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
		if (ishi) {bus.hi = val;} else {bus.low = val; cur->out(prt);}
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
	reg.state = 0x50;
	reg.err = 0x01;
}

// set REG value to bus (16bit for data, low for other)
void ATADev::in(int) {
}

void ATADev::out(int) {
}
