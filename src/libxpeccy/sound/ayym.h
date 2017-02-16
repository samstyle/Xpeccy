#ifndef _XPAYYM
#define _XPAYYM

#ifdef __cplusplus
extern "C" {
#endif

#include "sndcommon.h"

// TODO: move to own ticks as in gbsound
// 1T = 1e9 / frq ns
// sync: decrease current tick ns counter, change channels on tick borders

// ay_type
enum {
	SND_NONE = 0,
	SND_AY,
	SND_YM,
	SND_YM2203,	// FM
	SND_END
};
// ay_stereo
enum {
	AY_MONO = 0,
	AY_ABC,
	AY_ACB,
	AY_BAC,
	AY_BCA,
	AY_CAB,
	AY_CBA
};
// ts_type
enum {
	TS_NONE = 0,
	TS_NEDOPC
};

#include "sndcommon.h"

typedef struct {
	unsigned ten:1;		// tone on
	unsigned nen:1;		// noise on
	unsigned een:1;		// envelope on
	unsigned lev:1;		// current signal level
	int vol;
	int per;		// period in ticks (0:channel off)
	int cnt;		// ticks countdown
	int step;		// ++ each tick (used for envelope, noise)
} aymChan;

typedef struct {
	unsigned coarseEnv:1;	// 4-bit envelope volume
	int type;
	int stereo;
	double frq;		// in MHz
	aymChan chanA;
	aymChan chanB;
	aymChan chanC;
	aymChan chanN;
	aymChan chanE;
	int eForm;		// envelope form
	int per;		// pariod ns len
	int cnt;		// ns countdown
	unsigned char curReg;
	unsigned char reg[256];
} aymChip;

typedef struct {
	int type;
	aymChip* chipA;
	aymChip* chipB;
	aymChip* curChip;
} TSound;

void initNoise();

void aymSetType(aymChip*, int);
int ayGetChanVol(aymChip*, aymChan*);

TSound* tsCreate(int,int,int);
void tsDestroy(TSound*);
void tsReset(TSound*);
unsigned char tsIn(TSound*,int);
void tsOut(TSound*,int,unsigned char);

void tsSync(TSound*, long);

sndPair tsGetVolume(TSound*);

#ifdef __cplusplus
}
#endif

#endif
