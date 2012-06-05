#ifndef _XSP_HDD
#define _XSP_HDD

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

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
// get/set type
//#define	IDE_TYPE	0
//#define	IDE_FLAG	1
//#define	IDE_MAXLBA	2

#define HDD_BUFSIZE	512

// hdd port = 0.0.0.0.0.0.CS1.CS0.1.1.1.1.1.HS2.HS1.HS0
#define HDD_DATA	0x1F0		// 16 bit (!)
#define HDD_ERROR	0x1F1		// in
#define HDD_FEAT	HDD_ERROR	// out
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
// bufer mode
#define HDB_IDLE	0
#define HDB_READ	1
#define HDB_WRITE	2

typedef struct {
	uint16_t word;
	uint16_t cyls;	// cylinders
	uint16_t resrv;
	uint16_t hds;	// heads
	uint16_t bpt;	// bytes per track
	uint16_t bps;	// bytes per sector		512
	uint16_t spt;	// sectors per track
	char serial[20];	// serial
	uint16_t type;	// buffer type
	uint16_t vol;	// buffer volume / 512		1
	char mcver[8];	// microcode version
	char model[40];	// model
} ATAPassport;

typedef struct {
	int type;		// none / ata / atapi
	int flags;
	int lba;
	int maxlba;
	char* image;
	struct {
		uint8_t data[HDD_BUFSIZE];
		uint32_t pos;
		uint8_t mode;
	} buf;
	struct {
		uint8_t err;
		uint8_t state;
		uint8_t count;
		uint8_t sec;
		uint16_t cyl;
		uint8_t head;
		uint8_t com;
	} reg;				// registers
	ATAPassport pass;
} ATADev;

typedef struct {
	int type;
	ATADev* master;
	ATADev* slave;
	ATADev* curDev;
	uint16_t bus;
	uint8_t smucSys;
	uint8_t smucFdd;
	uint8_t cmosAdr;
	uint8_t cmosMem[256];
} IDE;

IDE* ideCreate(int);
void ideDestroy(IDE*);
int ideIn(IDE*,uint16_t,uint8_t*,int);
int ideOut(IDE*,uint16_t,uint8_t,int);
void ideReset(IDE*);

void ideSetImage(IDE*,int,const char*);
ATAPassport ideGetPassport(IDE*,int);
void ideSetPassport(IDE*,int,ATAPassport);

#ifdef __cplusplus
}
#endif

#endif
