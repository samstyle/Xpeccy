#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "i8042_kbd.h"

#define KBD_DELAY 1e6

// PS/2 controller

PS2Ctrl* ps2c_create(Keyboard* kp, Mouse* mp, cbirq cb, void* p) {
	PS2Ctrl* ctrl = (PS2Ctrl*)malloc(sizeof(PS2Ctrl));
	if (ctrl) {
		memset(ctrl, 0, sizeof(PS2Ctrl));
		ctrl->kbd = kp;
		ctrl->mouse = mp;
		ctrl->xirq = cb;
		ctrl->xptr = p;
		ctrl->cmd = -1;
		ctrl->status = 0;
		ctrl->delay = 0;
	}
	return ctrl;
}

void ps2c_destroy(PS2Ctrl* ctrl) {
	free(ctrl);
}

void ps2c_reset(PS2Ctrl* ctrl) {
	ctrl->cmd = -1;
	ctrl->reset = 0;
	ctrl->outbuf = 0;
	ctrl->outport = 2;
	ctrl->status &= ~0x03;
	ctrl->ram[0] = 0x00;
	ctrl->delay = 0;
}

void ps2c_clear(PS2Ctrl* ctrl) {
	ctrl->outbuf = 0;
	ctrl->status &= ~0x03;
}

// ram[0] = configuration byte
// b7: reserved
// b6: convert scan codes
// b5: ? 1-translate code to xt (tab 1), 0-to at (tab2)
// b4: disable kbd
// b3: 1-override inhibit keyswitch
// b2: system flag (1-warm reboot)
// b1: reserved
// b0: enable keyboard irq1

// status byte (rd from 0x64)
// b0: 1=output buffer full (can be readed from 0x60)
// b1: 1=input buffer full (command/data can't be written)
// b2: system flag
// b3: 0=last writting was command (0x64), 1=last writting was data (0x60)
// b4: 0=keyboard lock switch (0-locked)
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
// b7: kbd enabled

// DONE: ~1ms delay between data bytes from device
// DONE?: xt/at keyboard autorepeat

int ps2c_rd(PS2Ctrl* ctrl, int adr) {
	int res = -1;
	switch (adr) {
		case PS2_RDATA:
			res = ctrl->outbuf & 0xff;
			ctrl->outbuf >>= 8;
			//printf("i8042 read code %.2X\n",res);
			break;
		case PS2_RSTATUS:
			res = ctrl->status | 0x10;		// b4 = keyboard lock off
			ctrl->status &= ~2;
			res &= ~0x01;				// set b0[,1] manually
			if (ctrl->outbuf & 0xff)
				res |= 0x01;
			break;
	}
	return res;
}

void ps2c_wr_ob(PS2Ctrl* ctrl, int val) {
	ctrl->outbuf = val;
	ctrl->status &= ~0x21;
	ctrl->status |= 0x01;
}

void ps2c_wr_ob2(PS2Ctrl* ctrl, int val) {
	ctrl->outbuf = val;
	ctrl->status |= 0x21;
}

// read 1 byte from kbd to outbuf and generate intk if need
void ps2c_rd_kbd(PS2Ctrl* ctrl) {
	if (ctrl->kbd->outbuf & 0xff) {
		if (!ctrl->kbd->lock && !(ctrl->ram[0] & 0x10)) {
			ps2c_wr_ob(ctrl, xt_read(ctrl->kbd));
		}
//		ctrl->kbd->outbuf >>= 8;
//		if (ctrl->ram[0] & 1) {
			ctrl->xirq(IRQ_KBD, ctrl->xptr);
//		}
		ctrl->delay = KBD_DELAY;
		//printf("i8042 get scancode %X (remains %X)\n", ctrl->outbuf,ctrl->kbd->outbuf);
	} else {
		ctrl->outbuf = 0;
	}
}

void ps2c_rd_mouse(PS2Ctrl* ctrl) {
	if (ctrl->ram[0] & 0x20) return;	// 2nd device disabled
	ps2c_wr_ob2(ctrl, ctrl->mouse->outbuf);
	ctrl->mouse->outbuf = 0;
	if ((ctrl->ram[0] & 2) && (ctrl->outbuf & 0xff)) {
		ctrl->xirq(IRQ_MOUSE, ctrl->xptr);
		ctrl->delay = KBD_DELAY;
	}
}

