#ifndef _XPAYYM
#define _XPAYYM

#include <stdint.h>
#include <utility>

// ay_type
#define	SND_NONE	0
#define	SND_AY		1
#define	SND_YM		2
#define	SND_END		3
// ay_stereo
#define	AY_MONO		0
#define	AY_ABC		1
#define	AY_ACB		2
#define	AY_BAC		3
#define	AY_BCA		4
#define	AY_CAB		5
#define	AY_CBA		6
// ts_type
#define	TS_NONE		0
#define	TS_NEDOPC	1

// structures

typedef struct {
	bool lev;
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
	int freq;
	double aycoe;
	uint8_t curReg;
	uint8_t reg[16];
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
uint8_t tsIn(TSound*,int);
void tsOut(TSound*,int,uint8_t);
void tsSync(TSound*,int);
std::pair<uint8_t,uint8_t> tsGetVolume(TSound*);

#endif
