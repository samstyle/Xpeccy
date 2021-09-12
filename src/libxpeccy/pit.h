#pragma once

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
	unsigned out:1;		// output pin
	unsigned lout:1;	// prev.output (!lout && out)->signal
	unsigned gate:1;	// 1 if waiting external signal to start (ch2)
	unsigned half:1;	// what rd/wr next 0:low byte, 1:high byte (in lo-hi mode)
	unsigned wdiv:1;	// wait for div load (high byte in lo-hi mode)
	unsigned wgat:1;	// wait for gate input 0->1
	unsigned acmod:2;	// bit4,5
	unsigned opmod:3;	// bit1-3
	unsigned latch:2;	// bytes of counter latched in clat b1:hi, b0:lo (00 = read cnt instead)
	unsigned short div;	// divider (ticks)
	unsigned short cnt;	// current counter
	unsigned short clat;	// latched counter
	pchCore* cb;
};

typedef struct {
	unsigned bcd:1;
	pitChan ch0;
	pitChan ch1;
	pitChan ch2;
	int ns;			// sync: time couter for ticks. 1T = 838ns (~1.1933MHz)
} PIT;

void pit_reset(PIT*);
int pit_rd(PIT*, int);
void pit_wr(PIT*, int, int);
void pit_sync(PIT*, int);
