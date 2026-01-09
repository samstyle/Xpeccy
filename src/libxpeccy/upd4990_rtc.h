#pragma once

// uPD4990: RTC

#include "defines.h"

typedef struct {
	unsigned data:1;	// data out (LSB of shift register)
	unsigned clk:1;		// 1->0 write data in + shift register (if rh=0)
	unsigned stb:1;		// 1->0 execute command (com or coma)
	unsigned rh:1;		// register hold: don't shift on clk
	unsigned stp:1;		// timer stopped
	unsigned test:1;	// test mode
	unsigned com:3;		// command c0,c1,c2
	unsigned coma:4;	// c0',c1',c2',c3' - if com=111
	long long reg;		// shift register
	// 1tick = 4096Hz (244140 ns)
	int time;
	int tp_per;		// min 4096 Hz(1t), max 60sec (245760t)
	int tp_cnt;		// ticks countdown (if not stopped). when 0, send TP signal

	cbirq xirq;
	void* xptr;
} upd4990;

upd4990* upd4990_create(cbirq, void*);
void upd4990_destroy(upd4990*);
void upd4990_reset(upd4990*);
void upd4990_wr(upd4990*, int);
void upd4990_sync(upd4990*, int);
