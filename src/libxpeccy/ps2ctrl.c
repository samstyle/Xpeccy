#include <string.h>
#include <stdlib.h>

#include "ps2ctrl.h"

// PS/2 controller

PS2Ctrl* ps2c_create(Keyboard* kp, Mouse* mp) {
	PS2Ctrl* ctrl = (PS2Ctrl*)malloc(sizeof(PS2Ctrl));
	if (ctrl) {
		memset(ctrl, 0, sizeof(PS2Ctrl));
		ctrl->kbd = kp;
		ctrl->mouse = mp;
		ctrl->cmd = -1;
		ctrl->status = 0;
	}
	return ctrl;
}

void ps2c_destroy(PS2Ctrl* ctrl) {
	free(ctrl);
}

void ps2c_reset(PS2Ctrl* ctrl) {
	ctrl->cmd = -1;
	ctrl->reset = 0;
	ctrl->status &= ~3;
}

// ram[0] = configuration byte
// b0: 1=enable 1st ps/2 device interrupt
// b1: 1=enable 2nd ps/2 device interrupt
// b2: system flag
// b3: 0
// b4: 1=disable 1st ps/2 device clock (disable kbd)
// b5: 1=disable 2nd ps/2 device clock (disable mouse)
// b6: 1=enable 1st ps/2 port translation (?)
// b7: 0

// status byte (rd from 0x64)
// b0: 1=output buffer full (can be readed from 0x60)
// b1: 1=input buffer full (command/data can't be written)
// b2: system flag
// b3: 0=last writting was command (0x64), 1=last writting was data (0x60)
// b4: 0=keyboard blocked
// b5: 1=output buffer contains byte from mouse, 0=from keyboard
// b6: timeout error (outbuf=FF if it was read, FE if writting, from/to device)
// b7: parity error (outbuf=FF)

// controller output port
// b0: 0=reset system
// b1: a20 gate
// b2: 2nd ps/2 device clock
// b3: 2nd ps/2 device data
// b4: 1st device output buffer full (irq 1)
// b5: 2nd device output buffer full (irq 12)
// b6: 1st ps/2 device clock
// b7: 1st ps/2 device data

// controller input port
// b0: kbd data line status
// b1: mouse data line status

int ps2c_rd(PS2Ctrl* ctrl, int adr) {
	int res = -1;
	switch (adr) {
		case PS2_RDATA:
			// res = xt_read(ctrl->kbd);
			if (ctrl->status & 1) {
				res = ctrl->outbuf;
				ctrl->status &= ~1;
			}
			break;
		case PS2_RSTATUS:
			res = ctrl->status;
			break;
	}
	return res;
}

void ps2c_wr_ob(PS2Ctrl* ctrl, int val) {
	ctrl->outbuf = val;
	ctrl->status &= ~0x21;
	ctrl->status |= 0x01;
}

void ps2c_wr(PS2Ctrl* ctrl, int adr, int val) {
	ctrl->inbuf = val;
	switch (adr) {
		case PS2_RDATA:
			ctrl->status |= 8;
			if (ctrl->cmd < 0) break;
			if ((ctrl->cmd & 0xe0) == 0x60) {
				ctrl->ram[ctrl->cmd & 0x1f] = val & 0xff;
				if (ctrl->cmd == 0x60) {
					ctrl->status &= ~4;
					ctrl->status |= (val & 4);
				}
			} else {
				switch (ctrl->cmd) {
					case 0xd1:		// write to output port
						ctrl->outport = val;
						if (!(val & 1)) ctrl->reset = 1;
						break;
					case 0xd2:
						ps2c_wr_ob(ctrl, val);
						break;
					case 0xd3:
						ps2c_wr_ob(ctrl, val);
						ctrl->status |= 0x20;
						break;
					case 0xd4:		// send to mouse
						break;
				}
			}
			ctrl->cmd = -1;
			break;
		case PS2_RCMD:
			ctrl->status &= ~8;
			switch (val & 0xe0) {
				case 0x00: break;			// 00..1F nothing?
				case 0x20:				// 20..3F read byte from internal ram of ps/2 controller
					ps2c_wr_ob(ctrl, ctrl->ram[val & 0x1f]);
					break;
				case 0x40: break;
				case 0x60:
					ctrl->cmd = val;		// 60..7F: address to write through 60h
					break;
				case 0x80: break;
				case 0xa0:
					switch(val) {
						case 0xa1:		// read controller version
							break;
						case 0xa4:
							ps2c_wr_ob(ctrl, 0xf1);		// f1:no password protect, f1:password protect presented
							break;
						case 0xa5:		// write password (all input data bytes till 0x00)
							break;
						case 0xa6:		// password check
							break;
						case 0xa7:		// disable 2nd ps/2 device
							ctrl->status |= 0x20;
							break;
						case 0xa8:		// enable 2nd ps/2 device
							ctrl->status &= ~0x20;
							break;
						case 0xa9:		// test 2nd ps/2 deivce
							ps2c_wr_ob(ctrl, 0);
							break;
						case 0xaa:		// run selftest. write 55h to data port
							ps2c_wr_ob(ctrl, 0x55);
							break;
						case 0xab:		// test 1st ps/2 device
							ps2c_wr_ob(ctrl, 0);
							break;
						case 0xac:		// diagnostic dump (read 32 bytes of internal mem)
							break;
						case 0xad:		// disable kbd
							ctrl->status |= 0x10;
							break;
						case 0xae:		// enable kbd
							ctrl->status &= ~0x10;
							break;
						case 0xaf:		// read controller version (again?)
							break;
					}
					break;
				case 0xc0:
					switch (val) {
						case 0xc0:		// read controller input port
							ps2c_wr_ob(ctrl, ctrl->inbuf);
							break;
						case 0xc1:		// b0..3 input port -> b4..7 status byte
							ctrl->status &= 0x0f;
							ctrl->status |= ((ctrl->inbuf << 4) & 0xf0);
							break;
						case 0xc2:		// b4..7 input port -> b4..7 status byte
							ctrl->status &= 0x0f;
							ctrl->status |= (ctrl->inbuf & 0xf0);
							break;
						case 0xd0:		// read controller output port
							ps2c_wr_ob(ctrl, ctrl->outport);
							break;
						case 0xd1:		// write next byte controller output port
						case 0xd2:		// write next byte to 1st ps/2 output buffer (like it was readed from device)
						case 0xd3:		// write next byte to 2nd ps/2 output buffer
						case 0xd4:		// write next byte to 2nd ps/2 input buffer (send command to mouse)
							ctrl->cmd = val;
							break;
					}
					break;
				case 0xe0:
					if (val == 0xff) {				// FF reset
						ps2c_reset(ctrl);
					} else if ((val & 0xf0) == 0xf0) {		// Fx commands
						if (!(val & 1)) ctrl->reset = 1;	// b0=0: reset computer
					} else {					// Ex commands
						switch (val) {
							case 0xe0:
								// set outbuf:
								// b0=kbd clock line status
								// b1=mouse clock line status
								break;
						}
					}
					break;
			}
			break;
	}
}
