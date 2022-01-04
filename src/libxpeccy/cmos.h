#pragma once

#include "input.h"

enum {
	CMOS_ADR = 0,
	CMOS_DATA
};

#define CMOS_NMI 1

typedef struct {
	unsigned char adr;
	int mode;
	unsigned char data[256];
	int inten;
	int intrq;
//	xKeyBuf* kbuf;		// pointer to pc keyboard buffer
} CMOS;

unsigned char cmos_rd(CMOS*, int);
void cmos_wr(CMOS*, int, int);
