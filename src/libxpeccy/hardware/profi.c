#include "../spectrum.h"
#include <assert.h>

void prfMapMem(ZXComp* comp) {
	memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->prt0 & 0x10) ? 1 : 0);
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,((comp->prt1 & 7) << 3) | (comp->prt0 & 7));
}

Z80EX_WORD prfGetPort(int port, int bdiz) {
	if ((port & 0x00ff) == 0x00fe) return ((port & 0xff00) | 0xfe);
	if ((port & 0xe0ff) == 0x60fd) return 0x7ffd;
	if ((port & 0xe0ff) == 0xe0fd) return 0xfffd;
	if ((port & 0xe0ff) == 0xa0fd) return 0xbffd;
	return port;
}

Z80EX_BYTE prfIn(ZXComp* comp, Z80EX_WORD port, int bdiz) {
	Z80EX_BYTE res = 0xff;
	port = prfGetPort(port,bdiz);
	switch (port) {
		case 0xbffd:
		case 0xfffd:
			res = tsIn(comp->ts,port);
			break;
		default:
			switch (port & 0xff) {
				case 0xfe:
					res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00);
					break;
				default:
					printf("Profi in %.4X (%i)\n",port,bdiz);
					comp->flag |= ZX_BREAK;
					// assert(0);
					break;
			}
	}
	return res;
}

void prfOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int bdiz) {
	port = prfGetPort(port,bdiz);
	switch (port) {
		case 0x7ffd:
			if (comp->prt0 & 0x20) break;
			comp->prt0 = val;
			comp->vid->curscr = (val & 0x08) ? 7 : 5;
			prfMapMem(comp);
			break;
		case 0xbffd:
		case 0xfffd:
			tsOut(comp->ts,port,val);
			break;
		default:
			switch (port & 0xff) {
				case 0xfe:
					comp->vid->nextbrd = val & 7;
					comp->beeplev = val & 0x10;
					comp->tape->outsig = (val & 0x08) ? 1 : 0;
					break;
				default:
					printf("Profi out %.4X,%.2X (%i)\n",port,val,bdiz);
					comp->flag |= ZX_BREAK;
					//assert(0);
					break;
			}
	}
}
