#ifndef _NVRAM_H
#define _NVRAM_H

#define	NV_IDLE		0
#define	NV_WRITE	2
#define	NV_COM		3
#define	NV_ADR		4
/*
#define	NV_STABLE	1	// sda stable during sdc=1
#define NV_ACK		(1<<1)	// send ack bit to zx
#define	NV_RX		(1<<2)	// recieve bits (zx -> nvr)
#define	NV_TX		(1<<3)	// transfer bits (nvr -> zx)
*/
typedef struct {
//	int flag;
	unsigned stable:1;
	unsigned ack:1;
	unsigned rx:1;
	unsigned tx:1;

	int mode;
	int sdc;
	int sda;
	int bitcount;
	unsigned char data;		// data recieved from zx
	unsigned short adr;
	int bufpos;
	unsigned char buf[16];		// buffer for writed data (real writing process after STOP condition)
	unsigned char mem[0x7ff];	// 2K
} nvRam;

nvRam* nvCreate();
void nvDestroy(nvRam*);

void nvWr(nvRam*,int,int,int);	// sda,sdc,wp
int nvRd(nvRam*);

#endif
