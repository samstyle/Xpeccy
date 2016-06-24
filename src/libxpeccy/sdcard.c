#include "sdcard.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// commands
#define	CMD00		0x00	// + soft reset
#define	CMD01		0x01	// out of idle state
#define	CMD08		0x08	// +
#define	CMD09		0x09	// ask card to send card speficic data (CSD)
#define	CMD10		0x0a	// ask card to send card identification (CID)
#define	CMD12		0x0c	// stop transmission on multiple block read
#define	CMD13		0x0d	// ask the card to send it's status register
#define	CMD16		0x10    // + sets the block length used by the memory card
#define	CMD17		0x11	// + read single block
#define	CMD18		0x12	// + read multiple block
#define	CMD24		0x18	// + writes a single block
#define	CMD25		0x19	// + writes multiple blocks
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
#define	ACMD41		0x29	// +
#define	CMD42		0x2A	// locks a block
#define	CMD55		0x37	// + ACMD prefix
#define	CMD58		0x3A	// + reads the OCR register
#define	CMD59		0x3B	// + turn CRC

SDCard* sdcCreate() {
	SDCard* sdc = (SDCard*)malloc(sizeof(SDCard));
	memset(sdc,0x00,sizeof(SDCard));
	if (!sdc) return NULL;
	sdc->argCnt = 6;
	sdc->image = NULL;
	sdc->blkSize = 512;
	sdcSetCapacity(sdc,SDC_DEFAULT);
	sdc->file = NULL;
	sdcReset(sdc);
	return sdc;
}

void sdcDestroy(SDCard* sdc) {
	sdcCloseFile(sdc);
	free(sdc);
}

void sdcReset(SDCard* sdc) {
	sdc->mode = SDC_IDLE;
	sdc->state = SDC_FREE;
}

void sdcSetImage(SDCard* sdc, const char* name) {
	sdcCloseFile(sdc);
	if (strlen(name) == 0) {
		if (sdc->image) free(sdc->image);
		sdc->image = NULL;
		sdc->file = NULL;
	} else {
		sdc->image = realloc(sdc->image,strlen(name) + 1);
		strcpy(sdc->image,name);
		sdcOpenFile(sdc);
	}
}

void sdcSetCapacity(SDCard* sdc, int cpc) {
	if (cpc < SDC_32M) cpc = SDC_32M;
	if (cpc > SDC_1G) cpc = SDC_1G;
	sdc->capacity = cpc;
	sdc->maxlba = cpc * 1024 * 2;	// sec x 2 = 1K x 1024 = 1M
}

// file operation

void sdcRdSector(SDCard* sdc) {
//	printf("SDC read sector %i\n",sdc->addr);
	if ((sdc->addr < sdc->maxlba) && sdc->file) {
		fseek(sdc->file,sdc->addr << 9,SEEK_SET);
		fread(sdc->buf.data + 1, 512, 1, sdc->file);
	} else {
		memset((void*)&sdc->buf.data[1],512,0xff);
	}
}

void sdcWrSector(SDCard* sdc) {
//	printf("SDC write sector %i\n",sdc->addr);
	if ((sdc->addr < sdc->maxlba) && sdc->file) {
		fseek(sdc->file,sdc->addr << 9,SEEK_SET);
		fwrite(sdc->buf.data + 1,512,1,sdc->file);
	}
}

// io operations

