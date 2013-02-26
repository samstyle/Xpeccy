#ifndef _XPGSOUND
#define _XPGSOUND

#ifdef __cplusplus
extern "C" {
#endif

#include "z80ex.h"
#include "memory.h"

// gs flags
#define GS_ENABLE	1
#define GS_RESET	(1<<1)
// gs stereo
#define	GS_MONO		0
#define	GS_12_34	1
// return values
#define	GS_OK	0
#define	GS_ERR	1
// gs cpu freq
#define	GS_FRQ	12.0

typedef struct {
	unsigned char left;
	unsigned char right;
} gsPair;

typedef struct {
	int flag;
	Z80EX_CONTEXT* cpu;
	Memory* mem;
	unsigned char pb3_gs;	// gs -> zx
	unsigned char pb3_zx;	// zx -> gs
	unsigned char pbb_zx;
	unsigned char rp0;
	unsigned char pstate;	// state register (d0, d7)
	int vol1,vol2,vol3,vol4;
	int ch1,ch2,ch3,ch4;
	int cnt;
	int stereo;
	int sync;
	long counter;
} GSound;

GSound* gsCreate();
void gsDestroy(GSound*);
void gsReset(GSound*);
void gsSync(GSound*);
gsPair gsGetVolume(GSound*);

int gsIn(GSound*, int, unsigned char*);
int gsOut(GSound*, int, unsigned char);

void gsSetRom(GSound*,int,char*);

#ifdef __cplusplus
}
#endif

#endif
