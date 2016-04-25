#ifndef _XPAYYM
#define _XPAYYM

#ifdef __cplusplus
extern "C" {
#endif

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
	unsigned lev:1;
	int tone;	// 12 bit of tone registers
	double period;
	double count;
} aymChan;

typedef struct {
	int type;
	int stereo;
	aymChan chanA;
	aymChan chanB;
	aymChan chanC;
	aymChan chanN;
	aymChan chanE;
	int eCur;
	int ePos;
	int nPos;
	float freq;		// in MHz
	int aycoe;
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

TSound* tsCreate(int,int,int);
void tsDestroy(TSound*);
void tsReset(TSound*);
unsigned char tsIn(TSound*,int);
void tsOut(TSound*,int,unsigned char);
void tsSync(TSound*,int);
sndPair tsGetVolume(TSound*);

#ifdef __cplusplus
}
#endif

#endif