unsigned char sdcRead(SDCard* sdc) {
	if (!sdc->image || !sdc->on || sdc->cs) return 0xff;	// no image or OFF or !CS
	unsigned char res = 0xff;
	if (sdc->respCnt > 0) {				// if have response
		res = sdc->resp[sdc->respPos];
//		printf("resp %.2X\n",res);
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
//				printf("buf %X = %.2X\n",sdc->buf.pos,res);
				sdc->buf.pos++;
				if (sdc->buf.pos > 514) {		// 1token.512b.2crc = 0..514 = 515 bytes
					sdc->buf.pos = -1;
					if (sdc->cont) {
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
//	printf("SD exec %.2X\n",sdc->arg[0]);
	if (sdc->state != SDC_FREE) {
		if (sdc->arg[0] == CMD12) {
			sdcR1(sdc,0);		// ok
			sdc->mode = SDC_FREE;
		}
	}
	if ((sdc->arg[0] & 0x40) == 0x40) {
		if (sdc->acmd) {
//			printf("SD ACMD%.2i\n",sdc->arg[0] & 0x3f);
			sdc->acmd = 0;		// reset ACMD
			switch (sdc->arg[0] & 0x3f) {
				case ACMD41:
					sdcR1(sdc,0);
					break;
				default:
					printf("undef ACMD %.2X\n",sdc->arg[0] & 0x3f);
					// assert(0);
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
					sdc->cont = 0;
					sdcR1(sdc,0);
					if (sdc->buf.pos < 0) sdc->state = SDC_FREE;
					break;
				case CMD16:
					sdc->blkSize = sdcGetArg(sdc,0xffffffff);
					sdcR1(sdc,0);
					break;
				case CMD18:				// read multiple block
					sdc->cont = 1;
				case CMD17:				// read block
					sdc->addr = sdcGetArg(sdc,0xffffffff);
					if (sdc->addr < sdc->maxlba) {
						sdcR1(sdc,0);
						sdc->state = SDC_READ;
						sdc->buf.pos = -1;
					} else {
						sdcR1(sdc,R1_ADDRESS_ERR);
						sdc->cont = 0;
						sdc->state = SDC_FREE;
					}
					break;
				case CMD25:				// write multiple block
					sdc->cont = 1;
				case CMD24:				// write block
					sdc->addr = sdcGetArg(sdc,0xffffffff);
					if (sdc->addr < sdc->maxlba) {
						sdcR1(sdc,0);
						sdc->state = SDC_WRITE;
						sdc->buf.pos = 0;
					} else {
						sdcR1(sdc,R1_ADDRESS_ERR);
						sdc->cont = 0;
						sdc->state = SDC_FREE;
					}
					break;
				case CMD55:
					sdc->acmd = 1;			// next command is ACMD
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
					sdc->checkCrc = sdc->arg[4] & 1;
					sdcR1(sdc,0);
					break;
				default:
					printf("undef CMD %.2X\n",sdc->arg[0] & 0x3f);
					// assert(0);
					break;
			}
		}
	} else {
		sdcR1(sdc,R1_ILLEGAL);	// Illegal command
	}
}

void sdcWrite(SDCard* sdc, unsigned char val) {
	if (!sdc->image || !sdc->on || sdc->cs) return;
//	printf("SD out %.2X\n",val);
	if (sdc->state == SDC_WRITE) {
		if ((sdc->arg[0] = CMD25) && (sdc->buf.pos == 0) && (val == 0xfd)) {		// stop token for com25
			sdc->state = SDC_FREE;
//			printf("CMD25 BREAK (%i)\n",sdc->argCnt);
		} else {
			sdc->buf.data[sdc->buf.pos] = val;
			sdc->buf.pos++;
			if (sdc->buf.pos > 514) {	// 0..514 = 515 bytes = {1token,512data,2crc}
				if (sdc->lock) {
					sdcR1(sdc,0x1d);	// xxx01101 - data rejected due write error
					sdc->state = SDC_FREE;
				} else {
					sdcR1(sdc,0x05);	// xxx00101 - data accepted
					sdcWrSector(sdc);
					if (sdc->cont) {
						sdc->busy = 1;
						sdc->buf.pos = 0;
						sdc->addr++;
					} else {
						sdc->state = SDC_FREE;
					}
				}
			}
		}
	} else {
		sdc->arg[6 - sdc->argCnt] = val;
		sdc->argCnt--;
		if (sdc->argCnt == 0) {
			sdcExec(sdc);
			sdc->argCnt = 6;
		}
	}
}

void sdcOpenFile(SDCard* sdc) {
	if (sdc->file) fclose(sdc->file);
	if (sdc->image) sdc->file = fopen(sdc->image,"rb+");
}

void sdcCloseFile(SDCard* sdc) {
	if (sdc->file) fclose(sdc->file);
	sdc->file = NULL;
}
