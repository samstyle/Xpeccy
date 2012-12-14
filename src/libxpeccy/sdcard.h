#ifndef _SDCARD_H
#define _SDCARD_H

// state
#define	SDC_IDLE	0x00	// idle
#define	SDC_FREE	0x01	// waiting for command
#define	SDC_ARG		0x02	// get arguments
#define	SDC_RESPONSE	0x03	// send response
#define	SDC_BUF_RD	0x04	// allow cpu to read buffer
#define	SDC_BUF_WR	0x05	// allow cpu write to buffer
// response mode
#define	SDC_R1		0x01
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
// commands
#define	GO_IDLE_STATE		0x40	// soft reset
#define	SEND_OP_COND		0x41	// out of idle state
#define	SEND_CSD		0x49	// ask card to send card speficic data (CSD)
#define	SEND_CID		0x4a	// ask card to send card identification (CID)
#define	STOP_TRANSMISSION	0x4c	// stop transmission on multiple block read
#define	CMD13			0x4d	// ask the card to send it's status register
#define	SET_BLOCKLEN		0x50    // sets the block length used by the memory card
#define	READ_SINGLE_BLOCK	0x51	// read single block
#define	READ_MULTIPLE_BLOCK	0x52	// read multiple block
#define	WRITE_BLOCK		0x58	// writes a single block
#define	WRITE_MULTIPLE_BLOCK	0x59	// writes multiple blocks
#define	CMD27			0x5B	// change the bits in CSD
#define	CMD28			0x5C	// sets the write protection bit
#define	CMD29			0x5D    // clears the write protection bit
#define	CMD30			0x5E	// checks the write protection bit
#define	CMD32			0x60	// Sets the address of the first sector of the erase group
#define	CMD33			0x61	// Sets the address of the last sector of the erase group
#define	CMD34			0x62	// removes a sector from the selected group
#define	CMD35			0x63	// Sets the address of the first group
#define	CMD36			0x64	// Sets the address of the last erase group
#define	CMD37			0x65	// removes a group from the selected section
#define	CMD38			0x66	// erase all selected groups
#define	CMD42			0x6A	// locks a block
#define	READ_OCR		0x7A	// reads the OCR register
#define	CMD59			0x7B	// turns CRC off

typedef struct {
	int flag;
	unsigned char state;
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
		unsigned char pos;
		unsigned char data[515];	// data packet: token(1),data(512),crc(2)
	} buf;
} SDCard;

SDCard* sdcCreate();
void sdcDestroy(SDCard*);

unsigned char sdcRead(SDCard*);
void sdcWrite(SDCard*,unsigned char);

void sdcSetImage(SDCard*,const char*);

#endif
