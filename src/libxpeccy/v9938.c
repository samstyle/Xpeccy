#include "video.h"

#include <stdio.h>
#undef NDEBUG
#include <assert.h>

void vdpMemWr(VDP9938* vdp, unsigned char val) {
	vdp->ram[vdp->vadr & 0x1ffff] = val;
	vdp->vadr++;
}

void vdpPalWr(Video* vid, unsigned char val) {
	VDP9938* vdp = &vid->v9938;
	xColor col;
	if (vdp->high) {
		vdp->high = 0;
		col.b = (vdp->data & 7) << 5;
		col.r = (vdp->data & 0x70) << 1;
		col.g = (val & 7) << 5;
		vid->pal[vdp->reg[0x10] & 0x0f] = col;
		vdp->reg[0x10]++;
	} else {
		vdp->data = val;
		vdp->high = 1;
	}
}

void vdpRegWr(Video* vid, unsigned char val) {
	VDP9938* vdp = &vid->v9938;
	int reg;
	int vmode;
	if (vdp->high) {
		if (val & 0x80) {		// b7.hi = 1 : VDP register setup
			reg = val & 0x3f;
			val = vdp->data;
			vdp->reg[reg] = val;
			switch (reg) {
				case 0:
				case 1:
					vmode = ((vdp->reg[1] & 0x10) >> 4) | ((vdp->reg[1] & 8) >> 2) | ((vdp->reg[0] & 0x0e) << 1);
					switch (vmode) {
						case 1: vidSetMode(vid, VID_MSX_SCR0); break;	// text 40x24
						case 0: vidSetMode(vid, VID_MSX_SCR1); break;	// text 32x24
						case 4: vidSetMode(vid, VID_MSX_SCR2); break;	// 256x192
						case 2: vidSetMode(vid, VID_MSX_SCR3); break;	// multicolor 4x4
						case 0x08: vidSetMode(vid, VID_MSX_SCR4); break;	// scr2 8spr/line
						case 0x0c: vidSetMode(vid, VID_MSX_SCR5); break;	// 256x212,4bpp
						case 0x10: vidSetMode(vid, VID_MSX_SCR6); break;	// 512x212,2bpp
						case 0x14: vidSetMode(vid, VID_MSX_SCR7); break;	// 512x212,4bpp
						case 0x1c: vidSetMode(vid, VID_MSX_SCR8); break;	// 256x212,8bpp
						case 0x09: vidSetMode(vid, VID_MSX_SCR9); break;	// text 80x24
						default:
							printf("v9938 mode %.2X\n",vmode);
							assert(0);
							vidSetMode(vid, VID_UNKNOWN);
							break;
					}
					break;
				case 2: vdp->BGMap = (val & 0x7f) << 10; break;
				case 3: vdp->BGColors = val << 6; break;
				case 4: vdp->BGTiles = (val & 0x3f) << 11; break;
				case 5: vdp->OBJAttr = val << 7; break;
				case 6: vdp->OBJTiles = (val & 0x3f) << 11; break;
				case 7: vid->nextbrd = vdp->reg[7] & 7; break;		// border color = BG in R7
				case 0x09:
					vdp->lines = (val & 0x80) ? 212 : 192;
					break;
				case 0x0b: vdp->SPRAttr = (val & 3) << 15; break;		// sprite color base adr (A16,A15)
				case 0x0e: vdp->vadr = (vdp->vadr & 0x3fff) | ((val & 7) << 14); break;
				case 0x20:
				case 0x21: vdp->srcX = (vdp->reg[0x20] | (vdp->reg[0x21] << 8)) & 0x1ff; break;
				case 0x22:
				case 0x23: vdp->srcY = (vdp->reg[0x22] | (vdp->reg[0x23] << 8)) & 0x3ff; break;
				case 0x24:
				case 0x25: vdp->dstX = (vdp->reg[0x24] | (vdp->reg[0x25] << 8)) & 0x1ff; break;
				case 0x26:
				case 0x27: vdp->dstY = (vdp->reg[0x26] | (vdp->reg[0x27] << 8)) & 0x3ff; break;
				case 0x28:
				case 0x29: vdp->sizX = (vdp->reg[0x28] | (vdp->reg[0x29] << 8)) & 0x1ff; break;
				case 0x2a:
				case 0x2b: vdp->sizY = (vdp->reg[0x2a] | (vdp->reg[0x2b] << 8)) & 0x3ff; break;

				case 0x08:
				case 0x10:			// palette num
				case 0x17:			// Y offset (0..255)
				case 0x2c:
				case 0x2d:
					break;
				default:
					printf("v9938 register : wr #%.2X,#%.2X\n",reg,val);
					assert(0);
					break;
			}
		} else {			// b7.hi = 0 : VDP address setup
			vdp->vadr = ((val & 0x3f) << 8) | vdp->data;
			vdp->wr = (val & 0x40) ? 1 : 0;
		}
		vdp->high = 0;
	} else {
		vdp->data = val;
		vdp->high = 1;
	}
}
