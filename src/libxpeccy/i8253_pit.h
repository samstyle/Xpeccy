#pragma once

#include "defines.h"

typedef struct pitChan pitChan;

typedef void(*pccbv)(pitChan*);
typedef void(*pccbi)(pitChan*, int);

// different actions for different operating modes
typedef struct {
	pccbv wr;	// after write data
	pccbv tm;	// on cnt=0
	pccbi gt;	// set gate
} pchCore;

struct pitChan {
	unsigned bcd:1;
	unsigned out:1;		// output pin
	unsigned lout:1;	// prev.output (!lout && out)->signal
	unsigned gate:1;	// 1 if waiting external signal to start (ch2)
	unsigned half:1;	// what rd/wr next 0:low byte, 1:high byte (in lo-hi mode)
	unsigned wdiv:1;	// wait for div load (high byte in lo-hi mode)
	unsigned wgat:1;	// wait for gate input 0->1
	unsigned acmod:2;	// bit4,5
	unsigned opmod:3;	// bit1-3
	unsigned latch:2;	// bytes of counter latched in clat (0=read cnt, 1=state, 2=state+[hi|lo]byte, 3=state+word
	unsigned char state;	// mode setted through port 43
	unsigned short div;	// divider (ticks)
	unsigned short cnt;	// current counter
	int clat;		// latched state/counter
	int wav;
	pchCore* cb;
	int resp;
	int resp_cnt;
};

typedef struct {
	pitChan ch0;
	pitChan ch1;
	pitChan ch2;
	int ns;			// sync: time couter for ticks. 1T = 838ns (~1.1933MHz)
	cbirq xirq;
	void* xptr;
} PIT;

PIT* pit_create(cbirq, void*);
void pit_destroy(PIT*);

void pit_reset(PIT*);
int pit_rd(PIT*, int);
void pit_wr(PIT*, int, int);
void pit_gate(PIT*, int, int);
void pit_sync(PIT*, int);
