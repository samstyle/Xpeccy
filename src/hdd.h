#ifndef _XSP_HDD
#define _XSP_HDD

#include <string>
#include <stdint.h>

#define BUF_SIZE	512

#define IDE_NONE	0
#define IDE_ATA		1
#define IDE_ATAPI	2

#define IDE_NEMO	1
#define IDE_NEMOA8	2
#define IDE_SMUC	3
#define IDE_ATM		4

// hdd port = 0.0.0.0.0.0.CS1.CS0.1.1.1.1.1.HS2.HS1.HS0
#define HDD_DATA	0x1F0		// 16 bit (!)
#define HDD_ERROR	0x1F1
#define HDD_COUNT	0x1F2
#define HDD_SECTOR	0x1F3
#define HDD_CYL_LOW	0x1F4
#define HDD_CYL_HI	0x1F5
#define HDD_HEAD	0x1F6
#define	HDD_STATE	0x1F7		// in
#define	HDD_COM		HDD_STATE	// out
#define HDD_ASTATE	0x3F6
#define HDD_ADDR	0x3F7

class ATADev {
	public:
	ATADev();
	std::string image;		// image file path
	int32_t iface;
	uint16_t trk;			// real track (head pos) ??
	bool canlba;			// true if HDD can LBA addresation
	uint32_t lba;			// internal absolute sector number
	uint32_t maxlba;		// maximum lba (calculated)
	bool wr;			// 1 on write commands
	uint8_t buf[BUF_SIZE];		// buffer
	int32_t pos;			// current buffer pos
	struct {
		uint8_t data;
		uint8_t err;
		uint8_t state;
		uint8_t count;
		uint8_t sec;
		uint16_t cyl;
		uint8_t head;
		uint8_t com;
	} reg;				// registers
	struct {
		uint16_t word;
		uint16_t cyls;	// cylinders
		uint16_t rsrvd;
		uint16_t hds;	// heads
		uint16_t bpt;	// bytes per track
		uint16_t bps;	// bytes per sector
		uint16_t spt;	// sectors per track
		std::string serial;	// serial
		uint16_t type;	// buffer type
		uint16_t vol;	// buffer volume / 512
		std::string mcver;	// microcode version
		std::string model;	// model
	} pass;			// passport
//	void reset();
	void out(int32_t);
	void in(int32_t);
};

class IDE {
	public:
	IDE();
	ATADev master;
	ATADev slave;
	ATADev *cur;
	int32_t iface;		// none, nemo, ...
	bool in(uint16_t, uint8_t*);		// return value is iorge (true on accepting)
	bool out(uint16_t, uint8_t);
	struct {uint8_t low,hi;} bus;
	void reset();
};

// extern IDE* ide;

#endif
