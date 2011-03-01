#ifndef _Z80_H
#define _Z80_H

#ifndef WIN32
	#include "endian.h"
#else
	#define __BIG_ENDIAN 0
	#define __LITTLE_ENDIAN 1
	#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#define FS 0x80
#define FZ 0x40
#define F5 0x20
#define FH 0x10
#define F3 0x08
#define FP 0x04
#define FN 0x02
#define FC 0x01

//struct Z80;

struct Z80 {
	Z80(float);
#if __BYTE_ORDER == __LITTLE_ENDIAN
	union {unsigned short bc; struct{unsigned char c,b;};};
	union {unsigned short de; struct{unsigned char e,d;};};
	union {unsigned short hl; struct{unsigned char l,h;};};
	union {unsigned short af; struct{unsigned char f,a;};};
	struct {
		union {unsigned short bc; struct{unsigned char c,b;};};
		union {unsigned short de; struct{unsigned char e,d;};};
		union {unsigned short hl; struct{unsigned char l,h;};};
		union {unsigned short af; struct{unsigned char f,a;};};
	} alt;
	union {unsigned short pc; struct {unsigned char lpc,hpc;};};
	union {unsigned short sp; struct {unsigned char lsp,hsp;};};
	union {unsigned short ix; struct {unsigned char lx,hx;};};
	union {unsigned short iy; struct {unsigned char ly,hy;};};
	union {unsigned short ir; struct {unsigned char r,i;};};
	union {unsigned short mptr; struct {unsigned char lptr,hptr;};};
	union {unsigned short adr; struct {unsigned char ladr,hadr;};};
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
	union {unsigned short bc; struct{unsigned char b,c;};};
	union {unsigned short de; struct{unsigned char d,e;};};
	union {unsigned short hl; struct{unsigned char h,l;};};
	union {unsigned short af; struct{unsigned char a,f;};};
	struct {
		union {unsigned short bc; struct{unsigned char b,c;};};
		union {unsigned short de; struct{unsigned char d,e;};};
		union {unsigned short hl; struct{unsigned char h,l;};};
		union {unsigned short af; struct{unsigned char a,f;};};
	} alt;
	union {unsigned short pc; struct {unsigned char hpc,lpc;};};
	union {unsigned short sp; struct {unsigned char hsp,lsp;};};
	union {unsigned short ix; struct {unsigned char hx,lx;};};
	union {unsigned short iy; struct {unsigned char hy,ly;};};
	union {unsigned short ir; struct {unsigned char i,r;};};
	union {unsigned short mptr; struct {unsigned char hptr,lptr;};};
	union {unsigned short adr; struct {unsigned char hadr,ladr;};};
#endif
	float frq;			// in MHz
	unsigned char imode;
	bool iff1,iff2,nextei,onhalt;
	bool block,err;
	unsigned int t,tb,ti;			// счетчики тиков - всего, на начало тек.кадра, на начало M1
	unsigned char speed;			// биты - распределение тиков на проц по ходу отрисовки байта (def: 0x55)
	unsigned char mod,cod,dlt;		// mode (prefix) & current operation code & delta for CB[X|Y] coms
	unsigned char x;			// temp
	void reset();
	int interrupt();
//	void exec();
//	void sync(int);
//	ZOp* inst[9];
};

// extern Z80 *cpu;

#endif
