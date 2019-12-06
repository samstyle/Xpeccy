#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "video.h"

// C64 vic interrupts
#define VIC_IRQ_RASTER	0x01
#define	VIC_IRQ_SPRBGR	0x02
#define VIC_IRQ_SPRSPR	0x04
#define VIC_IRQ_LPEN	0x08

extern int xscr;
extern int yscr;
extern unsigned char col;
extern unsigned char ink;
extern unsigned char pap;
extern unsigned char scrbyte;
extern unsigned char atrbyte;
extern int adr;

void vic_irq(Video* vid, int mask) {
	vid->intrq |= mask;
	vid->intrq &= 0x0f;
	if (vid->intrq & vid->inten) {
		vid->intrq |= 0x80;
		vid->irq = 1;
	}
}

// rd/wr

unsigned char vic_rd(Video* vid, unsigned short adr) {
	adr &= 0x3f;
	unsigned char res = vid->reg[adr];
	switch (adr) {
		case 0x11:
			res &= 0x7f;
			if (vid->ray.y & 0x100)
				res |= 0x80;
			break;
		case 0x12:
			res = vid->ray.y & 0xff;
			break;
		case 0x19:
			res = vid->intrq;
			vid->intrq = 0;
			break;
	}
	return res;
}

void vic_set_mode(Video* vid) {
	int mode = ((vid->reg[0x11] & 0x60) | (vid->reg[0x16] & 0x10));
//	printf("mode:%.2X\n",mode);
	switch (mode) {
		case 0x00: vidSetMode(vid, VID_C64_TEXT); break;
		case 0x10: vidSetMode(vid, VID_C64_TEXT_MC); break;
		case 0x20: vidSetMode(vid, VID_C64_BITMAP); break;
		case 0x30: vidSetMode(vid, VID_C64_BITMAP_MC); break;
		// and extend modes
		default: vidSetMode(vid, VID_UNKNOWN); break;
	}
}

void vic_wr(Video* vid, unsigned short adr, unsigned char val) {
	adr &= 0x3f;
	vid->reg[adr] = val;
	switch (adr) {
		case 0x11:				// b7 = b8 of INT line
			vid->intp.y &= 0xff;
			if (val & 0x80)
				vid->intp.y |= 0x100;
			vic_set_mode(vid);
			break;
		case 0x12:				// b0..7 of INT line
			vid->intp.y &= 0x100;
			vid->intp.y |= (val & 0xff);
			break;
		case 0x16:
			vic_set_mode(vid);
			break;
		case 0x18:
			vid->sbank = (val & 0x0e) >> 1;
			vid->cbank = (val & 0xf0) >> 4;
			break;
		case 0x19:
			vid->inten = val & 0x0f;	// b0:line int, b1:spr-bgr collision, b2:spr-spr collision, b3:light pen
			break;
	}
}

// text mode
void vidC64TDraw(Video* vid) {
	if (vid->reg[0x11] & 0x10)
	yscr = vid->ray.y - vid->bord.y;
	if ((yscr < 0) || (yscr >= vid->scrn.y) || !(vid->reg[0x11] & 0x10)) {
		if (vid->line[vid->ray.xb] != 0xff) {
			col = vid->line[vid->ray.xb];
		} else if (vid->linb[vid->ray.x] != 0xff) {
			col = vid->linb[vid->ray.xb];
		} else {
			col = vid->reg[0x20];				// border color
		}
	} else {
		xscr = vid->ray.x - vid->bord.x;
		if ((xscr < 0) || (xscr >= vid->scrn.x)) {
			col = vid->reg[0x20];			// border color
		} else {
			if ((xscr & 7) == 0) {
				adr = ((yscr >> 3) * 40) + (xscr >> 3);
				ink = vid->mrd(adr | ((vid->reg[0x18] & 0xf0) << 6), vid->data);				// tile nr
				if ((~vid->vbank & 1) && ((vid->reg[0x18] & 0x0c) == 0x04)) {
					scrbyte = vid->font[(ink << 3) | (yscr & 7)];	// from char rom
				} else {
					scrbyte = vid->mrd(((vid->reg[0x18] & 0x0e) << 10) | (ink << 3) | (yscr & 7), vid->data);	// tile row data
				}
				ink = vid->colram[adr & 0x3ff];			// tile color
				pap = vid->reg[0x21];				// background color
			}
			if (vid->line[vid->ray.xb] != 0xff) {
				col = vid->line[vid->ray.xb];
			} else if (scrbyte & 0x80) {
				col = ink;
			} else if (vid->linb[vid->ray.x] != 0xff) {
				col = vid->linb[vid->ray.xb];
			} else {
				col = pap;
			}
			scrbyte <<= 1;
		}
	}
	vidPutDot(&vid->ray, vid->pal, col & 0x0f);
}

// multicolor text
// if bit 4 in color ram = 1, this is multicolor cell
// if bit 4 in color ram = 0, this is common cell

