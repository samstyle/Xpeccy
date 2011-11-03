#ifndef _Z80_H
#define _Z80_H

#include <stdint.h>

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
#ifdef BO_LITTLEENDIAN
	union {uint16_t bc; struct{uint8_t c,b;};};
	union {uint16_t de; struct{uint8_t e,d;};};
	union {uint16_t hl; struct{uint8_t l,h;};};
	union {uint16_t af; struct{uint8_t f,a;};};
	struct {
		union {uint16_t bc; struct{uint8_t c,b;};};
		union {uint16_t de; struct{uint8_t e,d;};};
		union {uint16_t hl; struct{uint8_t l,h;};};
		union {uint16_t af; struct{uint8_t f,a;};};
	} alt;
	union {uint16_t pc; struct {uint8_t lpc,hpc;};};
	union {uint16_t sp; struct {uint8_t lsp,hsp;};};
	union {uint16_t ix; struct {uint8_t lx,hx;};};
	union {uint16_t iy; struct {uint8_t ly,hy;};};
	union {uint16_t ir; struct {uint8_t r,i;};};
	union {uint16_t mptr; struct {uint8_t lptr,hptr;};};
	union {uint16_t adr; struct {uint8_t ladr,hadr;};};
#endif
#ifdef BO_BIGENDIAN
	union {uint16_t bc; struct{uint8_t b,c;};};
	union {uint16_t de; struct{uint8_t d,e;};};
	union {uint16_t hl; struct{uint8_t h,l;};};
	union {uint16_t af; struct{uint8_t a,f;};};
	struct {
		union {uint16_t bc; struct{uint8_t b,c;};};
		union {uint16_t de; struct{uint8_t d,e;};};
		union {uint16_t hl; struct{uint8_t h,l;};};
		union {uint16_t af; struct{uint8_t a,f;};};
	} alt;
	union {uint16_t pc; struct {uint8_t hpc,lpc;};};
	union {uint16_t sp; struct {uint8_t hsp,lsp;};};
	union {uint16_t ix; struct {uint8_t hx,lx;};};
	union {uint16_t iy; struct {uint8_t hy,ly;};};
	union {uint16_t ir; struct {uint8_t i,r;};};
	union {uint16_t mptr; struct {uint8_t hptr,lptr;};};
	union {uint16_t adr; struct {uint8_t hadr,ladr;};};
#endif
	float frq;			// in MHz
	uint8_t imode;
	bool iff1,iff2,nextei,onhalt;
	bool block,err;
	bool halt;			// true on HALT instruction execute
	uint32_t t,tb,ti;		// счетчики тиков - всего, на начало тек.кадра, на начало M1
	uint8_t speed;			// биты - распределение тиков на проц по ходу отрисовки байта (def: 0x55)
	uint8_t mod,cod,dlt;		// mode (prefix) & current operation code & delta for CB[X|Y] coms
	uint8_t x;			// temp
	void reset();
	int interrupt();
};

// extern Z80 *cpu;

#endif
