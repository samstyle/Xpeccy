#ifndef X_AYYM_H
#define X_AYYM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sndcommon.h"

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
	int step;		// env:vol change direction (+1 -1); noise:seed
} aymChan;

typedef struct {
	unsigned coarse:1;	// 4-bit DAC volume
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

void tsSync(TSound*, int);

sndPair tsGetVolume(TSound*);
sndPair aymGetVolume(aymChip*);

#ifdef __cplusplus
}
#endif

#endif
