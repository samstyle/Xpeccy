#ifndef _LIBXPECCY_HARDWARE
#define _LIBXPECCY_HARDWARE

#ifdef __cplusplus
extern "C" {
#endif

#include "../spectrum.h"
#include <assert.h>

// hw type
enum {
	HW_NULL = 0,
	HW_ZX48,	// ZX48K (Classic)
	HW_PENT,	// Pentagon
	HW_P1024,	// Pentagon1024SL
	HW_SCORP,	// ZS Scorpion
	HW_PLUS2,	// Spectrum 2+
	HW_PLUS3,	// Spectrum 3+
	HW_ATM1,	// ATM 1
	HW_ATM2,	// ATM 2+
	HW_PENTEVO,	// ZXEvo (Baseconf)
	HW_TSLAB,	// PentEvo (TSConf)
	HW_PROFI,	// Profi
	HW_PHOENIX,	// ZXM Phoenix
	HW_MSX,		// MSX 1
	HW_MSX2		// MSX 2
};

// mem size
#define	MEM_48	0
#define	MEM_128	1
#define	MEM_256 (1<<1)
#define	MEM_512	(1<<2)
#define	MEM_1M	(1<<3)
#define	MEM_2M	(1<<4)
#define	MEM_4M	(1<<5)

struct HardWare {
	const char* name;
	const char* optName;
	int type;
	int mask;		// mem size bits (b0:128, b1:256, b2:512, b3:1M, b4:2M, b5:4M); =0 for 48K
	void (*mapMem)(Computer*);
	void (*out)(Computer*,unsigned short,unsigned char,int);
	unsigned char (*in)(Computer*,unsigned short,int);
	unsigned char (*mrd)(Computer*,unsigned short,int);
	void (*mwr)(Computer*,unsigned short,unsigned char);
	void (*reset)(Computer*);
};

typedef struct {
	int mask;		// if (port & mask == value & mask) port is catched
	int value;
	unsigned dos:2;		// 00:!dos only; 01:dos only; 1x:nevermind
	unsigned rom:2;		// same: b4,7FFD
	unsigned cpm:2;		// cpm mode (for profi)
	unsigned char (*in)(Computer*, unsigned short);
	void (*out)(Computer*, unsigned short, unsigned char);
} xPort;

unsigned char hwIn(xPort*, Computer*, unsigned short, int);
void hwOut(xPort*, Computer*, unsigned short, unsigned char, int);

typedef struct HardWare HardWare;
extern HardWare hwTab[];

HardWare* findHardware(const char*);
unsigned char stdMRd(Computer*,unsigned short,int);
void stdMWr(Computer*,unsigned short,unsigned char);

void beepSync(Computer*);

// debug IO

unsigned char brkIn(Computer*, unsigned short);
void brkOut(Computer*, unsigned short, unsigned char);

unsigned char dummyIn(Computer*, unsigned short);
void dummyOut(Computer*, unsigned short, unsigned char);

// common IO

void xOutFE(Computer*, unsigned short, unsigned char);
void xOutBFFD(Computer*, unsigned short, unsigned char);
void xOutFFFD(Computer*, unsigned short, unsigned char);

unsigned char xIn1F(Computer*, unsigned short);
unsigned char xInFE(Computer*, unsigned short);
unsigned char xInFFFD(Computer*, unsigned short);
unsigned char xInFADF(Computer*, unsigned short);
unsigned char xInFBDF(Computer*, unsigned short);
unsigned char xInFFDF(Computer*, unsigned short);

// zx48
void speMapMem(Computer*);
void speOut(Computer*,unsigned short,unsigned char,int);
unsigned char speIn(Computer*,unsigned short,int);
void speReset(Computer*);

// pentagon
void penMapMem(Computer*);
void penOut(Computer*,unsigned short,unsigned char,int);
unsigned char penIn(Computer*,unsigned short,int);

// p1024sl
void p1mMapMem(Computer*);
void p1mOut(Computer*,unsigned short,unsigned char,int);
unsigned char p1mIn(Computer*,unsigned short,int);

// scorpion
void scoMapMem(Computer*);
void scoOut(Computer*,unsigned short,unsigned char,int);
unsigned char scoIn(Computer*,unsigned short,int);
unsigned char scoMRd(Computer*,unsigned short,int);

// plus 2
void pl2MapMem(Computer*);
void pl2Out(Computer*,unsigned short,unsigned char,int);
unsigned char pl2In(Computer*,unsigned short,int);

// plus 3
// void pl3MapMem(ZXComp*);		// = pl2MapMem
void pl3Out(Computer*,unsigned short,unsigned char,int);
unsigned char pl3In(Computer*,unsigned short,int);

// atm 2
void atm2MapMem(Computer*);
void atm2Out(Computer*,unsigned short,unsigned char,int);
unsigned char atm2In(Computer*,unsigned short,int);
void atm2Reset(Computer*);

// pentevo
void evoMapMem(Computer*);
void evoOut(Computer*,unsigned short,unsigned char,int);
unsigned char evoIn(Computer*,unsigned short,int);
unsigned char evoMRd(Computer*,unsigned short,int);
void evoMWr(Computer*,unsigned short,unsigned char);
void evoReset(Computer*);

// TSLab conf
void tslMapMem(Computer*);
void tslOut(Computer*,unsigned short,unsigned char,int);
unsigned char tslIn(Computer*,unsigned short,int);
unsigned char tslMRd(Computer*,unsigned short,int);
void tslMWr(Computer*,unsigned short,unsigned char);
void tslReset(Computer*);
void tslUpdatePorts(Computer*);

// Profi
void prfMapMem(Computer*);
void prfOut(Computer*,unsigned short,unsigned char,int);
unsigned char prfIn(Computer*,unsigned short,int);
void prfReset(Computer*);
unsigned char prfMRd(Computer*,unsigned short,int);

// ZXM Phoenix
void phxMapMem(Computer*);
void phxOut(Computer*,unsigned short,unsigned char,int);
unsigned char phxIn(Computer*,unsigned short,int);
void phxReset(Computer*);

// msx
void msxMapMem(Computer*);
void msxOut(Computer*,unsigned short,unsigned char,int);
unsigned char msxIn(Computer*,unsigned short,int);
void msxReset(Computer*);
unsigned char msxMRd(Computer*,unsigned short,int);
void msxMWr(Computer*,unsigned short,unsigned char);

// msx2
void msx2mapper(Computer*);
void msx2Out(Computer*,unsigned short,unsigned char,int);
unsigned char msx2In(Computer*,unsigned short,int);
void msx2Reset(Computer*);
unsigned char msx2mrd(Computer*,unsigned short,int);
void msx2mwr(Computer*,unsigned short,unsigned char);

#ifdef __cplusplus
}
#endif

#endif