void vidC64TMDraw(Video* vid) {
	yscr = vid->ray.y - vid->bord.y;
	if ((yscr < 0) || (yscr >= vid->scrn.y) || !(vid->reg[0x11] & 0x10)) {
		if (vid->line[vid->ray.xb] != 0xff) {
			col = vid->line[vid->ray.xb];
		} else if (vid->linb[vid->ray.xb] != 0xff) {
			col = vid->linb[vid->ray.xb];
		} else {
			col = vid->reg[0x20];				// border color
		}
	} else {
		xscr = vid->ray.x - vid->bord.x;
		if ((xscr < 0) || (xscr >= vid->scrn.x)) {
			col = vid->reg[0x20];			// border color
		} else {
			if ((xscr & 7) == 0) {
				adr = ((yscr >> 3) * 40) + (xscr >> 3);						// offset to tile
				ink = vid->mrd(adr | ((vid->reg[0x18] & 0xf0) << 6), vid->data);		// tile nr
				if ((~vid->vbank & 1) && ((vid->reg[0x18] & 0x0c) == 0x04)) {
					scrbyte = vid->font[(ink << 3) | (yscr & 7)];
				} else {
					scrbyte = vid->mrd(((vid->reg[0x18] & 0x0e) << 10) | (ink << 3) | (yscr & 7), vid->data);
				}
				atrbyte = vid->colram[adr & 0x3ff];			// tile color
				pap = vid->reg[0x21];				// background color
			}
			if (vid->line[xscr] != 0xff) {
				col = vid->line[xscr] & 0x0f;
			} else if (atrbyte & 8) {
				switch (scrbyte & 0xc0) {
					case 0x00:
						if (vid->linb[vid->ray.xb] != 0xff) {
							col = vid->linb[vid->ray.xb] & 0x0f;
						} else {
							col = vid->reg[0x21];
						}
						break;
					case 0x40:
						col = vid->reg[0x22];
						break;
					case 0x80:
						col = vid->reg[0x23];
						break;
					case 0xc0:
						col = atrbyte & 7;	// only colors 0..7
						break;
				}
				if (xscr & 1) scrbyte <<= 2;
			} else {			// not multicolor
				if (vid->line[vid->ray.xb] != 0xff) {
					col = vid->line[vid->ray.xb];
				} else if (scrbyte & 0x80) {
					col = atrbyte;
				} else if (vid->linb[vid->ray.xb] != 0xff) {
					col = vid->linb[vid->ray.xb];
				} else {
					col = pap;
				}
				scrbyte <<= 1;
			}
		}
	}
	vidPutDot(&vid->ray, vid->pal, col & 0x0f);
}

// bitmap
void vidC64BDraw(Video* vid) {
	if (vid->reg[0x11] & 0x10) {
		yscr = vid->ray.y - vid->bord.y;
		if ((yscr < 0) || (yscr >= vid->scrn.y)) {
			col = vid->reg[0x20];
		} else {
			xscr = vid->ray.x - vid->bord.x;
			if ((xscr < 0) || (xscr >= vid->scrn.x)) {
				col = vid->reg[0x20];
			} else {
				if ((xscr & 7) == 0) {
					adr = (yscr >> 3) * 320 + (xscr & ~7) + (yscr & 7);
					scrbyte = vid->mrd(adr | ((vid->reg[0x18] & 0x08) << 10), vid->data);
					adr = (yscr >> 3) * 40 + (xscr >> 3);
					ink = vid->mrd(adr | ((vid->reg[0x18] & 0xf0) << 6), vid->data);
					pap = ink & 0x0f;		// 0
					ink = (ink >> 4) & 0x0f;	// 1
				}
				col = (scrbyte & 0x80) ? ink : pap;
				scrbyte <<= 1;
			}
		}
	} else {
		col = vid->reg[0x20];
	}
	vidPutDot(&vid->ray, vid->pal, col & 0x0f);
}

// multicolor bitmap
void vidC64BMDraw(Video* vid) {
	if (vid->reg[0x11] & 0x10) {
		yscr = vid->ray.y - vid->bord.y;
		if ((yscr < 0) || (yscr >= vid->scrn.y)) {
			col = vid->reg[0x20];
		} else {
			xscr = vid->ray.x - vid->bord.x;
			if ((xscr < 0) || (xscr >= vid->scrn.x)) {
				col = vid->reg[0x20];
			} else {
				if ((xscr & 7) == 0) {
					adr = (yscr >> 3) * 320 + (xscr & 0xf8) + (yscr & 7);
					scrbyte = vid->mrd(adr | ((vid->reg[0x18] & 0x08) << 10), vid->data);
				}
				adr = (yscr >> 3) * 40 + (xscr >> 3);
				ink = vid->mrd(adr | ((vid->reg[0x18] & 0xf0) << 6), vid->data);
				if ((xscr & 1) == 0) {
					switch (scrbyte & 0xc0) {
						case 0x00:
							col = vid->reg[0x21];
							break;
						case 0x40:
							col = (ink >> 4) & 0x0f;
							break;
						case 0x80:
							col = ink & 0x0f;
							break;
						case 0xc0:
							col = vid->colram[adr & 0x3ff] >> 4;
							break;
					}
					scrbyte <<= 2;
				}
			}
		}
	} else {
		col = vid->reg[0x20];
	}
	vidPutDot(&vid->ray, vid->pal, col & 0x0f);
}

