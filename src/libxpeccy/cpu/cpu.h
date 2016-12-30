#ifndef _CPU_H
#define _CPU_H

extern const unsigned char daaTab[0x1000];
extern const unsigned char sz53pTab[0x100];
extern const unsigned char FHaddTab[8];
extern const unsigned char FHsubTab[8];
extern const unsigned char FVaddTab[8];
extern const unsigned char FVsubTab[8];

typedef struct {
	int fetch;
	const char* mnem;
} xMnem;

typedef unsigned char(*cbmr)(unsigned short,int,void*);
typedef void(*cbmw)(unsigned short,unsigned char,void*);
typedef unsigned char(*cbir)(unsigned short,void*);
typedef void(*cbiw)(unsigned short,unsigned char,void*);
typedef unsigned char(*cbirq)(void*);
typedef unsigned char(*cbdmr)(unsigned short,void*);

#ifdef WORDS_BIG_ENDIAN
	#define PAIR(p,h,l) union{unsigned short p; struct {unsigned char h; unsigned char l;};}
#else
	#define PAIR(p,h,l) union{unsigned short p; struct {unsigned char l; unsigned char h;};}
#endif

struct CPU;
struct opCode {
	unsigned prefix:1;
	int t;				// T-states
	void(*exec)(struct CPU *);	// fuction to exec
	struct opCode *tab;		// next opCode tab (for prefixes)
	const char* mnem;		// mnemonic
};
typedef struct opCode opCode;

typedef struct {
	unsigned match:1;
	int idx;
	opCode* op;
	char* ptr;
	char arg[8][256];
} xAsmScan;

#include "macros.h"
#include "Z80/z80.h"
#include "LR35902/lr35902.h"

enum {
	CPU_NONE = 0,		// dummy
	CPU_Z80,		// ZX, MSX
	CPU_LR35902		// GB, GBC
};

typedef struct CPU CPU;

struct CPU {
	unsigned halt:1;
	unsigned resPV:1;
	unsigned noint:1;
	unsigned lock:1;		// LR35902: CPU locked

	int type;

	PAIR(pc,hpc,lpc);
	PAIR(sp,hsp,lsp);
	PAIR(ix,hx,lx);
	PAIR(iy,hy,ly);
	PAIR(mptr,hptr,lptr);
	unsigned char i;
	unsigned char r;
	unsigned char r7;
	unsigned char iff1;
	unsigned char iff2;
	unsigned char imode;

	PAIR(af,a,f);
	PAIR(bc,b,c);
	PAIR(de,d,e);
	PAIR(hl,h,l);

	PAIR(af_,a_,f_);
	PAIR(bc_,b_,c_);
	PAIR(de_,d_,e_);
	PAIR(hl_,h_,l_);

	cbmr mrd;
	cbmw mwr;
	cbir ird;
	cbiw iwr;
	cbirq irq;
	void* data;

	opCode* tab;
	opCode* opTab;
	opCode* op;

	void (*reset)(CPU*);
	int (*exec)(CPU*);
	int (*intr)(CPU*);
	int (*nmi)(CPU*);
	xAsmScan (*asmbl)(const char*, char*);
	xMnem (*mnem)(unsigned short, cbdmr, void*);

	int t;
	unsigned char tmp;
	unsigned char tmpb;
	PAIR(tmpw,htw,ltw);
	int tmpi;
};

typedef struct {
	int type;				// cpu type
	const char* name;			// printable name
	opCode* tab;				// start opcode tab;
	void (*reset)(CPU*);			// reset
	int (*exec)(CPU*);			// exec opcode, return T
	int (*intr)(CPU*);			// handle INT, return T
	int (*nmi)(CPU*);			// handle NMI, return T
	xAsmScan (*asmbl)(const char*, char*);	// compile mnemonic
	xMnem (*mnem)(unsigned short, cbdmr, void*);
} cpuCore;

CPU* cpuCreate(int,cbmr,cbmw,cbir,cbiw,cbirq,void*);
void cpuDestroy(CPU*);
void cpuSetType(CPU*, int);

int getCoreID(const char*);
const char* getCoreName(int);

extern cpuCore cpuTab[];

int cpuDisasm(CPU*, unsigned short, char*, cbdmr, void*);
int cpuAsm(CPU*, const char*, char*, unsigned short);
xAsmScan scanAsmTab(const char*, opCode*);

#endif
