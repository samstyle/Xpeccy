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
//		printf("ATA in\t%.4X (%.4X) = %.4X\n",prt,port,bus);
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
//		printf("ATA out\t%.4X (%.4X) = %.4X\n",prt,port,bus);
	}
	return res;
}

void IDE::reset() {
	master.reset();
	slave.reset();
	cur = &master;
}

void IDE::refresh() {
	if (master.canlba) {
		master.pass.spt = 255;
		master.pass.hds = 16;
	} else {
		master.maxlba = master.pass.cyls * master.pass.hds * master.pass.spt;
	}
	if (slave.canlba) {
		slave.pass.spt = 255;
		slave.pass.hds = 16;
	} else {
		slave.maxlba = slave.pass.cyls * slave.pass.hds * slave.pass.spt;
	}
	master.pass.bpt = master.pass.bps * master.pass.spt;
	slave.pass.bpt = slave.pass.bps * slave.pass.spt;
}

// ATA(PI) devices

ATADev::ATADev() {
	pass.cyls = 1024;
	pass.hds = 16;
	pass.vol = 1;
	pass.bps = 512 * pass.vol;
	pass.spt = 255;
	pass.bpt = pass.bps * pass.spt;
	pass.type = 1;
	pass.serial = std::string(20,' ');
	pass.mcver = std::string(8,' ');
	pass.model = std::string(40,' ');
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
	if ((iface != IDE_ATA) || (image == "")) return res;
	switch (prt) {
		case HDD_DATA:
			if ((buf.mode == HDB_READ) && (reg.state & HDF_DRQ)) {
				res = buf.data[buf.pos] | (buf.data[buf.pos+1] << 8);
				buf.pos += 2;
				if (buf.pos >= HDD_BUFSIZE) {
					buf.pos = 0;
					if ((reg.com & 0xf0) == 0x20) {
						reg.count--;
						if (reg.count == 0) {
							buf.mode = HDB_IDLE;
							reg.state &= ~HDF_DRQ;
						} else {
							gotoNextSector();
							readSector();
						}
					} else {
						buf.mode = HDB_IDLE;
						reg.state &= ~HDF_DRQ;
					}
				}
			}
			break;
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
	return res;
}

void ATADev::out(int32_t prt, uint16_t val) {
	if ((iface != IDE_ATA) || (image == "")) return;
	switch (prt) {
		case HDD_DATA:
			if ((buf.mode == HDB_WRITE) && (reg.state & HDF_DRQ)) {
				buf.data[buf.pos++] = (val & 0xff);
				buf.data[buf.pos++] = ((val & 0xff00) >> 8);
				if (buf.pos >= HDD_BUFSIZE) {
					buf.pos = 0;
					if ((reg.com & 0xf0) == 0x30) {
						writeSector();
						reg.count--;
						if (reg.count == 0) {
							buf.mode = HDB_IDLE;
							reg.state &= ~HDF_DRQ;
						} else {
							gotoNextSector();
						}
					} else {
						buf.mode = HDB_IDLE;
						reg.state &= ~HDF_DRQ;
					}
				}
			}
			break;
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
		case HDD_ASTATE:
			if (val & 0x04) reset();
			break;
		default:
			printf("HDD out: port %.3X isn't emulated\n",prt);
			throw(0);
	}
}

void ATADev::getSectorNumber() {
	if (canlba && (reg.head & 0x40)) {
		lba = reg.sec | (reg.cyl << 8) | ((reg.head & 0x0f) << 24);
	} else {
		if ((reg.sec <= pass.spt) && (reg.cyl < pass.cyls) && ((reg.head & 15) < pass.hds)) {
			lba = ((reg.cyl * pass.hds + (reg.head & 0x0f)) * pass.spt) + reg.sec - 1;
		} else {
			lba = maxlba + 1;
		}
	}
}

void ATADev::setSectorNumber() {
	if (canlba && (reg.head & 0x40)) {
		reg.sec = lba & 0xff;
		reg.cyl = (lba >> 8) & 0xffff;
		reg.head &= 0xf0;
		reg.head |= ((lba >> 24) & 0x0f);
	} else {
		if (lba < maxlba) {
			reg.sec = int(lba / pass.spt) + 1;
			reg.head &= 0xf0;
			reg.head |= int((lba + 1 - reg.sec) / (pass.hds * pass.spt)) / pass.spt;
			reg.cyl = (lba + 1 - reg.sec - pass.spt * (reg.head & 0x0f)) / (pass.hds * pass.spt);
		}
	}
}

void ATADev::gotoNextSector() {
	if (lba < (maxlba - 1)) lba++;
	setSectorNumber();
}

void shithappens(std::string);

void ATADev::readSector() {
	getSectorNumber();
	if (lba > maxlba) {			// sector not found
		reg.state |= HDF_ERR;
		reg.err |= (HDF_ABRT | HDF_IDNF);
	} else {
		std::ifstream file(image.c_str(),std::ios::binary);
		if (!file.good()) {
			std::ofstream fzk(image.c_str());
			fzk.close();
			file.open(image.c_str(),std::ios::binary);
			if (!file.good()) {
				shithappens("Can't create HDD image file");
				iface = IDE_NONE;
			}
		} else {
			file.seekg(0,std::ios::end);
			size_t eps = file.tellg();
			size_t nps = lba * pass.bps;
			if (nps < eps) {
				file.seekg(lba * pass.bps);
				file.read((char*)&buf.data[0],pass.bps);
			} else {
				for (int i=0; i<pass.bps; i++) {
					buf.data[i] = 0x00;
				}
			}
			file.close();
		}
//		printf("NR: read sector %i\n",lba);
	}
}

void ATADev::writeSector() {
	getSectorNumber();
//printf("WRITE: lba = %i (%i)\n",lba,maxlba);
	if (lba > maxlba) {			// sector not found
		reg.state |= HDF_ERR;
		reg.err |= (HDF_ABRT | HDF_IDNF);
	} else {
		std::ofstream file(image.c_str(),std::ios::binary | std::ios::in | std::ios::out);
		if (!file.good()) {
			file.open(image.c_str(),std::ios::binary | std::ios::out);
			if (!file.good()) {
				shithappens("Can't write to HDD image file");
				iface = IDE_NONE;
			}
		}
		file.seekp(lba * pass.bps);
		file.write((char*)&buf.data[0],pass.bps);
		file.close();
//		printf("NR: write sector %i\n",lba);
	}
}

void ATADev::clearBuf() {
	for (int i=0; i < HDD_BUFSIZE; i++) {
		buf.data[i] = 0x00;
	}
}

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

void ATADev::exec(uint8_t cm) {
	reg.state &= ~HDF_ERR;
	switch (cm) {
		case 0x90:			// internal diagnostic
			reg.err = 0x01;
			break;
		case 0x10:			// positioning on cylinder 0
			reg.cyl = 0x0000;
			break;
		case 0xe4:			// prepare read buffer
			buf.pos = 0;
			buf.mode = HDB_READ;
			reg.state |= HDF_DRQ;
			break;
		case 0xe8:
			buf.pos = 0;		// prepare write buffer
			buf.mode = HDB_WRITE;
			reg.state |= HDF_DRQ;
			break;
		case 0x50:			// format track
			break;
		case 0x70:			// positioning
			break;
		case 0xec:			// identification
			clearBuf();
			buf.data[0] = 0x00; buf.data[1] = 0x00;							// main word
			buf.data[2] = pass.cyls & 0xff; buf.data[3] = ((pass.cyls & 0xff00) >> 8);		// cylinders
			buf.data[4] = 0x00; buf.data[5] = 0x00;							// reserved
			buf.data[6] = pass.hds & 0xff; buf.data[7] = ((pass.hds & 0xff00) >> 8);		// heads
			buf.data[8] = pass.bpt & 0xff; buf.data[9] = ((pass.bpt & 0xff00) >> 8);		// bytes per track
			buf.data[10] = pass.bps & 0xff; buf.data[11] = ((pass.bps & 0xff00) >> 8);		// bytes per sector
			buf.data[12] = pass.spt & 0xff; buf.data[13] = ((pass.spt & 0xff00) >> 8);		// sector per track
			copyStringToBuffer(&buf.data[20],pass.serial,20);				// serial (20 bytes)
			buf.data[40] = pass.type & 0xff; buf.data[41] = ((pass.type & 0xff00) >> 8);		// buffer type
			buf.data[42] = pass.vol & 0xff; buf.data[43] = ((pass.vol & 0xff00) >> 8);		// buffer size
			copyStringToBuffer(&buf.data[46],pass.mcver,8);					// microcode version (8 bytes)
			copyStringToBuffer(&buf.data[54],pass.model,10);				// model (40 bytes)
			buf.pos = 0;
			buf.mode = HDB_READ;
			reg.state |= HDF_DRQ;
			break;
		default:
			switch (cm & 0xf0) {
				case 0x20:
					readSector();
					buf.pos = 0;
					buf.mode = HDB_READ;
					reg.state |= HDF_DRQ;
					break;
				case 0x30:
					buf.pos = 0;
					buf.mode = HDB_WRITE;
					reg.state |= HDF_DRQ;
					break;
				default:
					reg.state |= HDF_ERR;
					reg.err |= HDF_ABRT;
					printf("HDD exec: command %.2X isn't emulated\n",cm);
					break;
			}
			break;
	}
}
