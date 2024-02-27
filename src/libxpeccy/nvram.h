#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum {
	NV_IDLE	= 0,
	NV_WRITE = 2,
	NV_COM,
	NV_ADR
};

typedef struct {
	unsigned stable:1;
	unsigned ack:1;
	unsigned rx:1;
	unsigned tx:1;

	int mode;
	int sdc;
	int sda;
	int bitcount;
	unsigned char data;		// data recieved from zx
	unsigned adr:11;
	unsigned bufpos:4;
	unsigned char buf[16];		// buffer for writed data (real writing process after STOP condition)
	unsigned char mem[0x7ff];	// 2K
} nvRam;

nvRam* nvCreate();
void nvDestroy(nvRam*);

void nvWr(nvRam*,int,int,int);	// sda,sdc,wp
int nvRd(nvRam*);

#ifdef __cplusplus
}
#endif
