#ifndef _XSP_HDD
#define _XSP_HDD

#include <string>
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
#define	IDE_TYPE	0
#define	IDE_FLAG	1
#define	IDE_MAXLBA	2

struct ATAPassport {
	uint16_t word;
	uint16_t cyls;	// cylinders
	uint16_t resrv;
	uint16_t hds;	// heads
	uint16_t bpt;	// bytes per track
	uint16_t bps;	// bytes per sector		512
	uint16_t spt;	// sectors per track
	std::string serial;	// serial
	uint16_t type;	// buffer type
	uint16_t vol;	// buffer volume / 512		1
	std::string mcver;	// microcode version
	std::string model;	// model
};

struct IDE;

IDE* ideCreate(int);
void ideDestroy(IDE*);
bool ideIn(IDE*,uint16_t,uint8_t*,bool);
bool ideOut(IDE*,uint16_t,uint8_t,bool);
void ideReset(IDE*);

int ideGet(IDE*,int,int);
void ideSet(IDE*,int,int,int);
std::string ideGetPath(IDE*,int);
void ideSetPath(IDE*,int,std::string);
ATAPassport ideGetPassport(IDE*,int);
void ideSetPassport(IDE*,int,ATAPassport);

#endif
