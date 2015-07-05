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
	void (*mapMem)(ZXComp*);
	void (*out)(ZXComp*,unsigned short,unsigned char,int);
	unsigned char (*in)(ZXComp*,unsigned short,int);
	unsigned char (*mrd)(ZXComp*,unsigned short,int);
	void (*mwr)(ZXComp*,unsigned short,unsigned char);
	void (*reset)(ZXComp*);
};

typedef struct {
	int mask;		// if (port & mask == value & mask) port is catched
	int value;
	unsigned dos:2;		// 00:!dos only; 01:dos only; 1x:nevermind
	unsigned rom:2;		// same: b4,7FFD
	unsigned cpm:2;		// cpm mode (for profi)
	unsigned char (*in)(ZXComp*, unsigned short);
	void (*out)(ZXComp*, unsigned short, unsigned char);
} xPort;

unsigned char hwIn(xPort*, ZXComp*, unsigned short, int);
void hwOut(xPort*, ZXComp*, unsigned short, unsigned char, int);

typedef struct HardWare HardWare;
extern HardWare hwTab[];

HardWare* findHardware(const char*);
unsigned char stdMRd(ZXComp*,unsigned short,int);
void stdMWr(ZXComp*,unsigned short,unsigned char);

// debug IO

unsigned char brkIn(ZXComp*, unsigned short);
void brkOut(ZXComp*, unsigned short, unsigned char);

unsigned char dummyIn(ZXComp*, unsigned short);
void dummyOut(ZXComp*, unsigned short, unsigned char);

// common IO

void xOutFE(ZXComp*, unsigned short, unsigned char);
void xOutBFFD(ZXComp*, unsigned short, unsigned char);
void xOutFFFD(ZXComp*, unsigned short, unsigned char);

unsigned char xIn1F(ZXComp*, unsigned short);
unsigned char xInFE(ZXComp*, unsigned short);
unsigned char xInFFFD(ZXComp*, unsigned short);
unsigned char xInFADF(ZXComp*, unsigned short);
unsigned char xInFBDF(ZXComp*, unsigned short);
unsigned char xInFFDF(ZXComp*, unsigned short);

// zx48
void speMapMem(ZXComp*);
void speOut(ZXComp*,unsigned short,unsigned char,int);
unsigned char speIn(ZXComp*,unsigned short,int);
void speReset(ZXComp*);

// pentagon
void penMapMem(ZXComp*);
void penOut(ZXComp*,unsigned short,unsigned char,int);
unsigned char penIn(ZXComp*,unsigned short,int);

// p1024sl
void p1mMapMem(ZXComp*);
void p1mOut(ZXComp*,unsigned short,unsigned char,int);
unsigned char p1mIn(ZXComp*,unsigned short,int);

// scorpion
void scoMapMem(ZXComp*);
void scoOut(ZXComp*,unsigned short,unsigned char,int);
unsigned char scoIn(ZXComp*,unsigned short,int);
unsigned char scoMRd(ZXComp*,unsigned short,int);

// plus 2
void pl2MapMem(ZXComp*);
void pl2Out(ZXComp*,unsigned short,unsigned char,int);
unsigned char pl2In(ZXComp*,unsigned short,int);

// plus 3
// void pl3MapMem(ZXComp*);		// = pl2MapMem
void pl3Out(ZXComp*,unsigned short,unsigned char,int);
unsigned char pl3In(ZXComp*,unsigned short,int);

// atm 2
void atm2MapMem(ZXComp*);
void atm2Out(ZXComp*,unsigned short,unsigned char,int);
unsigned char atm2In(ZXComp*,unsigned short,int);
void atm2Reset(ZXComp*);

// pentevo
void evoMapMem(ZXComp*);
void evoOut(ZXComp*,unsigned short,unsigned char,int);
unsigned char evoIn(ZXComp*,unsigned short,int);
unsigned char evoMRd(ZXComp*,unsigned short,int);
void evoMWr(ZXComp*,unsigned short,unsigned char);
void evoReset(ZXComp*);

// TSLab conf
void tslMapMem(ZXComp*);
void tslOut(ZXComp*,unsigned short,unsigned char,int);
unsigned char tslIn(ZXComp*,unsigned short,int);
unsigned char tslMRd(ZXComp*,unsigned short,int);
void tslMWr(ZXComp*,unsigned short,unsigned char);
void tslReset(ZXComp*);
void tslUpdatePorts(ZXComp*);

// Profi
void prfMapMem(ZXComp*);
void prfOut(ZXComp*,unsigned short,unsigned char,int);
unsigned char prfIn(ZXComp*,unsigned short,int);
void prfReset(ZXComp*);
unsigned char prfMRd(ZXComp*,unsigned short,int);

#ifdef __cplusplus
}
#endif

#endif
