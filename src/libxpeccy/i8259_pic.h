#pragma once

#define PIC_COM		0
#define PIC_DATA	1

enum {
	PIC_ICW1 = 0,		// wait icw1 in command
	PIC_ICW2,		// wait icw2 in data
	PIC_ICW3,
	PIC_ICW4,
	PIC_OCWX		// initialization completed, data=imr,com=ocwX
};

// send int only if bit irr=1,imr=0,isr=0

typedef struct {
	unsigned master:1;	// 1 on master pic, 0 on slave
	unsigned oint:1;	// int output
	unsigned smm:1;		// special mask mode
	unsigned srd:1;		// special rd (ocw3 bit1 = 1)
	unsigned sdt:1;		// data for special rd from data port
	unsigned char irr;	// input lines bits (1 = incomming int)
	unsigned char imr;	// mask bits (1 = disabled)
	unsigned char isr;	// on int send set bit in isr, reset in irr. on eoi reset bit in isr
	unsigned char num;	// last int input number
	unsigned char mask;	// mask of interrupt that sends oint (must be power of 2)
	int vec;
	int mode;
	unsigned char ocw2,ocw3;	// ocw1 = ocwx = imr
	unsigned char icw1,icw2,icw3,icw4;
} PIC;

void pic_reset(PIC*);
int pic_int(PIC*, int);
int pic_ack(PIC*);

void pic_wr(PIC*, int a, int d);
int pic_rd(PIC*, int a);
