#include "ulaplus.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ulaPlus* ulaCreate() {
	ulaPlus* ula = (ulaPlus*)malloc(sizeof(ulaPlus));
	if (ula) {
		memset(ula,0x00,sizeof(ulaPlus));
	}
	return ula;
}

void ulaDestroy(ulaPlus* ula) {
	if (ula) free(ula);
}

int ulaOut(ulaPlus* ula, unsigned short port, unsigned char val) {
	if (!ula->enabled) return 0;
	int res = 1;
	switch (port) {
		case 0xbf3b:
			ula->reg = val;
			break;
		case 0xff3b:
			switch (ula->reg & 0xc0) {
				case 0x00:		// palette group
					ula->pal[ula->reg & 0x3f] = val;
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
//	if (res == 1) printf("ULA+ out %.4X,%.2X\n",port,val);
	return res;
}

int ulaIn(ulaPlus* ula, unsigned short port, unsigned char* val) {
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
//	if (res) printf("ULA+ in %.4X = %.2X\n",port,*val);
	return res;
}
