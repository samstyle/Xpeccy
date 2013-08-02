#ifndef _XSP_HDD
#define _XSP_HDD

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "nvram.h"

#define	IDE_DEFAULT	-1
#define IDE_NEMO	1
#define IDE_NEMOA8	2
#define IDE_SMUC	3
#define IDE_ATM		4

// device select
#define IDE_NONE	0
#define	IDE_MASTER	1
#define	IDE_SLAVE	2
// device type (+ IDE_NONE)
#define IDE_ATA		1
#define IDE_ATAPI	2
// ata flags
#define ATA_IDLE	1
#define ATA_STANDBY	(1 << 1)
#define ATA_SLEEP	(1 << 2)
#define ATA_LBA		(1 << 3)
#define ATA_DMA		(1 << 4)

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
#define HDB_IDLE	0
#define HDB_READ	1
#define HDB_WRITE	2

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
	int type;		// none / ata / atapi
	int flags;
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

typedef struct {
	unsigned char mode;		// mode for F0..FF reading
	unsigned char adr;
	unsigned char data[256];
} CMOS;

typedef struct {
	int type;
	ATADev* master;
	ATADev* slave;
	ATADev* curDev;
	unsigned short bus;
	struct {
		unsigned char sys;
		unsigned char fdd;
		CMOS* cmos;		// pointer to ZXComp::CMOS
		nvRam* nv;		// NVRAM
	} smuc;
} IDE;

IDE* ideCreate(int);
void ideDestroy(IDE*);
int ideIn(IDE*,unsigned short,unsigned char*,int);
int ideOut(IDE*,unsigned short,unsigned char,int);
void ideReset(IDE*);
void ideOpenFiles(IDE*);
void ideCloseFiles(IDE*);

void ideSetImage(IDE*,int,const char*);
ATAPassport ideGetPassport(IDE*,int);
void ideSetPassport(IDE*,int,ATAPassport);

#ifdef __cplusplus
}
#endif

#endif
