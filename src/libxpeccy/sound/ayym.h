#pragma once

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
	TS_NEDOPC,
	TS_ZXNEXT
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

// yamaha-2203
void ym2203_reset(aymChip*);
int ym2203_rd(aymChip*, int);
void ym2203_wr(aymChip*, int, int);
void ym2203_sync(aymChip*, int);
sndPair ym2203_vol(aymChip*);

typedef void(*sccbwr)(aymChip*, int, int);
typedef int(*sccbrd)(aymChip*, int);
typedef void(*sccbsync)(aymChip*, int);
typedef sndPair(*sccbvol)(aymChip*);
typedef void(*sccbcmn)(aymChip*);

typedef int(*ayxrd)(int, void*);
typedef void(*ayxwr)(int, int, void*);

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

enum {
	OPST_OFF = 0,
	OPST_ATK,
	OPST_DEC,
	OPST_SUS,
	OPST_REL
};

typedef struct {
	unsigned key:1;
	int feedback;	// op1 only
	int tlev;	// 0:max, 1024:min
	struct {	// phase generator
		unsigned phase:20;		// 20-bit phase 10.10 (max is 2*pi)
		int freq;
		int block;
		int pstep;			// phase step
		int mult;
		int detune;
	} pg;
	struct {		// envelope generator
		int state;	// atk/dec/sus/rel/off
		int ks;		// from reg.value (2bits)
		int kscale;	// calculated (0-31)
		int atkrate;
		int decrate;
		int susrate;
		int suslev;	// [0;1024]
		int relrate;
		int envflag;
		int att;			// 0:max, 1023:min
		int out;			// att + tlev;
	} eg;
	int out;
	int outp;	// previous output for op1 (feedback)
} fmOper;

typedef struct {
	unsigned off:1;		// output = 0
	fmOper op[4];		// operators
	int algo;		// ops connection (algorithm)
	int out;		// output (last operator output)
} fmChan;

struct aymChip {
	unsigned coarse:1;	// 4-bit DAC volume
	unsigned blk_fm:1;	// 1:block fm output
	int stereo;

	int type;
	double frq;		// in MHz
	sccbcmn res;
	sccbrd rd;
	sccbwr wr;
	sccbsync sync;
	sccbvol vol;

	ayxrd xrd;		// read/write callbacks for ports 14,15
	ayxwr xwr;
	void* xptr;

	aymChan chanA;		// psg/ssg channels
	aymChan chanB;
	aymChan chanC;
	aymChan chanN;
	aymChan chanE;
	int eForm;		// envelope form
	int per;		// period ns len
	int cnt;		// ns countdown

	int pscnt;	// pre-scaler: (2,3,6) of master ticks
	int fmcnt;	// fm: 12 pre-scaled ticks
	int eg_timer;	// eg: 3 fm ticks
	unsigned eg_cnt:12;// inc each eg tick (12 bit)
	int sg_cnt;	// ssg divider counter

	fmChan chanFM[3];	// fm channels
	int fmdiv;		// divider for fm (2/3/6)
	int sgdiv;		// divider for ssg
	int divmode:2;		// 2bits
	int ta_value;		// timerA initial value
	int ta_cnt;		// timerA: 2 eg ticks
	int tb_cnt;		// timerB: 32 eg ticks

	unsigned char curReg;
	unsigned char reg[256];
} ;

typedef struct {
	unsigned mute_l:1;
	unsigned mute_r:1;
	unsigned r_stat:1;	// read status reg instead of chip regs
	int type;

	struct {
		unsigned char* data;
		int size;
		int mask;
	} rom;

	aymChip* chipA;
	aymChip* chipB;
	aymChip* chipC;
	aymChip* chipD;
	aymChip* curChip;
} TSound;

void initNoise();
void init_sin_tab();

void chip_set_type(aymChip*, int);
void chip_set_xdev(aymChip*, ayxrd, ayxwr, void*);

TSound* tsCreate(int,int,int);
void tsDestroy(TSound*);
void tsReset(TSound*);
int tsIn(TSound*,int);
void tsOut(TSound*,int,int);
void tsSync(TSound*, int);
void tsSetRomSize(TSound*, int);
void tsLoadRom(TSound*, const char*);
int tsReadRom(TSound*, int);

sndPair tsGetVolume(TSound*);

#ifdef __cplusplus
}
#endif
