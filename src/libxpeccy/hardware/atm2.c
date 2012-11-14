#include "../spectrum.h"

void atmSetBank(ZXComp* comp, int bank, memEntry me) {
	unsigned char page = me.page ^ 0xff;
	if (me.flag & 0x80) {
		if (me.flag & 0x40) {
			page = (page & 0x38) | (comp->prt0 & 7);	// mix with 7FFD bank;
		} else {
			page = (page & 0x3e) | (comp->dosen ? 1 : 0);	// mix with dosen
		}
	}
	memSetBank(comp->mem,bank,(me.flag & 0x40) ? MEM_RAM : MEM_ROM, page);
}

void atm2MapMem(ZXComp* comp) {
	if (comp->prt1 & 1) {			// pen = 0: last rom page in every bank && dosen on
		int adr = (comp->prt0 & 0x10) ? 4 : 0;
		atmSetBank(comp,MEM_BANK0,comp->memMap[adr]);
		atmSetBank(comp,MEM_BANK1,comp->memMap[adr+1]);
		atmSetBank(comp,MEM_BANK2,comp->memMap[adr+2]);
		atmSetBank(comp,MEM_BANK3,comp->memMap[adr+3]);
	} else {
		comp->dosen = 1;
		memSetBank(comp->mem,MEM_BANK0,MEM_ROM,0xff);
		memSetBank(comp->mem,MEM_BANK1,MEM_ROM,0xff);
		memSetBank(comp->mem,MEM_BANK2,MEM_ROM,0xff);
		memSetBank(comp->mem,MEM_BANK3,MEM_ROM,0xff);
	}
}

int atm2GetPort(Z80EX_WORD port,int bdiz) {
	if ((port & 0x0007) == 0x0006) return 0xfe;
	if ((port & 0x0007) == 0x0002) return 0xfa;	// interface port
	if ((port & 0x0007) == 0x0003) return 0xfb;	// covox
	if ((port & 0x8202) == 0x0200) return 0x7ffd;	// mem.main
	if ((port & 0x8202) == 0x0000) return 0x7dfd;	// digital.rd
	if ((port & 0xc202) == 0x8200) return 0xbffd;	// ay
	if ((port & 0xc202) == 0xc200) return 0xfffd;
	if (bdiz) {
		if ((port & 0x009f) == 0x009f) return 0xff;				// bdi: ff
		if ((port & 0x00ff) == 0x001f) return 0x1f;
		if ((port & 0x00ff) == 0x003f) return 0x3f;
		if ((port & 0x00ff) == 0x005f) return 0x5f;
		if ((port & 0x00ff) == 0x007f) return 0x7f;
		if ((port & 0x009f) == 0x0097) return 0xf7;				// F7: mem.manager (a14.15)
		if ((port & 0x009f) == 0x0017) return 0x77;				// 77: system (a8.a9.a14)
		if ((port & 0x001f) == 0x000f) return 0xef;				// EF: hdd (a8..11)
	}
	return 0;
}

void atm2Out(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int bdiz) {
	int adr;
	int ptype = atm2GetPort(port,bdiz);
	switch (ptype) {
		case 0x1f:
			if (bdiz) bdiOut(comp->bdi,FDC_COM,val);
			break;
		case 0x3f:
			if (bdiz) bdiOut(comp->bdi,FDC_TRK,val);
			break;
		case 0x5f:
			if (bdiz) bdiOut(comp->bdi,FDC_SEC,val);
			break;
		case 0x7f:
			if (bdiz) bdiOut(comp->bdi,FDC_DATA,val);
			break;
		case 0xff:
			if (bdiz) {
				bdiOut(comp->bdi,BDI_SYS,val);
				if (!(comp->prt1 & 0x40)) {
					val ^= 0xff;	// inverse colors
					comp->colMap[comp->vid->brdcol & 0x0f] =		// grbG--RB to -grb-GRB
							(val & 0x03) | ((val & 0x10) >> 2) | ((val & 0xe0) >> 1);
					comp->flag |= ZX_PALCHAN;
				}
			}
			break;
		case 0xfe:
			comp->vid->nextbrd = (val & 0x07) | (~port & 8);
			if (!(comp->vid->flags & VID_BORDER_4T))
				comp->vid->brdcol = comp->vid->nextbrd;
			comp->beeplev = val & 0x10;
			comp->tape->outsig = (val & 0x08) ? 1 : 0;
			break;
		case 0x77:
			switch (val & 7) {
				case 0: comp->vid->mode = VID_ATM_EGA; break;
				case 2: comp->vid->mode = VID_ATM_HWM; break;
				case 3: comp->vid->mode = VID_NORMAL; break;
				case 6: comp->vid->mode = VID_ATM_TEXT; break;
				default: comp->vid->mode = VID_UNKNOWN; break;
			}
			zxSetFrq(comp,(val & 0x08) ? 7.0 : 3.5);
			comp->prt1 = (port & 0xff00) >> 8;
			atm2MapMem(comp);
			break;
		case 0xf7:
			adr = ((comp->prt0 & 0x10) ? 4 : 0) | ((port & 0xc000) >> 14);	// rom2.a15.a14
			comp->memMap[adr].flag = val & 0xc0;		// copy b6,7 to flag
			comp->memMap[adr].page = (val & 0x3f) | 0xc0;	// set b6,7 for PentEvo capability
			atm2MapMem(comp);
			break;
		case 0x7ffd:
			if (comp->prt0 & 0x20) break;
			comp->prt0 = val;
			comp->vid->curscr = (val & 0x08) ? 1 : 0;
			atm2MapMem(comp);
			break;
		case 0xbffd:
		case 0xfffd:
			tsOut(comp->ts,ptype,val); break;
//		default:
//			printf("ATM2: out %.4X (%.4X) %.2X\n",port,ptype,val);
//			break;
	}
}

Z80EX_BYTE atm2In(ZXComp* comp, Z80EX_WORD port, int bdiz) {
	Z80EX_BYTE res = 0xff;
	int ptype = atm2GetPort(port,bdiz);
	switch (ptype) {
		case 0x1f:
			res = bdiz ? bdiIn(comp->bdi,FDC_STATE) : 0xff;
			break;
		case 0x3f:
			res = bdiz ? bdiIn(comp->bdi,FDC_TRK) : 0xff;
			break;
		case 0x5f:
			res = bdiz ? bdiIn(comp->bdi,FDC_SEC) : 0xff;
			break;
		case 0x7f:
			res = bdiz ? bdiIn(comp->bdi,FDC_DATA) : 0xff;
			break;
		case 0xff:
			res = bdiz ? bdiIn(comp->bdi,BDI_SYS) : vidGetAttr(comp->vid);
			break;
		case 0xfe:
			res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00);
			break;
		case 0x7ffd:
			res = 0xff;
			break;
		case 0x7dfd:
			res = 0xff;
			break;
		case 0xfffd:
			res = tsIn(comp->ts,port);
			break;
//		default:
//			printf("ATM2: in %.4X (%.4X)\n",port,ptype);
//			break;
	}
	return res;
}
