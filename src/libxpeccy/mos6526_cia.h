#pragma once

#include "defines.h"

#define CIA_IRQ_TIMA	(1<<0)
#define CIA_IRQ_TIMB	(1<<1)
#define CIA_IRQ_ALARM	(1<<2)
#define CIA_IRQ_SP	(1<<3)
#define CIA_IRQ_FLAG	(1<<4)

#define CIA_CR_START	(1<<0)	// start timer
#define CIA_CR_PBXON	(1<<1)	// use reg B bits
#define CIA_CR_TOGGLE	(1<<2)	// 0:pulse, 1:toggle
#define CIA_CR_ONESHOT	(1<<3)	// 0:continuous, 1:oneshot
#define CIA_CR_RELOAD	(1<<4)	// 0:no effect, 1:load latch on overflow

typedef struct {
	int ns;
	unsigned char tenth;	// 1/10 sec (each 1e8 ns)
	unsigned char sec;
	unsigned char min;
	unsigned char hour;
} ciaTime;

typedef struct {
	unsigned overflow:1;
	unsigned char flags;		// cia E/F registers
	PAIR(inival,inih,inil);		// initial value
	PAIR(value,valh,vall);		// countdown
} ciaTimer;

typedef struct {
	unsigned char portA_mask;
	unsigned char portB_mask;
	ciaTimer timerA;
	ciaTimer timerB;
	ciaTime time;
	ciaTime alarm;
	int ns;
	unsigned char ssr;		// serial shift register
	unsigned char intrq;		// reg D - interrupt state
	unsigned char inten;		// reg D - interrupt mask
	unsigned char reg[16];

	cbxrd pard;	// external port a rd/wr
	cbxwr pawr;
	cbxrd pbrd;	// external port b rd/wr
	cbxwr pbwr;
	cbirq xirq;	// send signal outside
	void* xptr;
	int xirqn;	// irq id (IRQ_CIA1, IRQ_CIA2)
} CIA;

CIA* cia_create(int, cbirq, void*);
void cia_destroy(CIA*);
void cia_set_port(CIA*, int, cbxrd, cbxwr);
void cia_sync(CIA*, int, int);
void cia_irq(CIA*, int);
int c64_cia_rd(CIA*, int);
void c64_cia_wr(CIA*, int, int);
