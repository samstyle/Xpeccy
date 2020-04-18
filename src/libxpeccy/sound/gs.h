#ifndef X_GSOUND
#define X_GSOUND

#ifdef __cplusplus
extern "C" {
#endif

#include "sndcommon.h"

#include "cpu.h"
#include "memory.h"

// gs stereo
enum {
	GS_MONO = 0,
	GS_12_34
};

enum {
	GS_ARG_ENABLE = 0,
	GS_ARG_RESET,
	GS_ARG_STEREO,
	GS_ARG_ROM
};

// gs cpu freq
#define	GS_FRQ	12.0

#include "sndcommon.h"

typedef struct {
	unsigned enable:1;
	unsigned reset:1;

	CPU* cpu;
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
	int time;
	long counter;
} GSound;

GSound* gsCreate();
void gsDestroy(GSound*);
void gsReset(GSound*);
void gsSync(GSound*, int);
void gsFlush(GSound*);
sndPair gsVolume(GSound*);
int gsWrite(GSound*, int, int);
int gsRead(GSound*, int, int*);

#ifdef __cplusplus
}
#endif

#endif
