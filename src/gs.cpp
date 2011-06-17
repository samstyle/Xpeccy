#include "gs.h"

//extern GS* gs;

GS::GS() {
	sys = new ZXBase;
	sys->cpu = new Z80(GS_FRQ);
	sys->mem = new Memory(MEM_GS);
	sys->io = new IOSys(IO_GS);
	sys->mem->pt0 = &sys->mem->rom[0][0];
	sys->mem->pt1 = &sys->mem->ram[0][0];
	sys->mem->pt2 = &sys->mem->ram[0][0];
	sys->mem->pt3 = &sys->mem->ram[1][0];
	t = cnt = 0;
	pstate = 0x7e;
	flags = GS_ENABLE;
	stereo = GS_12_34;
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
	ZOpResult res;
	int ln = (tk - t) * GS_FRQ / 7.0;		// scale to GS ticks;
	t = tk;
	if (~flags & GS_ENABLE) return;
	while (ln > 0) {
		res = sys->fetch();
		res.exec(sys);
		ln -= res.ticks;
		cnt += res.ticks;
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
		pstate &= 0x7f;		// reset b7,state
	}
	return true;
}

bool GS::out(int prt,uint8_t val) {
	if (~flags & GS_ENABLE) return false;
	if ((prt & 0x44) != 0) return false;
	if (prt & 8) {
		pbb_zx = val;
		pstate |= 1;
	} else {
		pb3_zx = val;
		pstate |= 0x80;	// set b7,state
	}
	return true;
}

// internal in/out

uint8_t GS::intin(int32_t prt) {
	uint8_t res = 0xff;
	prt &= 0x0f;
	switch (prt) {
		case 0: break;
		case 1: res = pbb_zx; break;
		case 2: pstate &= 0x7f; res = pb3_zx; break;
		case 3: pstate |= 0x80; break;
		case 4: res = pstate; break;
		case 5: pstate &= 0xfe; break;
		case 6: break;
		case 7: break;
		case 8: break;
		case 9: break;
		case 10: if (rp0 & 0x01) pstate &= 0x7f; else pstate |= 0x80; break;
		case 11: if (vol1 & 0x20) pstate |= 1; else pstate &= 0xfe; break;
	}
	return res;
}

void GS::intout(int32_t prt,uint8_t val) {
	prt &= 0x0f;
	switch (prt) {
		case 0: rp0 = val;
			val &= 7;
			if ((val & 7) == 0) {
				sys->mem->pt2 = &sys->mem->rom[0][0];
				sys->mem->pt3 = &sys->mem->rom[1][0];
			} else {
				sys->mem->pt2 = &sys->mem->ram[val*2 - 2][0];
				sys->mem->pt3 = &sys->mem->ram[val*2 - 1][0];
			}
			break;
		case 1: break;
		case 2: pstate &= 0x7f; break;
		case 3: pstate |= 0x80; pb3_gs = val; break;
		case 4: break;
		case 5: pstate &= 0xfe; break;
		case 6: vol1 = val & 0x3f; break;
		case 7: vol2 = val & 0x3f; break;
		case 8: vol3 = val & 0x3f; break;
		case 9: vol4 = val & 0x3f; break;
		case 10: if (rp0 & 1) pstate &= 0x7f; else pstate |= 0x80; break;
		case 11: if (vol1 & 64) pstate |= 1; else pstate &= 0xfe; break;
	}
}
