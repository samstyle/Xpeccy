// modes: https://osdev.fandom.com/ru/wiki/VGA_режимы

#include "vga.h"

#include <stdio.h>

int vga_rd(Video* vid, int port) {
	int res = -1;
	switch (port) {
		case VGA_REGVAL:
			if ((vid->idx >= 0x0c) && (vid->idx <= 0x11))
				res = vid->reg[vid->idx];
			break;
		case VGA_STAT1:
			// b0:videomem is busy
			// b2:light pen off
			// b3:vblank
			res = 4;
			if (vid->vblank)
				res |= 8;
			break;
		case VGA_MODE:
			res = vid->reg[0xff];
			break;
	}
	return res;
}

void vga_wr(Video* vid, int port, int val) {
	switch (port) {
		case VGA_REGNUM:
			vid->idx = val & 0xff;
			break;
		case VGA_REGVAL:
			printf("VGA: reg[%.2X] = %.2X\n", vid->idx & 0xff, val & 0xff);
			vid->idx &= 0xff;
			if (vid->idx > 0x11) break;
			vid->reg[vid->idx] = val & 0xff;
			switch(vid->idx) {
				case 0x00:		// horizontal total
					break;
				case 0x01:		// horizontal displayed
					break;
				case 0x02:		// horizontal sync position
					break;
				case 0x03:		// horizontal sync pulse width
					break;
				case 0x04:		// vertical total
					break;
				case 0x05:		// vertical displayed
					break;
				case 0x06:		// vertical sync position
					break;
				case 0x07:		// vertical sync pulse width
					break;
				case 0x08:		// interlace mode
					break;
				case 0x09:		// maximum scanlines
					break;
				case 0x0a:		// curosr start
					break;
				case 0x0b:		// cursor end
					break;
				case 0x0c:		// start address high
					break;
				case 0x0d:		// start address low
					break;
				case 0x0e:		// cursor location high
					break;
				case 0x0f:		// cursor location low
					break;
				case 0x10:		// light pen high
					break;
				case 0x11:		// light pen low
					break;
			}
			break;
		case VGA_MODE:
			vid->reg[0xff] = val & 0xff;
			// b4:gfx 640x200 or 320x200; b2:gfx(1) or text(0); b1:text 80x25
			printf("VGA mode %.2X\n",val & 0x13);
			switch (val & 0x13) {
				case 0x00: case 0x10:
					vidSetMode(vid, VID_VGA_T40);
					break;		// text 40x25
				case 0x01: case 0x11: break;		// text 80x25
				case 0x02: case 0x03: break;		// gfx 320x200
				case 0x12: case 0x13: break;		// gfx 640x200
			}
			if (val & 0x04) {}		// b2:monochrome
			if (val & 0x08) {}		// b3:video enabled
			if (val & 0x20) {}		// b5:blink enabled
			break;
	}
}