void vidC64Fram(Video* vid) {
}

// default size: 24x21 pix
// 07f8..ff	sprites pointers (+ [53272] shift)
// d000/01	sprite 0 x,y
// d00e/0f	sprite 7 x,y
// d010	bits	sprite x 8th bit
// d015	bits	sprite enabled
// d017 bits	sprite is double height
// d01b	bits	sprite priority (0:above screen, 1:behind screen)
// d01c	bits	sprite multicolor
// d01d bits	sprite is double width
// d01e	bits	wr:spr-bgr collision enabled; rd:detected collisions
// d01f bits	wr:spr-spr collision enabled; rd:detected collisions
// d025		spr extra color 1
// d026		spr extra color 2
// d027..2e	sprite colors

void vidC64Line(Video* vid) {
	if (vid->intp.y == vid->ray.y) {		// if current line == interrupt line and raster interrupt is enabled
		vic_irq(vid, VIC_IRQ_RASTER);
	}
	// sprites
	memset(vid->line, 0xff, 512);	// ff as transparent color
	memset(vid->linb, 0xff, 512);
	int yscr = vid->ray.yb;// - vid->lay.bord.y;	// current screen line
	unsigned char sprxh = vid->reg[0x10];	// bit x : sprite x X 8th bit
	unsigned char spren = vid->reg[0x15];	// bit x : sprite x enabled
	unsigned char sprmc = vid->reg[0x1c];	// bit x : sprite x multicolor
	unsigned char sprhi = vid->reg[0x17];	// bit x : sprite x double height
	unsigned char sprpr = vid->reg[0x1b];	// bit x : sprite x behind screen
	unsigned char sprwi = vid->reg[0x1d];	// bit x : sprite x double width
	unsigned char msk = 1;
	int posy;
	int posx;
	int sadr;
	unsigned char dat;
	unsigned char scol;
	unsigned char* ptr;
	int bn;
	int bt;
	int i;
	vid->sprxspr = 0;
	for (i = 0; i < 8; i++) {
		if (spren & msk) {
			posy = yscr - vid->reg[i * 2 + 1];		// line inside sprite
			if (sprhi & msk)
				posy = posy / 2;
			if ((posy >= 0) && (posy < 21)) {
				posx = vid->reg[i * 2];
				if (sprxh & msk)
					posx |= 0x100;
//				printf("%.2X, %.4X, %.2X\n",vid->reg[0x18] & 0x07, (((vid->reg[0x18] & 0x07) << 11) | 0x7f8) + i, vid->vbank);
//				assert(0);
				sadr = vid->mrd(((vid->reg[0x18] & 0xf0) << 6) + 0x3f8 + i, vid->data) << 6;		// address of sprite data
				sadr += posy * 3;									// address of current line
				for (bn = 0; bn < 3; bn++) {
					dat = vid->mrd(sadr, vid->data);
					sadr++;
					ptr = (sprpr & msk) ? vid->linb : vid->line;
					if (sprmc & msk) {		// multicolor
						for (bt = 0; bt < 4; bt++) {
							switch (dat & 0xc0) {
								case 0x00: scol = 0xff; break;	// transparent
								case 0x40: scol = vid->reg[0x25] & 0x0f; break;
								case 0x80: scol = vid->reg[0x27 + i] & 0x0f; break;
								default: scol = vid->reg[0x26] & 0x0f; break;
							}
							//if (ptr[posx] != 0xff) vid->sprxspr |= msk;
							ptr[posx & 0x1ff] = scol;
							posx++;
							//if (ptr[posx] != 0xff) vid->sprxspr |= msk;
							ptr[posx & 0x1ff] = scol;
							posx++;
							if (sprwi & msk) {
								//if (ptr[posx] != 0xff) vid->sprxspr |= msk;
								ptr[posx & 0x1ff] = scol;
								posx++;
								//if (ptr[posx] != 0xff) vid->sprxspr |= msk;
								ptr[posx & 0x1ff] = scol;
								posx++;
							}
							dat <<= 2;
						}
					} else {			// common
						for (bt = 0x80; bt > 0; bt >>= 1) {
							scol = (dat & bt) ? (vid->reg[0x27 + i] & 0x0f) : 0xff;
							if (ptr[posx] == 0xff) {
								ptr[posx & 0x1ff] = scol;
							} else {
								vid->sprxspr |= msk;
							}
							posx++;
							if (sprwi & msk) {
								if (ptr[posx] == 0xff) {
									ptr[posx & 0x1ff] = scol;
								} else {
									vid->sprxspr |= msk;
								}
								ptr[posx & 0x1ff] = scol;
								posx++;
							}
							// dat <<= 1;
						}
					}
				}
			}
		}
		msk <<= 1;
	}
}
