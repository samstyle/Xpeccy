#ifndef _XPGSOUND
#define _XPGSOUND

#include "spectrum.h"
#include "sound.h"

#define GS_FRQ		12.0

#define GS_ENABLE	1
#define GS_RESET	2

#define	GS_MONO		0
#define	GS_12_34	1

unsigned char gs_in(int);
void gs_out(int,unsigned char);

struct GS {
	GS();
	int flags;
	Spec* sys;
	unsigned int t;
	unsigned char pb3_gs;	// gs -> zx
	unsigned char pb3_zx;	// zx -> gs
	unsigned char pbb_zx;
	unsigned char rp0;
	unsigned char pstate;	// state register (d0, d7)
	int vol1,vol2,vol3,vol4;
	int ch1,ch2,ch3,ch4;
	int cnt;
	int stereo;
	void reset();
	bool in(int,unsigned char*);
	bool out(int,unsigned char);
	void sync(unsigned int);
	SndData getvol();
};

extern GS *gs;

#endif
