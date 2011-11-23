#include "ayym.h"

bool noizes[0x20000];		// here iz noize values [generated at start]
uint8_t envforms[16][33]={
/*	  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32	*/
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,253},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,253},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,253},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,253},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255}
};

AYChan::AYChan() {
	lev = false;
	vol = 0;//lim = bgn = pos = cur = 0;
	period = 0;
	counter = 0;
}

AYSys::AYSys() {
	sc1 = new AYProc(SND_AY);
	sc2 = new AYProc(SND_NONE);
	tstype = TS_NEDOPC;
}

AYProc::AYProc(int t) {
	stereo = AY_MONO;
	settype(t);
}

void AYProc::settype(int t) {
	type = t;
	switch (type) {
		case SND_NONE:
			freq = 1;
			break;
		case SND_AY:
			freq = 1774400;
			break;
		case SND_YM:
			freq = 1750000;
			break;
		default: throw("Internal error\nUnexpected AY type in AYProc::settype"); break;
	}
	aycoe = 400 * 448 * 320 / (float)freq;		// vid ticks in half-period of note 1 (400)	400 * zx->vid->frmsz / (float)freq
}

void AYProc::sync(int tk) {
	if (type == SND_NONE) return;
	if (a.period != 0) {
		a.counter -= tk;
		while (a.counter < 0) {
			a.lev = !a.lev;
			a.counter += a.period;
		}
	}
	if (b.period != 0) {
		b.counter -= tk;
		while (b.counter < 0) {
			b.lev = !b.lev;
			b.counter += b.period;
		}
	}
	if (c.period != 0) {
		c.counter -= tk;
		while (c.counter < 0) {
			c.lev = !c.lev;
			c.counter += c.period;
		}
	}
	if (n.period != 0) {
		n.counter -= tk;
		while (n.counter < 0) {
			n.counter += n.period;
			nPos++;
		}
		n.lev = noizes[nPos & 0x1ffff];
	}
	if (e.period != 0) {
		e.counter -= tk;
		while (e.counter < 0) {
			e.counter += e.period;
			ePos++;
			switch (envforms[eCur][ePos]) {
				case 255: ePos--; break;
				case 253: ePos = 0; break;
			}
		}
	}
}

AYData AYProc::getvol() {
	AYData res;
	res.l = res.r = 8;
	if (type == SND_NONE) return res;
// mix channels, envelope & noise
	a.vol = (reg[8] & 16) ? envforms[eCur][ePos] : (reg[8] & 15);
	b.vol = (reg[9] & 16) ? envforms[eCur][ePos] : (reg[9] & 15);
	c.vol = (reg[10] & 16) ? envforms[eCur][ePos] : (reg[10] & 15);
	if ((reg[7] & 0x09) != 0x09) {a.vol *= ((reg[7] & 1) ? 0 : a.lev) + ((reg[7] & 8) ? 0 : n.lev);}
	if ((reg[7] & 0x12) != 0x12) {b.vol *= ((reg[7] & 2) ? 0 : b.lev) + ((reg[7] & 16) ? 0 : n.lev);}
	if ((reg[7] & 0x24) != 0x24) {c.vol *= ((reg[7] & 4) ? 0 : c.lev) + ((reg[7] & 32) ? 0 : n.lev);}
	switch (stereo) {
		case AY_ABC:
			res.l = a.vol + 0.7 * b.vol;
			res.r = c.vol + 0.7 * b.vol;
			break;
		case AY_ACB:
			res.l = a.vol + 0.7 * c.vol;
			res.r = b.vol + 0.7 * c.vol;
			break;
		case AY_BAC:
			res.l = b.vol + 0.7 * a.vol;
			res.r = c.vol + 0.7 * a.vol;
			break;
		case AY_BCA:
			res.l = b.vol + 0.7 * c.vol;
			res.r = a.vol + 0.7 * c.vol;
			break;
		case AY_CAB:
			res.l = c.vol + 0.7 * a.vol;
			res.r = b.vol + 0.7 * a.vol;
			break;
		case AY_CBA:
			res.l = c.vol + 0.7 * b.vol;
			res.r = a.vol + 0.7 * b.vol;
			break;
		default:
			res.l = res.r = (a.vol + b.vol + c.vol) * 0.7;	// mono
			break;
	}
	return res;
}

void AYProc::reset() {
	int i; for (i = 0;i < 16;i++) reg[i] = 0;
	eCur = 0;
	nPos = 0;
	ePos = 0;
	a.period = b.period = c.period = n.period = e.period = 0;
	a.counter = b.counter = c.counter = n.counter = e.counter = 0;
}

void AYProc::setreg(uint8_t value) {
	if (curreg > 15) return;
	if (curreg < 14) reg[curreg]=value;
	switch (curreg) {
		case 0x00:
		case 0x01: a.period = aycoe * (reg[0] + ((reg[1] & 0x0f) << 8)); break;
		case 0x02:
		case 0x03: b.period = aycoe * (reg[2] + ((reg[3] & 0x0f) << 8)); break;
		case 0x04:
		case 0x05: c.period = aycoe * (reg[4] + ((reg[5] & 0x0f) << 8)); break;
		case 0x06: n.period = 2 * aycoe * (value & 0x1f); break;
		case 0x0b:
		case 0x0c: e.period = 2 * aycoe * (reg[11] + (reg[12] << 8)); break;
		case 0x0d: eCur = value & 0x0f; ePos = 0; e.counter = 0; break;
		case 0x0e: if (reg[7] & 0x40) reg[14] = value; break;
		case 0x0f: if (reg[7] & 0x80) reg[15] = value; break;
	}
}

// overall

void initNoise() {
	uint32_t cur = 0xffff;
	bool lev;
	for (int i=0; i<0x20000; i++) {
		lev = cur & 0x10000;
		noizes[i] = lev;
		cur = ((cur << 1) + ((lev == ((cur & 0x2000) == 0x2000)) ? 0 : 1)) & 0x1ffff;
	}
}