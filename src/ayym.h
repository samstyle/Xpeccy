#ifndef _XPAYYM
#define _XPAYYM

#include <stdint.h>

#define	SND_NONE	0
#define	SND_AY		1
#define	SND_YM		2
#define	SND_END		3

#define	AY_MONO		0
#define	AY_ABC		1
#define	AY_ACB		2
#define	AY_BAC		3
#define	AY_BCA		4
#define	AY_CAB		5
#define	AY_CBA		6

#define	TS_NONE		0
#define	TS_NEDOPC	1

/*
class SndData {
	public:
		int r;
		int l;
		friend SndData operator +(SndData p1, SndData p2) {p1.r += p2.r; p1.l += p2.l; return p1;}
		friend SndData operator *(SndData p1, float mul) {p1.r *= mul; p1.l *= mul; return p1;}
};
*/

struct AYData {
	int r;
	int l;
};

class AYChan {
	public:
		AYChan();
		bool lev;		// for A,B,C,noise : signal level
		uint8_t vol;		// volume
		float lim;		// period
		uint32_t bgn;		// 1st tick (for tickless snd)
		uint32_t pos,cur;	// for envelope; noise.cur = nvalue
};

class AYProc {
	public:
		AYProc(int32_t);
		int32_t type;
		int32_t stereo;
		uint8_t reg[16];
		AYChan a,b,c,e,n;
		int32_t freq;
		uint8_t curreg;
		float aycoe;
		void reset(uint32_t);
		void setreg(uint8_t);
		void settype(int32_t);
		AYData getvol();
		void calculate();
};

class AYSys {
	public:
		AYSys();
		AYProc* sc1;
		AYProc* sc2;
		AYProc* scc;
		int32_t tstype;
};

void initNoise();

#endif
