#ifndef _LIBXPECCY_HARDWARE
#define _LIBXPECCY_HARDWARE

#ifdef __cplusplus
extern "C" {
#endif

#include "../spectrum.h"

// hw type
#define HW_NULL		0
#define HW_ZX48		1
#define	HW_PENT		2
#define	HW_P1024	3
#define	HW_SCORP	4
#define	HW_PLUS2	5
#define	HW_PLUS3	6
#define	HW_ATM1		7
#define	HW_ATM2		8
#define	HW_PENTEVO	9
#define	HW_TSLAB	10
#define HW_PROFI	11
#define HW_MSX		12
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

// msx
void msxMapMem(Computer*);
void msxOut(Computer*,unsigned short,unsigned char,int);
unsigned char msxIn(Computer*,unsigned short,int);
void msxReset(Computer*);
void msxMWr(Computer*,unsigned short,unsigned char);

#ifdef __cplusplus
}
#endif

#endif
