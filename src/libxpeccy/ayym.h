#ifndef _XPAYYM
#define _XPAYYM

#ifdef __cplusplus
extern "C" {
#endif

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
	unsigned char left;
	unsigned char right;
} tsPair;

typedef struct {
	int lev;
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
	unsigned char reg[16];
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
tsPair tsGetVolume(TSound*);

#ifdef __cplusplus
}
#endif

#endif
