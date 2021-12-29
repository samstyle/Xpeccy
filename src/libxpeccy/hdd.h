#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "nvram.h"
#include "cmos.h"

// IDE interface type
enum {
	IDE_DEFAULT = -1,
	IDE_NEMO = 1,
	IDE_NEMOA8,
	IDE_SMUC,
	IDE_ATM,
	IDE_NEMO_EVO,	// with hi/low trigger
	IDE_PROFI,
	IDE_SMK		// for BK
};
// device select
enum {
	IDE_NONE = 0,
	IDE_MASTER,
	IDE_SLAVE
};
// device type (+ IDE_NONE)
enum {
	IDE_ATA = 1,
	IDE_ATAPI
};

#define HDD_BUFSIZE	512

// hdd port = 0.0.0.0.0.0.CS1.CS0.1.1.1.1.1.HS2.HS1.HS0
#define HDD_DATA	0x00		// 16 bit (!)
#define HDD_ERROR	0x01		// in
#define HDD_FEAT	HDD_ERROR	// out
#define HDD_COUNT	0x02
#define HDD_SECTOR	0x03
#define HDD_CYL_LOW	0x04
#define HDD_CYL_HI	0x05
#define HDD_HEAD	0x06
#define	HDD_STATE	0x07		// in
#define	HDD_COM		HDD_STATE	// out
#define HDD_ASTATE	0x16
#define HDD_ADDR	0x17
// state flags
#define	HDF_BSY		(1 << 7)
#define	HDF_DRDY	(1 << 6)
#define	HDF_WFT		(1 << 5)
#define	HDF_DSC		(1 << 4)
#define HDF_DRQ		(1 << 3)
#define	HDF_CORR	(1 << 2)
#define	HDF_IDX		(1 << 1)
#define HDF_ERR		1
// error flags
#define	HDF_BBK		(1 << 7)
#define HDF_UNC		(1 << 6)
#define	HDF_IDNF	(1 << 4)
#define HDF_ABRT	(1 << 2)
#define	HDF_T0NF	(1 << 1)
#define HDF_AMNF	1
// head register flag
#define HDF_DRV		(1<<4)
#define HDF_LBA		(1<<6)

// bufer mode
enum {
	HDB_IDLE = 0,
	HDB_READ,
	HDB_WRITE
};

typedef struct {
	unsigned short word;
	unsigned short cyls;	// cylinders
	unsigned short resrv;
	unsigned short hds;	// heads
	unsigned short bpt;	// bytes per track
	unsigned short bps;	// bytes per sector		512
	unsigned short spt;	// sectors per track
	char serial[20];	// serial
	unsigned short type;	// buffer type
	unsigned short vol;	// buffer volume / 512		1
	char mcver[8];		// firmware version
	char model[40];		// model
} ATAPassport;

typedef struct {
	unsigned idle:1;
	unsigned standby:1;
	unsigned sleep:1;
	unsigned hasLBA:1;
	unsigned hasDMA:1;

	int type;		// none / ata / atapi
	int lba;
	int maxlba;
	char* image;
	FILE* file;
	struct {
		unsigned char data[HDD_BUFSIZE];
		unsigned int pos;
		unsigned char mode;
	} buf;
	struct {
		unsigned char err;
		unsigned char state;
		unsigned char count;
		unsigned char sec;
		unsigned short cyl;
		unsigned char head;
		unsigned char com;
	} reg;				// registers
	ATAPassport pass;
} ATADev;

/*
typedef struct {
	int mode;		// mode for F0..FF reading
	unsigned char adr;
	unsigned char data[256];
} CMOS;
*/

typedef struct {
	int type;
	ATADev* master;
	ATADev* slave;
	ATADev* curDev;
	unsigned short bus;
	int hiTrig;
	struct {
		unsigned char sys;
		unsigned char fdd;
		CMOS* cmos;		// pointer to ZXComp::CMOS
		nvRam* nv;		// NVRAM
	} smuc;
} IDE;

IDE* ideCreate(int);
void ideDestroy(IDE*);
int ideIn(IDE*, int, int*, int);
int ideOut(IDE*, int, int, int);
void ideReset(IDE*);
void ideOpenFiles(IDE*);
void ideCloseFiles(IDE*);

void ideSetImage(IDE*,int,const char*);
ATAPassport ideGetPassport(IDE*,int);

unsigned short ataRd(ATADev*, int);
void ataWr(ATADev*, int, unsigned short);

#ifdef __cplusplus
}
#endif
