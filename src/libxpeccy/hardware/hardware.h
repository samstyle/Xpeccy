#ifndef _LIBXPECCY_HARDWARE
#define _LIBXPECCY_HARDWARE

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

// zx48
void speMapMem(ZXComp*);
void speOut(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int);
Z80EX_BYTE speIn(ZXComp*,Z80EX_WORD,int);
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
// pentevo
void evoMapMem(ZXComp*);
void evoOut(ZXComp*,Z80EX_WORD,Z80EX_BYTE,int);
Z80EX_BYTE evoIn(ZXComp*,Z80EX_WORD,int);

#endif
