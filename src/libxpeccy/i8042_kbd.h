#pragma once

#include "defines.h"
#include "input.h"

#define PS2_RDATA	0
#define PS2_RCMD	4
#define PS2_RSTATUS	PS2_RCMD

// data port mode
enum {
	PS2C_MODE_KBD = 0,	// default, send data to keyboard
	PS2C_MODE_MOU,		// send data to mouse
	PS2C_MODE_COM		// data is argument for controller commands
};

typedef struct {
	unsigned m_rdy:1;		// mouse have data to read
	unsigned k_rdy:1;		// keyboard have data to read (future feature)
	int dmode;

	Keyboard* kbd;
	Mouse* mouse;

	cbirq xirq;
	void* xptr;

	unsigned char ram[0x20];
	int delay;
	int cmd;	// last command for data port writing
	int data;
	int status;
	int inbuf;		// controller input buffer (cpu wr)
	unsigned int outbuf;	// controller output buffer (cpu rd)
	int inport;		// controller input port
	int outport;		// controller output port
} PS2Ctrl;

PS2Ctrl* ps2c_create(Keyboard*, Mouse*, cbirq, void*);
void ps2c_destroy(PS2Ctrl*);
void ps2c_sync(PS2Ctrl*, int);
void ps2c_reset(PS2Ctrl*);
void ps2c_clear(PS2Ctrl*);
int ps2c_rd(PS2Ctrl*, int);
void ps2c_wr(PS2Ctrl*, int, int);
void ps2c_ready(PS2Ctrl*, int);
