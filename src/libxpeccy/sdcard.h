#ifndef X_SDCARD_H
#define X_SDCARD_H

#include <stdio.h>

/*
// mode
enum {
	SDC_INACTIVE = 0,
	SDC_IDLE,
};
*/
// state
enum {
	SDC_FREE = 0,		// ready to accept a command
	SDC_WAIT,		// waiting for command arguments
	SDC_READ,		// send data to CPU
	SDC_WRITE		// accept data from CPU
};

// R1 flags
#define	R1_IDLE		1
#define	R1_ERASE_RESET	(1<<1)
#define	R1_ILLEGAL	(1<<2)
#define	R1_CRC_ERROR	(1<<3)
#define	R1_ERASE_SEQ	(1<<4)
#define	R1_ADDRESS_ERR	(1<<5)
#define	R1_PARAM_ERR	(1<<6)
// capacity
#define	SDC_32M		32
#define	SDC_64M		64
#define	SDC_128M	128
#define	SDC_256M	256
#define	SDC_512M	512
#define	SDC_1G		1024
#define	SDC_2G		2048
#define	SDC_4G		4096
#define	SDC_8G		8192
#define	SDC_DEFAULT	SDC_128M

typedef struct {
	unsigned on:1;
	unsigned cs:1;
	unsigned acmd:1;
	unsigned checkCrc:1;
	unsigned cont:1;
	unsigned lock:1;
	unsigned busy:1;

//	unsigned char mode;	// page 18 of SDCard specification 3.01
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
