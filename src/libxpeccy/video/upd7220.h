#pragma once

typedef struct {
	unsigned char data[32];
	int pos;
	int cnt;
} x7220buf;

typedef struct {
	unsigned off:1;		// don't display
	unsigned master:1;	// is master
	x7220buf inbuf;		// command [0]=com, [1+]=params
	x7220buf outbuf;	// answer
	unsigned char par[16];
	int ead;
	unsigned dpos:4;
} upd7220;

upd7220* upd7220_create();
void upd7220_destroy(upd7220*);

#include "video.h"

int upd7220_rd(Video*, upd7220*, int);
void upd7220_wr(Video*, upd7220*, int, int);