void ps2c_wr(PS2Ctrl* ctrl, int adr, int val) {
	ctrl->inbuf = val;
	ctrl->status |= 2;
	switch (adr) {
		case PS2_RDATA:
			//printf("PS/2 controller wr data %.2X\n",val);
			ctrl->status |= 8;
			if (ctrl->cmd < 0) {
				// send byte(s) to keyboard
				// printf("i8042 kbd wr %.2X\n",val);
				switch(val) {
					case 0xed: ctrl->cmd = val | 0x100; break;	// leds
					case 0xee: ps2c_wr_ob(ctrl, 0xee); break;	// echo
					case 0xf0: ctrl->cmd = val | 0x100; break;	// get/set scancode
					case 0xf2: ps2c_wr_ob(ctrl, 0xfa); break;	// get dev type (no code = at-keyboard)
					case 0xf3: ctrl->cmd = val | 0x100; break;	// set repeat rate/delay
					case 0xf4:					// enable sending scancodes
						ctrl->kbd->lock = 0;
						ps2c_wr_ob(ctrl, 0xfa);
						break;
					case 0xf5:					// disable sending scancodes
						ctrl->kbd->lock = 1;
						ps2c_wr_ob(ctrl, 0xfa);
						break;
					case 0xf6: ps2c_wr_ob(ctrl, 0xfa); break;	// set default params
					case 0xf7: ps2c_wr_ob(ctrl, 0xfa); break;	// f7..fd: scanset3 specific
					case 0xf8: ps2c_wr_ob(ctrl, 0xfa); break;
					case 0xf9: ps2c_wr_ob(ctrl, 0xfa); break;
					case 0xfa: ps2c_wr_ob(ctrl, 0xfa); break;
					case 0xfb: ps2c_wr_ob(ctrl, 0xfa); break;
					case 0xfc: ps2c_wr_ob(ctrl, 0xfa); break;
					case 0xfd: ps2c_wr_ob(ctrl, 0xfa); break;
					case 0xfe: ps2c_wr_ob(ctrl, ctrl->data & 0xff); break;	// resend last byte
					case 0xff: ps2c_reset(ctrl);			// reset & run selftest
						ps2c_wr_ob(ctrl, 0xaa);
						break;
				}
			} else if ((ctrl->cmd & 0xfe0) == 0x60) {
				if (ctrl->cmd == 0x60) {
					ctrl->status &= ~4;
					ctrl->status |= (val & 4);
					val &= ~0x40;
				}
				ctrl->ram[ctrl->cmd & 0x1f] = val & 0xff;
			} else {
				// printf("i8042 arg wr %.2X\n",val);
				switch (ctrl->cmd) {
					case 0xd1:		// write to output port
						ctrl->outport = val;
						// printf("i8042 outport = %.2X\n",val);
						if (!(val & 1)) ctrl->reset = 1;
						// b1:a20 gate. if 0 here [or port 92 bit 1 = 0], a20+ is disabled (always 0)
						break;
					case 0xd2:
						ps2c_wr_ob(ctrl, val);
						break;
					case 0xd3:
						ps2c_wr_ob2(ctrl, val);
						break;
					case 0xd4:		// send to mouse
						break;
					case 0x1ed:
						// set leds: b0-scrlck,b1-numlck,b2-caps
						ps2c_wr_ob(ctrl, 0xfa);
						break;
					case 0x1f0:
						// set scancode tab: 0-get current, 1..3-scanset 1..3
						if (val == 0) {
							ps2c_wr_ob(ctrl, 0x02fa);	// (kbd->scanset << 8) | fa)
						} else {
							ps2c_wr_ob(ctrl, 0xfa);
						}
						break;
					case 0x1f3:
						ctrl->kbd->kdel = (((val >> 5) & 3) + 1) * 250e6;	// 1st delay - 250,500,750,1000ms
						ctrl->kbd->kper = (33 + 7 * (val & 0x1f)) * 1e6;	// repeat period: 33 to 250 ms
						ps2c_wr_ob(ctrl, 0xfa);
						break;
				}
			}
			ctrl->data = val;
			ctrl->cmd = -1;
			break;
		case PS2_RCMD:
			// printf("i8042 cmd %.2X\n",val);
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
							ps2c_wr_ob(ctrl, 0xFF);
							break;
						case 0xa4:
							ps2c_wr_ob(ctrl, 0xf1);		// f1:no password protect, fa:password protect presented
							break;
						case 0xa5:		// write password (all input data bytes till 0x00)
							break;
						case 0xa6:		// allow password check
							break;
						case 0xa7:		// disable 2nd ps/2 device
							ctrl->ram[0] |= 0x20;
							break;
						case 0xa8:		// enable 2nd ps/2 device
							ctrl->ram[0] &= ~0x20;
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
							ctrl->ram[0] |= 0x10;
							ctrl->inport &= 0x7f;
							break;
						case 0xae:		// enable kbd
							ctrl->ram[0] &= ~0x10;
							ctrl->inport |= 0x80;
							break;
						case 0xaf:		// read controller version (again?)
							break;
					}
					break;
				case 0xc0:
					switch (val) {
						case 0xc0:		// read controller input port
							ctrl->inport = 0x20;				// kbd unlocked bit set
							if (!ctrl->kbd->lock) ctrl->outbuf |= 0x80;	// kbd enabled
							ps2c_wr_ob(ctrl, ctrl->inport);
							break;
						case 0xc1:		// b0..3 input port -> b4..7 status byte
							//ctrl->status &= 0x0f;
							//ctrl->status |= ((ctrl->inbuf << 4) & 0xf0);
							break;
						case 0xc2:		// b4..7 input port -> b4..7 status byte
							//ctrl->status &= 0x0f;
							//ctrl->status |= (ctrl->inbuf & 0xf0);
							break;
						case 0xd0:		// read controller output port
							ps2c_wr_ob(ctrl, ctrl->outport);
							break;
						case 0xd1:		// write next byte to controller output port
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
								ps2c_wr_ob(ctrl, 0);
								break;
						}
					}
					break;
			}
			break;
	}
}

void ps2c_sync(PS2Ctrl* ctrl, int ns) {
	ctrl->delay += xt_sync(ctrl->kbd, ns);
	if (ctrl->delay > 0) {
		ctrl->delay -= ns;
		if (ctrl->delay < 0) {
			ctrl->delay = 0;
			if (ctrl->kbd->outbuf & 0xff) {
				ps2c_rd_kbd(ctrl);
			}
		}
	}
}
