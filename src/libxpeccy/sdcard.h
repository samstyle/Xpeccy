#ifndef _SDCARD_H
#define _SDCARD_H

#include <stdio.h>

// mode
#define	SDC_INACTIVE	0
#define	SDC_IDLE	1
#define	SDC_WAIT	2

// action
#define	SDC_FREE	0
#define	SDC_READ	1
#define	SDC_WRITE	2

// R1 flags
#define	R1_IDLE		1
#define	R1_ERASE_RESET	(1<<1)
#define	R1_ILLEGAL	(1<<2)
#define	R1_CRC_ERROR	(1<<3)
#define	R1_ERASE_SEQ	(1<<4)
#define	R1_ADDRESS_ERR	(1<<5)
#define	R1_PARAM_ERR	(1<<6)
// flag
#define	SDC_ON		1
#define	SDC_CS		(1<<1)
#define	SDC_ACMD	(1<<2)
#define	SDC_CHECK_CRC	(1<<3)
#define	SDC_CONT	(1<<4)		// multiple block op
#define	SDC_LOCK	(1<<5)		// write protect
// capacity
#define	SDC_32M		32
#define	SDC_64M		64
#define	SDC_128M	128
#define	SDC_256M	256
#define	SDC_512M	512
#define	SDC_1G		1024
#define	SDC_DEFAULT	SDC_128M

typedef struct {
	int flag;
	unsigned char mode;	// page 18 of SDCard specification 3.01
	unsigned char state;	// current action

	unsigned char argCnt;	// arguments countdown for command (4 + 1)
	unsigned char arg[6];	// input block (command, 4 arguments, crc)

	unsigned char respCnt;	// size of response in bytes
	unsigned char respPos;	// current response byte
	unsigned char resp[17];	// response (max 136 bits @ R2)

	unsigned int blkSize;
	unsigned int addr;
	int capacity;
	unsigned int maxlba;
	char* image;		// image file name
	FILE* file;		// image file
	struct {		// data buffer
		int pos;
		unsigned char data[515];	// data packet: token(1),data(512),crc(2)
	} buf;
} SDCard;

SDCard* sdcCreate();
void sdcDestroy(SDCard*);
void sdcReset(SDCard*);

unsigned char sdcRead(SDCard*);
void sdcWrite(SDCard*,unsigned char);

void sdcSetImage(SDCard*,const char*);
void sdcSetCapacity(SDCard*,int);

void sdcOpenFile(SDCard*);
void sdcCloseFile(SDCard*);

#endif
