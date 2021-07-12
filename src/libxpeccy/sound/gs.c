#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gs.h"

#define GS_FLUSH 1

// internal MEMRQ
int gsmemrd(int adr, int m1, void* ptr) {
	GSound* gs = (GSound*)ptr;
	unsigned char res = memRd(gs->mem, adr & 0xffff) & 0xff;
	switch (adr & 0xe300) {
		case 0x6000:
			gs->ch1 = res;
			break;
		case 0x6100:
			gs->ch2 = res;
			break;
		case 0x6200:
			gs->ch3 = res;
			break;
		case 0x6300:
			gs->ch4 = res;
			break;
	}
	return res;
}

void gsmemwr(int adr, int val, void* ptr) {
	GSound* gs = (GSound*)ptr;
	memWr(gs->mem, adr & 0xffff, val & 0xff);
}

// internal IORQ
int gsiord(int port,void* ptr) {
	GSound* gs = (GSound*)ptr;
	int res = 0xff;
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

void gsiowr(int port, int val, void* ptr) {
	GSound* gs = (GSound*)ptr;
	port &= 0x0f;
	switch (port) {
		case 0: gs->rp0 = val & 0xff;
			val &= 0x1f;
			if (val == 0) {
				memSetBank(gs->mem, 0x80, MEM_ROM, 0, MEM_16K, NULL, NULL, NULL);
				memSetBank(gs->mem, 0xc0, MEM_ROM, 1, MEM_16K, NULL, NULL, NULL);
			} else {
				val--;
				memSetBank(gs->mem, 0x80, MEM_RAM, val << 1, MEM_16K, NULL, NULL, NULL);
				memSetBank(gs->mem, 0xc0, MEM_RAM, (val << 1) + 1, MEM_16K, NULL, NULL, NULL);
			}
			break;
		case 1: break;
		case 2: gs->pstate &= 0x7f;
			break;
		case 3: gs->pstate |= 0x80;
			gs->pb3_gs = val & 0xff;
			break;
		case 4: break;
		case 5: gs->pstate &= 0xfe;
			break;
		case 6: gs->vol1 = val & 0x3f;
			break;
		case 7: gs->vol2 = val & 0x3f;
			break;
		case 8: gs->vol3 = val & 0x3f;
			break;
		case 9: gs->vol4 = val & 0x3f;
			break;
		case 10: if (gs->rp0 & 0x01)
				gs->pstate &= 0x7f;
			else
				gs->pstate |= 0x80;
			break;
		case 11: if (gs->vol1 & 0x20)
				gs->pstate |= 1;
			else
				gs->pstate &= 0xfe;
			break;
	}
}

int gsintrq(void* ptr) {
	return 0xff;
}

// EXTERNAL

GSound* gsCreate() {
	GSound* res = (GSound*)malloc(sizeof(GSound));
	memset(res,0x00,sizeof(GSound));
	res->cpu = cpuCreate(CPU_Z80, &gsmemrd, &gsmemwr, &gsiord, &gsiowr, &gsintrq, res);
	res->cpu->inten = 0;
//	res->cpu->ack = 1;
	res->mem = memCreate();
	memSetSize(res->mem, MEM_2M, MEM_32K);
	memSetBank(res->mem, 0x00, MEM_ROM, 0, MEM_16K, NULL, NULL, NULL);
	memSetBank(res->mem, 0x40, MEM_RAM, 0, MEM_16K, NULL, NULL, NULL);
	memSetBank(res->mem, 0x80, MEM_RAM, 0, MEM_16K, NULL, NULL, NULL);
	memSetBank(res->mem, 0xc0, MEM_RAM, 1, MEM_16K, NULL, NULL, NULL);
	res->pstate = 0x7e;
	res->stereo = GS_12_34;
	res->ch1 = 0x80;
	res->ch2 = 0x80;
	res->ch3 = 0x80;
	res->ch4 = 0x80;
	res->ns_per_tick = 1000 / GS_FRQ;
	return res;
}

void gsDestroy(GSound* gs) {
	cpuDestroy(gs->cpu);
	memDestroy(gs->mem);
	free(gs);
}

void gsReset(GSound* gs) {
	if (!gs->reset) return;
	gs->cpu->reset(gs->cpu);
}

#if GS_FLUSH

void gsSync(GSound* gs, int ns) {
	if (!gs->enable) return;
	gs->time += ns;
}

void gsFlush(GSound* gs) {
	if (!gs->enable) return;
	int res;
	while (gs->time > 0) {
		res = gs->cpu->exec(gs->cpu);
		gs->time -= res * gs->ns_per_tick;
		gs->cnt += res;
		if (gs->cnt > 320) {	// 12MHz CLK, 37.5KHz INT -> int in each 320 ticks
			gs->cnt -= 320;
			gs->cpu->intrq |= Z80_INT;
		}
	}
}

#else

void gsFlush(GSound* gs) {

}

void gsSync(GSound* gs, int ns) {
	if (!gs->enable) return;
	gs->time += ns;
	int res;
	while (gs->time > 0) {
		res = gs->cpu->exec(gs->cpu);
		gs->time -= res * gs->ns_per_tick;
		gs->cnt += res;
		if (gs->cnt > 320) {	// 12MHz CLK, 37.5KHz INT -> int in each 320 ticks
			gs->cnt -= 320;
			gs->cpu->intrq |= Z80_INT;
		}
	}
}

#endif

int gsCheck(GSound* gs, int adr) {
	if (!gs->enable) return 0;
	if ((adr & 0xf7) != 0xb3) return 0;	// bit3 is for register selection
	return 1;
}

int gsWrite(GSound* gs, int adr, int data) {
	if (!gsCheck(gs, adr)) return 0;
	gsFlush(gs);
	if (adr & 8) {
		gs->pbb_zx = data & 0xff;
		gs->pstate |= 1;
	} else {
		gs->pb3_zx = data & 0xff;
		gs->pstate |= 0x80;		// set b7,state
	}
	return 1;
}

int gsRead(GSound* gs, int adr, int* dptr) {
	if (!gsCheck(gs, adr)) return 0;
	gsFlush(gs);
	if (adr & 8) {
		*dptr = gs->pstate;
	} else {
		*dptr = gs->pb3_gs;
		gs->pstate &= 0x7f;
	}
	return 1;
}

// max 1ch = 256 * 64 = 16384 = 2^14
// 4ch = 2^14 * 4 = 2^16 = 65536
// 2ch = 2^15

sndPair gsVolume(GSound* gs) {
	sndPair res;
	res.left = 0;
	res.right = 0;
	if (!gs->enable) return res;
	switch (gs->stereo) {
		case GS_MONO:
			res.left = ((gs->ch1 * gs->vol1 + \
				gs->ch2 * gs->vol2 + \
				gs->ch3 * gs->vol3 + \
				gs->ch4 * gs->vol4) >> 2);
			res.right = res.left;
			break;
		case GS_12_34:
			res.left = ((gs->ch1 * gs->vol1 + gs->ch2 * gs->vol2) >> 1);
			res.right = ((gs->ch3 * gs->vol3 + gs->ch4 * gs->vol4) >> 1);
			break;
	}
	return res;
}
