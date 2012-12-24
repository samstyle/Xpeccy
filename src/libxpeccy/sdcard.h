#ifndef _SDCARD_H
#define _SDCARD_H

// mode
#define	SDC_INACTIVE	0
#define	SDC_IDLE	1
#define	SDC_READ	2
#define	SDC_WRITE	3

// action
#define	SDC_FREE	0

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
#define	SDC_CONT	(1<<4)

typedef struct {
	int flag;
	unsigned char mode;	// page 18 of SDCard specification 3.01
	unsigned char state;	// current action
	unsigned char act;	// next action after response

	unsigned char argCnt;	// arguments countdown for command (4 + 1)
	unsigned char arg[6];	// input block (command, 4 arguments, crc)

	unsigned char respCnt;	// size of response in bytes
	unsigned char respPos;	// current response byte
	unsigned char resp[17];	// response (max 136 bits @ R2)

	unsigned int blkSize;
	unsigned int addr;
	char* image;		// image file
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

#endif
