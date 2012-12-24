#include "sdcard.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

// commands
#define	CMD00		0x00	// soft reset
#define	CMD01		0x01	// out of idle state
#define	CMD08		0x08
#define	CMD09		0x09	// ask card to send card speficic data (CSD)
#define	CMD10		0x0a	// ask card to send card identification (CID)
#define	CMD12		0x0c	// stop transmission on multiple block read
#define	CMD13		0x0d	// ask the card to send it's status register
#define	CMD16		0x10    // sets the block length used by the memory card
#define	CMD17		0x11	// read single block
#define	CMD18		0x12	// read multiple block
#define	CMD24		0x18	// writes a single block
#define	CMD25		0x19	// writes multiple blocks
#define	CMD27		0x1B	// change the bits in CSD
#define	CMD28		0x1C	// sets the write protection bit
#define	CMD29		0x1D    // clears the write protection bit
#define	CMD30		0x1E	// checks the write protection bit
#define	CMD32		0x20	// Sets the address of the first sector of the erase group
#define	CMD33		0x21	// Sets the address of the last sector of the erase group
#define	CMD34		0x22	// removes a sector from the selected group
#define	CMD35		0x23	// Sets the address of the first group
#define	CMD36		0x24	// Sets the address of the last erase group
#define	CMD37		0x25	// removes a group from the selected section
#define	CMD38		0x26	// erase all selected groups
#define	ACMD41		0x29
#define	CMD42		0x2A	// locks a block
#define	CMD55		0x37	// ACMD prefix
#define	CMD58		0x3A	// reads the OCR register
#define	CMD59		0x3B	// turn CRC

SDCard* sdcCreate() {
	SDCard* sdc = (SDCard*)malloc(sizeof(SDCard));
	if (!sdc) return NULL;
	sdc->addr = 0;
	sdc->argCnt = 6;
	sdc->flag = 0;
	sdc->buf.pos = 0;
	sdc->image = NULL;
	sdc->blkSize = 512;
	sdcReset(sdc);
	return sdc;
}

void sdcDestroy(SDCard* sdc) {
	if (sdc) free(sdc);
}

void sdcReset(SDCard* sdc) {
	sdc->mode = SDC_IDLE;
	sdc->state = SDC_FREE;
}

void sdcSetImage(SDCard* sdc, const char* name) {
	sdc->image = realloc(sdc->image, strlen(name) + 1);
	strcpy(sdc->image,name);
}

// file operation

void sdcRdSector(SDCard* sdc) {
//	printf("SDC read sector %i\n",sdc->addr);
	FILE* file = fopen(sdc->image,"rb");
	fseek(file,sdc->addr * 512,SEEK_SET);
	fread((void*)&sdc->buf.data[1],512,1,file);
	fclose(file);
}

// io operations

unsigned char sdcRead(SDCard* sdc) {
#ifndef ISDEBUG
	return 0xff;		// blocked until it start to work
#endif
	if (!sdc->image || ((sdc->flag & 3) != SDC_ON)) return 0xff;	// no image or OFF or !CS
	unsigned char res = 0xff;
	if (sdc->respCnt > 0) {			// if have response
//		printf("resp\n");
		res = sdc->resp[sdc->respPos];
		sdc->respPos++;
		sdc->respCnt--;
	} else {
		switch (sdc->state) {
			case SDC_READ:
				if (sdc->buf.pos < 0) {
					sdcRdSector(sdc);
					sdc->buf.data[0] = 0xfe;
					sdc->buf.data[513] = 0x00;	// TODO: data crc
					sdc->buf.data[514] = 0x00;
					sdc->buf.pos = 0;
				}
				res = sdc->buf.data[sdc->buf.pos];
//				printf("buf.%.2X\n",res);
				sdc->buf.pos++;
				if (sdc->buf.pos > 514) {		// 1token.512b.2crc = 0..514 = 515 bytes
					sdc->buf.pos = -1;
					if (sdc->flag & SDC_CONT) {
						sdc->addr++;
						sdc->buf.pos = -1;
					} else {
						sdc->state = SDC_FREE;
					}
				}

				break;
		}
	}
	return res;
}

