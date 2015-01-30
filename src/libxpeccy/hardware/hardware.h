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
	void (*out)(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int);
	Z80EX_BYTE (*in)(ZXComp*,Z80EX_WORD,int);
	Z80EX_BYTE (*mrd)(ZXComp*,Z80EX_WORD,int);
	void (*mwr)(ZXComp*,Z80EX_WORD,Z80EX_BYTE);
	void (*reset)(ZXComp*);
};

typedef struct {
	int mask;		// if (port & mask == value & mask) port is catched
	int value;
	unsigned all:1;		// 1 if dos-independed
	unsigned dos:1;		// if all=0: is port dos-only
	Z80EX_BYTE (*in)(ZXComp*, Z80EX_WORD);
	void (*out)(ZXComp*, Z80EX_WORD, Z80EX_BYTE);
} xPort;

Z80EX_BYTE hwIn(xPort*, ZXComp*, Z80EX_WORD, int);
void hwOut(xPort*, ZXComp*, Z80EX_WORD, Z80EX_BYTE, int);

typedef struct HardWare HardWare;
extern HardWare hwTab[];

HardWare* findHardware(const char*);
Z80EX_BYTE stdMRd(ZXComp*,Z80EX_WORD,int);
void stdMWr(ZXComp*,Z80EX_WORD,Z80EX_BYTE);

// common IO

void xOutBFFD(ZXComp*, Z80EX_WORD, Z80EX_BYTE);
void xOutFFFD(ZXComp*, Z80EX_WORD, Z80EX_BYTE);

Z80EX_BYTE xInFE(ZXComp*, Z80EX_WORD);
Z80EX_BYTE xInFFFD(ZXComp*, Z80EX_WORD);
Z80EX_BYTE xInFADF(ZXComp*, Z80EX_WORD);
Z80EX_BYTE xInFBDF(ZXComp*, Z80EX_WORD);
Z80EX_BYTE xInFFDF(ZXComp*, Z80EX_WORD);

// zx48
void speMapMem(ZXComp*);
void speOut(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int);
Z80EX_BYTE speIn(ZXComp*,Z80EX_WORD,int);
void speReset(ZXComp*);

// pentagon
void penMapMem(ZXComp*);
void penOut(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int);
Z80EX_BYTE penIn(ZXComp*,Z80EX_WORD,int);

// p1024sl
void p1mMapMem(ZXComp*);
void p1mOut(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int);
Z80EX_BYTE p1mIn(ZXComp*,Z80EX_WORD,int);

// scorpion
void scoMapMem(ZXComp*);
void scoOut(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int);
Z80EX_BYTE scoIn(ZXComp*,Z80EX_WORD,int);
Z80EX_BYTE scoMRd(ZXComp*,Z80EX_WORD,int);

// plus 2
void pl2MapMem(ZXComp*);
void pl2Out(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int);
Z80EX_BYTE pl2In(ZXComp*,Z80EX_WORD,int);

// plus 3
// void pl3MapMem(ZXComp*);		// = pl2MapMem
void pl3Out(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int);
Z80EX_BYTE pl3In(ZXComp*,Z80EX_WORD,int);

// atm 2
void atm2MapMem(ZXComp*);
void atm2Out(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int);
Z80EX_BYTE atm2In(ZXComp*,Z80EX_WORD,int);
void atm2Reset(ZXComp*);

// pentevo
void evoMapMem(ZXComp*);
void evoOut(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int);
Z80EX_BYTE evoIn(ZXComp*,Z80EX_WORD,int);
Z80EX_BYTE evoMRd(ZXComp*,Z80EX_WORD,int);
void evoMWr(ZXComp*,Z80EX_WORD,Z80EX_BYTE);
void evoReset(ZXComp*);

// TSLab conf
void tslMapMem(ZXComp*);
void tslOut(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int);
Z80EX_BYTE tslIn(ZXComp*,Z80EX_WORD,int);
Z80EX_BYTE tslMRd(ZXComp*,Z80EX_WORD,int);
void tslMWr(ZXComp*,Z80EX_WORD,Z80EX_BYTE);
void tslReset(ZXComp*);
void tslUpdatePorts(ZXComp*);
#endif

#ifdef __cplusplus
}
#endif
