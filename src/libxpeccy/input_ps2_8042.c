// https://wiki.osdev.org/"8042"_PS/2_Controller

#include <stdlib.h>
#include <string.h>

#include "input.h"

ps2Ctrl* ps2_create(Keyboard* kp, Mouse* mp) {
	ps2Ctrl* ps2c = (ps2Ctrl*)malloc(sizeof(ps2Ctrl));
	memset(ps2c, 0, sizeof(ps2Ctrl));
	ps2c->kbd = kp;
	ps2c->mouse = mp;
	return ps2c;
}

void ps2_destroy(ps2Ctrl* ps2c) {
	free(ps2c);
}

int ps2_rd(ps2Ctrl* ps2c, int port) {
	int res = -1;
	switch (port) {
		case PS2_STATUS:
			res = ps2c->status;
			break;
		case PS2_DATA:
			if (ps2c->status & PS2S_OBF) {
				res = ps2c->obuf;
				ps2c->status &= ~PS2S_OBF;
			}
			break;
	}
	return res;
}

/* controller output port:
0	System reset (output)
1	A20 gate (output)
2	Second PS/2 port clock (output, only if 2 PS/2 ports supported)
3	Second PS/2 port data (output, only if 2 PS/2 ports supported)
4	Output buffer full with byte from first PS/2 port (connected to IRQ1)
5	Output buffer full with byte from second PS/2 port (connected to IRQ12, only if 2 PS/2 ports supported)
6	First PS/2 port clock (output)
7	First PS/2 port data (output)
*/
// par = -1 for 1byte commands
void ps2_exec(ps2Ctrl* ps2c, int com, int par) {
	switch (com & 0xe0) {
		case 0x20:		// 20..3F read internal ram. 20 = config
			if (com == 0x20) {
				ps2c->obuf = ps2c->status;
			} else {
				ps2c->obuf = ps2c->ram[com & 0x1f];
			}
			ps2c->status |= PS2S_OBF;
			break;
		case 0x40: break;		// no com
		case 0x60:			// 60..7F write internal ram. 20 = config
			if (par < 0) {
				ps2c->wpar = 1;
			} else {
				ps2c->wpar = 0;
				ps2c->ram[com & 0x1f] = par & 0xff;
				if (com == 0x20)
					ps2c->config = par & 0xff;
			}
			break;
		default:
			switch (com & 0xff) {
				case 0xa7:		// disable 2nd port
					break;
				case 0xa8:		// enable 2nd port
					break;
				case 0xa9:		// test 2nd port
					ps2c->obuf = 0x00;	// 0x00 test passed, 0x01 clock line stuck low, 0x02 clock line stuck high, 0x03 data line stuck low, 0x04 data line stuck high
					ps2c->status |= PS2S_OBF;
					break;
				case 0xaa:		// test controller
					ps2c->obuf = 0x55;	// 0x55 test passed, 0xFC test failed
					ps2c->status |= PS2S_OBF;
					break;
				case 0xab:		// test 1st port
					ps2c->obuf = 0x00;
					ps2c->status |= PS2S_OBF;
					break;
				case 0xac:		// read all bytes of internal ram
					break;
				case 0xad:		// disable 1st port
					break;
				case 0xae:		// enable 1st port
					break;
				case 0xc0:		// read controller input port
					ps2c->obuf = 0xff;
					ps2c->status |= PS2S_OBF;
					break;
				case 0xc1:		// input port bits 0-3 -> status bits 4-7
					break;
				case 0xc2:		// input port bits 4-7 -> status bits 4-7
					break;
				case 0xd0:		// read contorller output port
					ps2c->obuf = 0xff;
					ps2c->status |= PS2S_OBF;
					break;
				case 0xd1:		// write next byte to controller output port
					if (par < 0) {
						ps2c->wpar = 1;
					} else {

					}
					break;
				case 0xd2:		// write next byte to 1st port output buffer
					if (par < 0) {
						ps2c->wpar = 1;
					} else {

					}
					break;
				case 0xd3:		// write next byte to 2nd port output buffer
					if (par < 0) {
						ps2c->wpar = 1;
					} else {

					}
					break;
				case 0xd4:		// write next byte to 2nd port input buffer
					if (par < 0) {
						ps2c->wpar = 1;
					} else {

					}
					break;
				default:
					if ((com & 0xf0) == 0xf0) {		// com.bit0 = 1 -> reset
						// output pulses
					}
					break;
			}
			break;
	}
}

void ps2_wr(ps2Ctrl* ps2c, int port, int val) {
	switch(port) {
		case PS2_COM:
			ps2c->status |= PS2S_COM;
			ps2c->ibuf = val & 0xff;
			ps2c->com = ps2c->ibuf;
			ps2_exec(ps2c, ps2c->com, -1);			// exec 1-byte command or wait for data for 2-byte commands
			break;
		case PS2_DATA:
			if (!(ps2c->status & PS2S_IBF)) {		// input buffer is free
				ps2c->ibuf = val & 0xff;		// fill input buffer
				ps2c->status |= PS2S_IBF;
				if (ps2c->wpar) {			// is this command argument
					ps2_exec(ps2c, ps2c->com, ps2c->ibuf);	// exec 2-byte command
					ps2c->status &= ~PS2S_IBF;
				} else {
					// else what?
				}
			} else {
				// else what?
			}
			break;
	}
}