void sdcR1(SDCard* sdc, int resp) {
	sdc->respCnt = 1;
	sdc->respPos = 0;
	sdc->resp[0] = resp & 0xff;
}

unsigned int sdcGetArg(SDCard* sdc, unsigned int mask) {
	unsigned int res = sdc->arg[4] | (sdc->arg[3] << 8) | (sdc->arg[2] << 16) | (sdc->arg[1] << 24);
	return (res & mask);
}

void sdcExec(SDCard* sdc) {
	if (sdc->state != SDC_FREE) {
		if (sdc->arg[0] == CMD12) {

		}
	}
	if ((sdc->arg[0] & 0x40) == 0x40) {
		if (sdc->flag & SDC_ACMD) {
//			printf("SD ACMD%.2i\n",sdc->arg[0] & 0x3f);
			sdc->flag &= ~SDC_ACMD;		// reset ACMD
			switch (sdc->arg[0] & 0x3f) {
				case ACMD41:
					sdcR1(sdc,0);
					break;
				default:
//					printf("undef sdcard com");
					assert(0);
					break;
			}
		} else {
//			printf("SD CMD%.2i\n",sdc->arg[0] & 0x3f);
			switch (sdc->arg[0] & 0x3f) {
				case CMD00:
					sdcR1(sdc,R1_IDLE);
					break;
				case CMD08:
					sdc->resp[0] = 0;		// R1:no error
					sdc->resp[1] = sdc->arg[1];	// return 4 bytes of argument (ORLY)
					sdc->resp[2] = sdc->arg[2];
					sdc->resp[3] = sdc->arg[3];
					sdc->resp[4] = sdc->arg[4];
					sdc->respCnt = 5;
					sdc->respPos = 0;
					break;
				case CMD12:				// stop multiple block rd/wr
					sdc->flag &= ~SDC_CONT;
					if (sdc->buf.pos < 0) sdc->state = SDC_FREE;
					sdcR1(sdc,0);
					break;
				case CMD16:
					sdc->blkSize = sdcGetArg(sdc,0xffffffff);
					sdcR1(sdc,0);
					break;
				case CMD18:
					sdcR1(sdc,0);
					sdc->addr = sdcGetArg(sdc,0xffffffff);
//					printf("CMD18 addr %i\n",sdc->addr);
					sdc->state = SDC_READ;
					sdc->flag |= SDC_CONT;
					sdc->buf.pos = -1;
					break;
				case CMD55:
					sdc->flag |= SDC_ACMD;		// next command is ACMD
					sdcR1(sdc,0);
					break;
				case CMD58:				// read OCR register
					sdc->resp[0] = 0;		// r1
					sdc->resp[1] = 0xc0;		// powerup ready.sdh(x)c
					sdc->resp[2] = 0xff;		// all voltage is supported
					sdc->resp[3] = 0x80;
					sdc->resp[4] = 0x00;
					sdc->respCnt = 5;
					sdc->respPos = 0;
					break;
				case CMD59:
					if (sdc->arg[4] & 1) {
						sdc->flag |= SDC_CHECK_CRC;
					} else {
						sdc->flag &= ~SDC_CHECK_CRC;
					}
					sdcR1(sdc,0);
					break;
				default:
//					printf("undef sdcard com");
					assert(0);
					break;
			}
		}
	} else {
		sdcR1(sdc,R1_ILLEGAL);	// Illegal command
	}
}

void sdcWrite(SDCard* sdc, unsigned char val) {
#ifndef ISDEBUG
	return;
#endif
	if (!sdc->image || ((sdc->flag & 3) != SDC_ON)) return;
//	printf("SD out %.2X\n",val);
	if (sdc->state == SDC_WRITE) {

	} else {
		sdc->arg[6 - sdc->argCnt] = val;
		sdc->argCnt--;
		if (sdc->argCnt == 0) {
			sdcExec(sdc);
			sdc->argCnt = 6;
		}
	}
}
