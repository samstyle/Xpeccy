#ifndef SAA1099_H
#define SAA1099_H

#include "sndcommon.h"

enum {
	SAA_OFF = 0,
	SAA_MONO,
	SAA_STEREO
};

enum {
	SAA_ARG_MODE = 0
};

typedef struct {
	unsigned lev:1;
	unsigned freqEn:1;
	unsigned noizEn:1;
	int ampLeft;
	int ampRight;
	int octave;
	int freq;
	int period;
	int count;
} saaChan;

typedef struct {
	struct {
		unsigned update:1;
		unsigned extCLK:1;
		unsigned invRight:1;
		int form;
		int period;
	} buf;
	unsigned enable:1;
	unsigned invRight:1;	// inverse right channel
	unsigned lowRes:1;	// 3-bit env control;
	unsigned extCLK:1;	// external frq control (8MHz)
//	unsigned busy:1;
	unsigned char vol;
	int form;
	int period;
	int count;
	int pos;
} saaEnv;

typedef struct {
	unsigned lev:1;
	int period;
	int count;
	int pos;
} saaNoise;

typedef struct {
	unsigned enabled:1;	// options : present/not present
//	unsigned mono:1;
	unsigned off:1;		// software control : Reg 1C bit 0
	int time;
	int curReg;
	saaChan chan[6];	// 6 8-bit channels
	saaNoise noiz[2];	// 2 noize channels (mix ch 0,1,2 & 3,4,5; generators from ch 0,3)
	saaEnv env[2];		// 2 envelope (aff ch 2 & 5, generators from ch 1, 4)
} saaChip;

saaChip* saaCreate();
void saaDestroy(saaChip*);
void saaReset(saaChip*);
int saaWrite(saaChip*, int, int);
void saaSync(saaChip*, int);
sndPair saaGetVolume(saaChip*);
// void saaFlush(saaChip*);
sndPair saaVolume(saaChip*);

#endif
