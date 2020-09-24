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

typedef struct aymChip aymChip;

// ay-3-8910
void ay_reset(aymChip*);
int ay_rd(aymChip*, int);
void ay_wr(aymChip*, int, int);
void ay_sync(aymChip*, int);
sndPair ay_vol(aymChip*);

// yamaha-2149
//void ym_reset(aymChip*);
int ym_rd(aymChip*, int);
void ym_wr(aymChip*, int, int);
//void ym_sync(aymChip*, int);
sndPair ym_vol(aymChip*);

typedef void(*sccbwr)(aymChip*, int, int);
typedef int(*sccbrd)(aymChip*, int);
typedef void(*sccbsync)(aymChip*, int);
typedef sndPair(*sccbvol)(aymChip*);
typedef void(*sccbcmn)(aymChip*);

typedef struct {
	int id;
	const char* name;
	const char* short_name;
	double frq;
	sccbcmn res;
	sccbrd rd;
	sccbwr wr;
	sccbsync sync;
	sccbvol vol;
} scDesc;

typedef struct {
	unsigned tdis:1;	// tone off
	unsigned ndis:1;	// noise off
	unsigned een:1;		// envelope on
	unsigned lev:1;		// current signal level
	int vol;
	int per;		// period in ticks (0:channel off)
	int cnt;		// ticks countdown
	int step;		// env:vol change direction (+1 -1); noise:seed
} aymChan;

struct aymChip {
	unsigned coarse:1;	// 4-bit DAC volume
	int stereo;

	int type;
	double frq;		// in MHz
	sccbcmn res;
	sccbrd rd;
	sccbwr wr;
	sccbsync sync;
	sccbvol vol;

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
} ;

typedef struct {
	int type;
	aymChip* chipA;
	aymChip* chipB;
	aymChip* curChip;
} TSound;

void initNoise();

void chip_set_type(aymChip*, int);
//void aymSetType(aymChip*, int);
//int ayGetChanVol(aymChip*, aymChan*);

TSound* tsCreate(int,int,int);
void tsDestroy(TSound*);
void tsReset(TSound*);
int tsIn(TSound*,int);
void tsOut(TSound*,int,int);

void tsSync(TSound*, int);

sndPair tsGetVolume(TSound*);
//sndPair aymGetVolume(aymChip*);

#ifdef __cplusplus
}
#endif

#endif
