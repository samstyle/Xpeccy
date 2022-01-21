#pragma once

#include "input.h"

#define PS2_RDATA	0
#define PS2_RCMD	4
#define PS2_RSTATUS	PS2_RCMD

typedef struct {
	unsigned reset:1;	// system reset requested
	unsigned intk:1;	// dev1(kbd) interrupt
	unsigned intm:1;	// dev2(mouse) interrupt
	Keyboard* kbd;
	Mouse* mouse;
	unsigned char ram[0x20];
	int cmd;	// last command for data port writing
	int data;
	int status;
	int inbuf;		// controller input buffer (cpu wr)
	unsigned int outbuf;	// controller output buffer (cpu rd)
	int inport;		// controller input port
	int outport;		// controller output port
} PS2Ctrl;

PS2Ctrl* ps2c_create(Keyboard*, Mouse*);
void ps2c_destroy(PS2Ctrl*);

void ps2c_reset(PS2Ctrl*);
void ps2c_clear(PS2Ctrl*);
int ps2c_rd(PS2Ctrl*, int);
void ps2c_rd_kbd(PS2Ctrl*);
void ps2c_wr(PS2Ctrl*, int, int);
void ps2c_wr_ob(PS2Ctrl*, int);
