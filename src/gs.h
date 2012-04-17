#ifndef _XPGSOUND
#define _XPGSOUND

#include <stdint.h>
#include <utility>

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
	int flag;
	Z80EX_CONTEXT* cpu;
	Memory* mem;
	uint8_t pb3_gs;	// gs -> zx
	uint8_t pb3_zx;	// zx -> gs
	uint8_t pbb_zx;
	uint8_t rp0;
	uint8_t pstate;	// state register (d0, d7)
	int vol1,vol2,vol3,vol4;
	int ch1,ch2,ch3,ch4;
	int cnt;
	int stereo;
	double counter;
} GSound;

GSound* gsCreate();
void gsDestroy(GSound*);
void gsReset(GSound*);
void gsSync(GSound*, int);
std::pair<uint8_t,uint8_t> gsGetVolume(GSound*);

int gsIn(GSound*, int, uint8_t*);
int gsOut(GSound*, int, uint8_t);

void gsSetRom(GSound*,int,char*);

#endif
