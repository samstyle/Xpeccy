#include "gs.h"

extern GS* gs;

GS::GS() {
	sys = new ZXBase;
	sys->cpu = new Z80(GS_FRQ);
	sys->mem = new Memory(MEM_GS);
	sys->io = new IOSys(&gs_in,&gs_out);
	sys->vid = NULL;
	sys->mem->pt0 = &sys->mem->rom[0][0];
	sys->mem->pt1 = &sys->mem->ram[0][0];
	sys->mem->pt2 = &sys->mem->ram[0][0];
	sys->mem->pt3 = &sys->mem->ram[1][0];
	t = cnt = 0;
	pstate = 0x7e;
	flags = GS_ENABLE;
	stereo = GS_12_34;
//	rset = false;
}

void GS::reset() {
	sys->cpu->reset();
}

SndData GS::getvol() {
	SndData res; res.l = res.r = 0;
	if (~flags & GS_ENABLE) return res;
	switch (stereo) {
		case GS_MONO:
			res.l = res.r = ((ch1 * vol1 + ch2 * vol2 + ch3 * vol3 + ch4 * vol4) >> 9);
			break;
		case GS_12_34:
			res.l = ((ch1 * vol1 + ch2 * vol2) >> 8);
			res.r = ((ch3 * vol3 + ch4 * vol4) >> 8);
			break;
	}
	return res;
}

void GS::sync(uint32_t tk) {
	int ln = (tk - t) * GS_FRQ / 7.0;		// scale to GS ticks;
	t = tk;
	if (~flags & GS_ENABLE) return;
	while (ln > 0) {
		tk = sys->exec();
		ln -= tk;
		cnt += tk;
		if (cnt > 320) {	// 12MHz CLK, 37.5KHz INT -> int in each 320 ticks
			cnt -= 320;
			tk = sys->interrupt();
			cnt += tk;
			ln -= tk;
		}
	}
}

// external in/out

bool GS::in(int prt,uint8_t* val) {
	if (~flags & GS_ENABLE) return false;	// gs disabled
	if ((prt & 0x44) != 0) return false;	// port don't catched
	if (prt & 8) {
		*val = pstate;
	} else {
		*val = pb3_gs;
		gs->pstate &= 0x7f;		// reset b7,state
	}
	return true;
}

bool GS::out(int prt,uint8_t val) {
	if (~flags & GS_ENABLE) return false;
	if ((prt & 0x44) != 0) return false;
	if (prt & 8) {
		pbb_zx = val;
		gs->pstate |= 1;
	} else {
		pb3_zx = val;
		gs->pstate |= 0x80;	// set b7,state
	}
	return true;
}

// internal in/out

uint8_t gs_in(int prt) {
	uint8_t res = 0xff;
	prt &= 0x0f;
	switch (prt) {
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
//printf("GS in %i = %.2X\n",prt,res);
	return res;
}

void gs_out(int prt,uint8_t val) {
	prt &= 0x0f;
	switch (prt) {
		case 0: gs->rp0 = val;
			val &= 7;
			if ((val & 7) == 0) {
				gs->sys->mem->pt2 = &gs->sys->mem->rom[0][0];
				gs->sys->mem->pt3 = &gs->sys->mem->rom[1][0];
			} else {
				gs->sys->mem->pt2 = &gs->sys->mem->ram[val*2 - 2][0];
				gs->sys->mem->pt3 = &gs->sys->mem->ram[val*2 - 1][0];
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
//printf("GS out %i,%.2X\n",prt,val);
}
