#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "i8042_kbd.h"

#define KBD_DELAY 1e7

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
	ctrl->outbuf = 0;
	ctrl->outport = 2;
	ctrl->status &= ~0x03;
	ctrl->ram[0] = 0x00;
	ctrl->delay = 0;
	ctrl->dmode = PS2C_MODE_KBD;
}

void ps2c_clear(PS2Ctrl* ctrl) {
	ctrl->status &= ~0x03;
}

void ps2c_ready(PS2Ctrl* ctrl, int dev) {
	if (dev & 1) {		// mouse
		ctrl->m_rdy = 1;
	} else {		// keyboard
		ctrl->k_rdy = 1;
	}
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
			if (ctrl->status & 1) {
				res = ctrl->outbuf & 0xff;
				ctrl->status &= ~1;
			}
			//printf("i8042 read code %.2X\n",res);
			break;
		case PS2_RSTATUS:
			res = ctrl->status | 0x10;		// b4 = keyboard lock off
			ctrl->status &= ~2;
			break;
	}
	return res;
}

// TODO: set status.bit0 after reading status reg (0 immediately, 1 after reading) ???
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
	int d = xt_read(ctrl->kbd);
	if (ctrl->kbd->lock || (ctrl->ram[0] & 0x10)) return;
	if (d < 0) {
		ctrl->k_rdy = 0;
	} else {
		ps2c_wr_ob(ctrl, d);
		ctrl->xirq(IRQ_KBD, ctrl->xptr);
	}
}

void ps2c_rd_mouse(PS2Ctrl* ctrl) {
	int d = mouse_rd(ctrl->mouse);
	if (ctrl->ram[0] & 0x20) return;	// 2nd device disabled (data readed and droped)
	if (d < 0) {				// no data left
		ctrl->m_rdy = 0;
	} else {
		ps2c_wr_ob2(ctrl, d);		// write data to outbuf
		if (ctrl->ram[0] & 2) {		// 2nd device interrupt enabled
			ctrl->xirq(IRQ_MOUSE, ctrl->xptr);
		}
	}
}

void ps2c_wr(PS2Ctrl* ctrl, int adr, int val) {
	ctrl->inbuf = val;
	ctrl->status |= 2;
	switch (adr) {
		case PS2_RDATA:
			//printf("PS/2 controller wr data %.2X\n",val);
			ctrl->status |= 8;
			switch (ctrl->dmode) {
				case PS2C_MODE_KBD:
					kbd_wr(ctrl->kbd, val);
					break;
				case PS2C_MODE_MOU:
					mouse_wr(ctrl->mouse, val);
					ctrl->dmode = PS2C_MODE_KBD;
					break;
				case PS2C_MODE_COM:
					switch (ctrl->cmd) {
						case 0xd1:		// write to output port
							ctrl->outport = val;
							if (!(val & 1)) {
								ctrl->xirq(IRQ_RESET, ctrl->xptr);
							}
							// b1:a20 gate. if 0 here [or port 92 bit 1 = 0], a20+ is disabled (always 0)
							break;
						case 0xd2:
							ps2c_wr_ob(ctrl, val);
							break;
						case 0xd3:
							ps2c_wr_ob2(ctrl, val);
							break;
						default:
							if ((ctrl->cmd & 0xe0) == 0x60) {
								if (ctrl->cmd == 0x60) {
									ctrl->status &= ~4;
									ctrl->status |= (val & 4);
									val &= ~0x40;
								}
								ctrl->ram[ctrl->cmd & 0x1f] = val & 0xff;
								// if (!(ctrl->cmd & 0x1f)) printf("i8042 conf = %.2X\n",val & 0xff);
							}
							break;
					}
					ctrl->cmd = -1;
					ctrl->dmode = PS2C_MODE_KBD;
					break;
			}
			ctrl->data = val;	// last byte
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
					ctrl->dmode = PS2C_MODE_COM;
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
							ctrl->cmd = val;
							ctrl->dmode = PS2C_MODE_COM;
							break;
						case 0xd4:		// write next byte to 2nd ps/2 input buffer (send command to mouse)
							ctrl->dmode = PS2C_MODE_MOU;
							break;
					}
					break;
				case 0xe0:
					if (val == 0xff) {				// FF reset
						ps2c_reset(ctrl);
					} else if ((val & 0xf0) == 0xf0) {		// Fx commands
						if (!(val & 1)) {
							ctrl->xirq(IRQ_RESET, ctrl->xptr);
						}
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
#if !USE_HOST_KEYBOARD
	xt_sync(ctrl->kbd, ns);
#endif
	ctrl->delay -= ns;
	while (ctrl->delay < 0) {
		ctrl->delay += 1e9/2000;		// TODO: 2KB/s ?
		if (ctrl->k_rdy) { // (ctrl->kbd->outbuf & 0xff) {
			ps2c_rd_kbd(ctrl);
		} else if (ctrl->m_rdy) {
			ps2c_rd_mouse(ctrl);
		}
	}
}
