#ifndef _Z80_H
#define _Z80_H

struct CPU;

struct opCode {
	unsigned prefix:1;
	int t;			// T-states
//	int len;		// opcode len
//	int c1,c2,c3,c4;	// opcode bytes
	void(*exec)(struct CPU *);
	struct opCode *tab;
	const char* mnem;
};

typedef struct opCode opCode;

#define FS	0x80
#define	FZ	0x40
#define	F5	0x20
#define	FH	0x10
#define	F3	0x08
#define	FP	0x04
#define	FV	FP
#define	FN	0x02
#define	FC	0x01

#ifdef WORDS_BIG_ENDIAN
	#define PAIR(p,h,l) union{unsigned short p; struct {unsigned char h; unsigned char l;};}
#else
	#define PAIR(p,h,l) union{unsigned short p; struct {unsigned char l; unsigned char h;};}
#endif

typedef struct CPU CPU;

typedef unsigned char(*cbmr)(unsigned short,int,void*);
typedef void(*cbmw)(unsigned short,unsigned char,void*);
typedef unsigned char(*cbir)(unsigned short,void*);
typedef void(*cbiw)(unsigned short,unsigned char,void*);
typedef unsigned char(*cbirq)(void*);
typedef unsigned char(*cbdmr)(unsigned short,void*);

struct CPU {
	unsigned halt:1;
	unsigned resPV:1;
	unsigned noint:1;

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

	opCode* opTab;
	opCode* op;

	int t;
	unsigned char tmp;
	unsigned char tmpb;
	PAIR(tmpw,htw,ltw);
	int tmpi;
};

CPU* cpuCreate(cbmr,cbmw,cbir,cbiw,cbirq,void*);
void cpuDestroy(CPU*);

void cpuReset(CPU*);

int cpuINT(CPU*);
int cpuNMI(CPU*);

int cpuExec(CPU*);

int cpuDisasm(unsigned short,char*,cbdmr,void*);
int cpuAsm(const char*, char*, unsigned short);

#endif
