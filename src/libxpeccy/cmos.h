#pragma once

#include "input.h"

enum {
	CMOS_ADR = 0,
	CMOS_DATA
};

typedef struct {
	unsigned char adr;
	int mode;
	unsigned char data[256];
	xKeyBuf* kbuf;		// pointer to pc keyboard buffer
} CMOS;

unsigned char cmos_rd(CMOS*, int);
void cmos_wr(CMOS*, int, int);
