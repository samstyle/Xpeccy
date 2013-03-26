#ifndef _Z80_H
#define _Z80_H

struct Z80CPU;

struct opCode {
	int flag;
	int t;
	int t1;
	int t2;
	int t3;
	int t4;
	void(*exec)(struct Z80CPU *);
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

typedef unsigned char(*cbmr)(unsigned short,int,void*);
typedef void(*cbmw)(unsigned short,unsigned char,void*);
typedef unsigned char(*cbir)(unsigned short,void*);
typedef void(*cbiw)(unsigned short,unsigned char,void*);
typedef unsigned char(*cbirq)(void*);
typedef unsigned char(*cbdmr)(unsigned short,void*);

struct Z80CPU {
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

	int halt;
	int resPV;
	int noint;

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

typedef struct Z80CPU Z80CPU;

Z80CPU* cpuCreate(cbmr,cbmw,cbir,cbiw,cbirq,void*);
void cpuDestroy(Z80CPU*);

void cpuReset(Z80CPU*);

int cpuINT(Z80CPU*);
int cpuNMI(Z80CPU*);

int cpuExec(Z80CPU*);

int cpuDisasm(unsigned short,char*,cbdmr,void*);

#endif
