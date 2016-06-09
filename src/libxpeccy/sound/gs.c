#include <stdlib.h>
#include <string.h>

#include "gs.h"

unsigned char gsmemrd(unsigned short adr,int m1,void* ptr) {
	GSound* gs = (GSound*)ptr;
	unsigned char res = memRd(gs->mem,adr);
	switch (adr & 0xe300) {
		case 0x6000: gs->ch1 = res; break;
		case 0x6100: gs->ch2 = res; break;
		case 0x6200: gs->ch3 = res; break;
		case 0x6300: gs->ch4 = res; break;
	}
	return res;
}

void gsmemwr(unsigned short adr,unsigned char val,void* ptr) {
	GSound* gs = (GSound*)ptr;
	memWr(gs->mem,adr,val);
}

// internal IN
unsigned char gsiord(unsigned short port,void* ptr) {
	GSound* gs = (GSound*)ptr;
	unsigned char res = 0xff;
	port &= 0x0f;
	switch (port) {
		case 0: break;
		case 1: res = gs->pbb_zx; break;
		case 2: gs->pstate &= 0x7f; res = gs->pb3_zx; break;
		case 3: gs->pstate |= 0x80; break;
		case 4: res = gs->pstate; break;
		case 5: gs->pstate &= 0xfe; break;
		case 6: break;
		case 7: break;
		case 8: break;
		case 9: break;
		case 10: if (gs->rp0 & 0x01) gs->pstate &= 0x7f; else gs->pstate |= 0x80; break;
		case 11: if (gs->vol1 & 0x20) gs->pstate |= 1; else gs->pstate &= 0xfe; break;
	}
	return res;
}

void gsiowr(unsigned short port,unsigned char val,void* ptr) {
	GSound* gs = (GSound*)ptr;
	port &= 0x0f;
	switch (port) {
		case 0: gs->rp0 = val;
			val &= 0x1f;
			if (val == 0) {
				memSetBank(gs->mem,MEM_BANK2,MEM_ROM,0);	// gs->mem->pt2 = &gs->mem->rom[0][0];
				memSetBank(gs->mem,MEM_BANK3,MEM_ROM,1);	// gs->mem->pt3 = &gs->mem->rom[1][0];
			} else {
				memSetBank(gs->mem,MEM_BANK2,MEM_RAM,(val << 1) - 2);	// gs->mem->pt2 = &gs->mem->ram[(val << 1) - 2][0];
				memSetBank(gs->mem,MEM_BANK3,MEM_RAM,(val << 1) - 1);	// gs->mem->pt3 = &gs->mem->ram[(val << 1) - 1][0];
			}
			break;
		case 1: break;
		case 2: gs->pstate &= 0x7f; break;
		case 3: gs->pstate |= 0x80; gs->pb3_gs = val; break;
		case 4: break;
		case 5: gs->pstate &= 0xfe; break;
		case 6: gs->vol1 = val & 0x3f; break;
		case 7: gs->vol2 = val & 0x3f; break;
		case 8: gs->vol3 = val & 0x3f; break;
		case 9: gs->vol4 = val & 0x3f; break;
		case 10: if (gs->rp0 & 1) gs->pstate &= 0x7f; else gs->pstate |= 0x80; break;
		case 11: if (gs->vol1 & 64) gs->pstate |= 1; else gs->pstate &= 0xfe; break;
	}
}

unsigned char gsintrq(void* ptr) {
	return 0xff;
}

void gsSetRom(GSound* gs, int part, char* buf) {
	memSetPageData(gs->mem,MEM_ROM,part,buf);
}

GSound* gsCreate() {
	GSound* res = (GSound*)malloc(sizeof(GSound));
	memset(res,0x00,sizeof(GSound));
	void* ptr = (void*)res;
	res->cpu = cpuCreate(&gsmemrd,&gsmemwr,&gsiord,&gsiowr,&gsintrq,ptr);
	res->mem = memCreate();
	memSetSize(res->mem,2048);
	memSetBank(res->mem,MEM_BANK0,MEM_ROM,0);
	memSetBank(res->mem,MEM_BANK1,MEM_RAM,0);
	memSetBank(res->mem,MEM_BANK2,MEM_RAM,0);
	memSetBank(res->mem,MEM_BANK3,MEM_RAM,1);
	res->sync = 0;
	res->cnt = 0;
	res->pstate = 0x7e;
	res->stereo = GS_12_34;
	res->counter = 0;
	res->ch1 = 0;
	res->ch2 = 0;
	res->ch3 = 0;
	res->ch4 = 0;
	res->vol1 = 0;
	res->vol2 = 0;
	res->vol3 = 0;
	res->vol4 = 0;
	return res;
}

void gsDestroy(GSound* gs) {
	cpuDestroy(gs->cpu);
	memDestroy(gs->mem);
	free(gs);
}

void gsReset(GSound* gs) {
	cpuReset(gs->cpu);
}

void gsSync(GSound* gs) {
	if (!gs->enable) return;
	int res;
	gs->counter += gs->sync * GS_FRQ / 980;		// ticks to emulate
	while (gs->counter > 0) {
		res = cpuExec(gs->cpu);
		gs->counter -= res;
		gs->cnt += res;
		if (gs->cnt > 320) {	// 12MHz CLK, 37.5KHz INT -> int in each 320 ticks
			gs->cnt -= 320;
			res = cpuINT(gs->cpu);	// z80ex_int(gs->cpu);
			gs->cnt += res;
			gs->counter -= res;
		}
	}
	gs->sync = 0;
}

sndPair gsGetVolume(GSound* gs) {
	sndPair res;
	res.left = 0;
	res.right = 0;
	if (!gs->enable) return res;
	switch (gs->stereo) {
		case GS_MONO:
			res.left = ((gs->ch1 * gs->vol1 + \
				gs->ch2 * gs->vol2 + \
				gs->ch3 * gs->vol3 + \
				gs->ch4 * gs->vol4) >> 9);
			res.right = res.left;
			break;
		case GS_12_34:
			res.left = ((gs->ch1 * gs->vol1 + gs->ch2 * gs->vol2) >> 8);
			res.right = ((gs->ch3 * gs->vol3 + gs->ch4 * gs->vol4) >> 8);
			break;
	}
	return res;
}

// external in/out

int gsIn(GSound* gs, int prt, unsigned char* val) {
	if (!gs->enable) return 0;	// gs disabled
	if ((prt & 0x44) != 0) return 0;		// port don't catched
	gsSync(gs);
	if (prt & 8) {
		*val = gs->pstate;
	} else {
		*val = gs->pb3_gs;
		gs->pstate &= 0x7f;		// reset b7,state
	}
	return 1;
}

int gsOut(GSound* gs, int prt,unsigned char val) {
	if (!gs->enable) return 0;
	if ((prt & 0x44) != 0) return 0;
	gsSync(gs);
	if (prt & 8) {
		gs->pbb_zx = val;
		gs->pstate |= 1;
	} else {
		gs->pb3_zx = val;
		gs->pstate |= 0x80;	// set b7,state
	}
	return 1;
}
