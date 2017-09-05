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
	HW_MSX2,	// MSX 2
	HW_GBC,		// Game boy color (gameboy capatible)
	HW_NES		// Nintendo Entertaiment System (Dendy)
};

enum {
	NES_DENDY = 0,
	NES_NTSC,
	NES_PAL
};

// mem size
#define	MEM_48	0
#define	MEM_128	1
#define	MEM_256 (1<<1)
#define	MEM_512	(1<<2)
#define	MEM_1M	(1<<3)
#define	MEM_2M	(1<<4)
#define	MEM_4M	(1<<5)

// Hardware callbacks

// reset
typedef void(*cbHwRes)(Computer*);
// map memory
typedef void(*cbHwMap)(Computer*);
// sync: calls after every CPU command
typedef void(*cbHwSnc)(Computer*, int);
// memory read
typedef unsigned char(*cbHwMrd)(Computer*, unsigned short, int);
// memory write
typedef void(*cbHwMwr)(Computer*, unsigned short, unsigned char);
// io read; TODO:remove last argument (bdi activity)
typedef unsigned char(*cbHwIrd)(Computer*, unsigned short, int);
// io write
typedef void(*cbHwIwr)(Computer*, unsigned short, unsigned char, int);
// key press/release
typedef void(*cbHwKey)(Computer*, keyEntry);
// get volume
typedef sndPair(*cbHwVol)(Computer*, sndVolume*);

struct HardWare {
	int id;			// id
	const char* name;	// name used for conf file
	const char* optName;	// name used for setup window
	int fps;
	int mask;		// mem size bits (b0:128, b1:256, b2:512, b3:1M, b4:2M, b5:4M); =0 for 48K
	cbHwMap mapMem;
	cbHwIwr out;
	cbHwIrd in;
	cbHwMrd mrd;
	cbHwMwr mwr;
	cbHwRes reset;
	cbHwSnc sync;
	cbHwKey keyp;
	cbHwKey keyr;
	cbHwVol vol;
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

void zx_sync(Computer*, int);
sndPair zx_vol(Computer*, sndVolume*);

// debug IO

unsigned char brkIn(Computer*, unsigned short);
void brkOut(Computer*, unsigned short, unsigned char);

unsigned char dummyIn(Computer*, unsigned short);
void dummyOut(Computer*, unsigned short, unsigned char);

// common IO

int zx_dev_wr(Computer*, unsigned short, unsigned char, int);
int zx_dev_rd(Computer*, unsigned short, unsigned char*, int);

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
void zx_keyp(Computer*, keyEntry);
void zx_keyr(Computer*, keyEntry);

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

// Evo baseconf
void evoMapMem(Computer*);
void evoOut(Computer*,unsigned short,unsigned char,int);
unsigned char evoIn(Computer*,unsigned short,int);
unsigned char evoMRd(Computer*,unsigned short,int);
void evoMWr(Computer*,unsigned short,unsigned char);
void evoReset(Computer*);

// Evo tsconf
void tslMapMem(Computer*);
void tslOut(Computer*,unsigned short,unsigned char,int);
unsigned char tslIn(Computer*,unsigned short,int);
unsigned char tslMRd(Computer*,unsigned short,int);
void tslMWr(Computer*,unsigned short,unsigned char);
void tslReset(Computer*);

// Profi
void prfMapMem(Computer*);
void prfOut(Computer*,unsigned short,unsigned char,int);
unsigned char prfIn(Computer*,unsigned short,int);
void prfReset(Computer*);
unsigned char prfMRd(Computer*,unsigned short,int);
void prf_keyp(Computer*, keyEntry);
void prf_keyr(Computer*, keyEntry);

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
void msx_sync(Computer*, int);
void msx_keyp(Computer*, keyEntry);
void msx_keyr(Computer*, keyEntry);
sndPair msx_vol(Computer*, sndVolume*);

// msx2
void msx2mapper(Computer*);
void msx2Out(Computer*,unsigned short,unsigned char,int);
unsigned char msx2In(Computer*,unsigned short,int);
void msx2Reset(Computer*);
unsigned char msx2mrd(Computer*,unsigned short,int);
void msx2mwr(Computer*,unsigned short,unsigned char);

// gameboy
void gbMaper(Computer*);
void gbReset(Computer*);
unsigned char gbMemRd(Computer*, unsigned short, int);
void gbMemWr(Computer*, unsigned short, unsigned char);
void gbcSync(Computer*, int);
void gbc_keyp(Computer*, keyEntry);
void gbc_keyr(Computer*, keyEntry);
sndPair gbc_vol(Computer*, sndVolume*);

// nes
void nesMaper(Computer*);
void nesReset(Computer*);
unsigned char nesMemRd(Computer*, unsigned short, int);
void nesMemWr(Computer*, unsigned short, unsigned char);
void nesSync(Computer*, int);
void nes_keyp(Computer*, keyEntry);
void nes_keyr(Computer*, keyEntry);
sndPair nes_vol(Computer*, sndVolume*);
unsigned char nes_apu_ext_rd(unsigned short, void*);
unsigned char nes_ppu_ext_rd(unsigned short, void*);
unsigned char nes_apu_ext_wr(unsigned short, unsigned char, void*);

#ifdef __cplusplus
}
#endif

#endif
