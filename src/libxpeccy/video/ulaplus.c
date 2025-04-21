#include "ulaplus.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ulaPlus* ula_create() {
	ulaPlus* ula = (ulaPlus*)malloc(sizeof(ulaPlus));
	if (ula) {
		memset(ula,0x00,sizeof(ulaPlus));
		ula->conttype = CONT_PATA;
		ula->early = 0;
	}
	return ula;
}

void ula_destroy(ulaPlus* ula) {
	if (ula) free(ula);
}

int ula_wr(ulaPlus* ula, int port, int val) {
	if (!ula->enabled) return 0;
	int res = 1;
	switch (port) {
		case 0xbf3b:
			ula->reg = val & 0xff;
			break;
		case 0xff3b:
			switch (ula->reg & 0xc0) {
				case 0x00:		// palette group
					ula->pal[ula->reg & 0x3f] = val & 0xff;
					ula->palchan = 1;
					break;
				case 0x40:		// mode group
					ula->active = val & 1;
					ula->palchan = 1;
					break;
			}
			break;
		default:
			res = 0;
			break;
	}
	return res;
}

int ula_rd(ulaPlus* ula, int port, int* val) {
	if (!ula->enabled) return 0;
	int res = 1;
	switch (port) {
		case 0xbf3b:		// port is write only
			*val = 0xff;
			break;
		case 0xff3b:
			switch (ula->reg & 0xc0) {
				case 0x00:
					*val = ula->pal[ula->reg & 0x3f];
					break;
				case 0x40:
					*val = ula->active ? 1 : 0;
					break;
			}
			break;
		default:
			res = 0;
			break;
	}
	return res;
}
