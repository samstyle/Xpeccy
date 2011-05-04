#ifndef _XSP_HDD
#define _XSP_HDD

#include <string>
#include <stdint.h>

#define IDE_ENABLE	1

#define HDD_BUFSIZE	512

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

// state flags
#define	HDF_BSY		0x80
#define	HDF_DRDY	0x40
#define	HDF_WFT		0x20
#define	HDF_DSC		0x10
#define HDF_DRQ		0x08
#define	HDF_CORR	0x04
#define	HDF_IDX		0x02
#define HDF_ERR		0x01

// error flags
#define	HDF_BBK		0x80
#define HDF_UNC		0x40
#define	HDF_IDNF	0x10
#define HDF_ABRT	0x04
#define	HDF_T0NF	0x02
#define HDF_AMNF	0x01

// bufer mode
#define HDB_IDLE	0x00
#define HDB_READ	0x01
#define HDB_WRITE	0x02

// ata flags
#define ATA_IDLE	0x01
#define ATA_STANDBY	0x02
#define ATA_SLEEP	0x04
#define ATA_LBA		0x08
#define ATA_DMA		0x10

class ATADev {
	public:
		ATADev();
		std::string image;		// image file path
		int32_t iface;
//		bool canlba;			// true if HDD can LBA addresation
		uint32_t lba;			// internal absolute sector number
		uint32_t maxlba;		// maximum lba (calculated)
		int32_t flags;
		struct {
			uint8_t data[HDD_BUFSIZE];
			uint32_t pos;
			uint8_t mode;
		} buf;
		struct {
//			uint16_t data;
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
			uint16_t resrv;
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
		void reset();
		void out(int32_t,uint16_t);
		uint16_t in(int32_t);
		void exec(uint8_t);
		void abort();
		void clearBuf();
		void readSector();
		void writeSector();
		void getSectorNumber();
		void setSectorNumber();
		void gotoNextSector();
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
		uint16_t bus;
		void reset();
		void refresh();
};

#endif
