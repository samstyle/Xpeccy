#pragma once

typedef struct {
	unsigned char data[32];
	int pos;	// =0 on sending command, +1 each writing in fifo
	int cnt;	// count of remaining parameters, countdown
	int queue;	// +1 each writing in fifo, -1 each line(?) when >0; 16 = fifo full, 0 = fifo empty
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

void upd7220_frame(Video*);
void upd7220_line(Video*);
void upd7220_dot(Video*);
