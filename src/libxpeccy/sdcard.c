#include "sdcard.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

SDCard* sdcCreate() {
	SDCard* sdc = (SDCard*)malloc(sizeof(SDCard));
	if (!sdc) return NULL;
	sdc->state = SDC_FREE;
	sdc->addr = 0;
	sdc->argCnt = 6;
	sdc->flag = 0;
	sdc->buf.pos = 0;
	sdc->image = NULL;
	sdc->blkSize = 512;
	return sdc;
}

void sdcDestroy(SDCard* sdc) {
	if (sdc) free(sdc);
}

void sdcSetImage(SDCard* sdc, const char* name) {
	sdc->image = realloc(sdc->image, strlen(name) + 1);
	strcpy(sdc->image,name);
}

// io operations

unsigned char sdcRead(SDCard* sdc) {
#ifndef ISDEBUG
	return 0xff;
#endif
	if (!sdc->image || ((sdc->flag & 3) != SDC_ON)) return 0xff;	// no image or OFF or !CS
	unsigned char res = 0xff;
	switch (sdc->state) {
		case SDC_IDLE:
		case SDC_FREE:
			res = 0xff;
			break;
		case SDC_RESPONSE:
			res = sdc->resp[sdc->respPos];
			sdc->respPos++;
			if (sdc->respPos >= sdc->respCnt)
				sdc->state = sdc->act;
			break;
	}
	return res;
}

void sdcResponse(SDCard* sdc, int resp) {
	sdc->resp[0] = resp & 0xff;
	sdc->resp[1] = (resp & 0xff00) >> 8;
	sdc->resp[2] = (resp & 0xff0000) >> 16;
	sdc->resp[3] = ((resp & 0xff000000) >> 24) | 1;	// stop bit
	sdc->respCnt = 4;
	sdc->respPos = 0;
	sdc->state = SDC_RESPONSE;
}

unsigned int sdcGetArg(SDCard* sdc, unsigned int mask) {
	unsigned int res = sdc->arg[4] | (sdc->arg[3] << 8) | (sdc->arg[2] << 16) | (sdc->arg[1] << 24);
	return (res & mask);
}

void sdcExec(SDCard* sdc) {
	if ((sdc->arg[0] & 0x40) == 0x40) {
		printf("SD command %.2X\n",sdc->arg[0]);
		switch (sdc->arg[0]) {
			case GO_IDLE_STATE:
				sdcResponse(sdc,R1_IDLE);
				sdc->act = SDC_FREE;
				break;
			case SET_BLOCKLEN:
				sdc->blkSize = sdcGetArg(sdc,0xffffffff);
				sdcResponse(sdc,0);
				sdc->act = SDC_FREE;
				break;
			default:
				printf("undef!\n");
				sdc->state = SDC_FREE;
				break;
		}
	} else {
		sdcResponse(sdc,R1_ILLEGAL);	// Illegal command
		sdc->act = SDC_FREE;
	}
}

void sdcWrite(SDCard* sdc, unsigned char val) {
#ifndef ISDEBUG
	return;
#endif
	if (!sdc->image || ((sdc->flag & 3) != SDC_ON)) return;
	printf("SD out %.2X\n",val);
	switch (sdc->state) {
		case SDC_FREE:
			sdc->arg[6 - sdc->argCnt] = val;
			sdc->argCnt--;
			if (sdc->argCnt == 0) {
				sdcExec(sdc);
				sdc->argCnt = 6;
			}
			break;
	}
}
