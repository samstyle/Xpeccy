#include "../spectrum.h"

#include <stdio.h>
#include <assert.h>

void evoSetVideoMode(ZXComp* comp) {
	int mode = (comp->prt2 & 0x20) | ((comp->prt2 & 0x01) << 1) | (comp->prt1 & 0x07);	// z5.z0.0.b2.b1.b0	b:FF77, z:eff7
	switch (mode) {
		case 0x03: vidSetMode(comp->vid,VID_NORMAL); break;		// common
		case 0x13: vidSetMode(comp->vid,VID_ALCO); break;		// alco 16c
		case 0x23: vidSetMode(comp->vid,VID_HWMC); break;		// zx hardware multicolor
		case 0x02: vidSetMode(comp->vid,VID_ATM_HWM); break;	// atm hardware multicolor
		case 0x00: vidSetMode(comp->vid,VID_ATM_EGA); break;	// atm ega
		case 0x06: vidSetMode(comp->vid,VID_ATM_TEXT); break;	// atm text
		case 0x07: vidSetMode(comp->vid,VID_EVO_TEXT); break;	// pentevo text
		default: vidSetMode(comp->vid,VID_UNKNOWN); break;
	}
}

Z80EX_BYTE evoMRd(ZXComp* comp, Z80EX_WORD adr, int m1) {
	if (m1 && (comp->bdi->fdc->type == FDC_93)) {
		if (comp->dosen && (memGetBankPtr(comp->mem,adr)->type == MEM_RAM) && (comp->prt1 & 0x40)) {
			comp->dosen = 0;
			if (comp->prt0 & 0x10) comp->hw->mapMem(comp);
		}
		if (!comp->dosen && ((adr & 0xff00) == 0x3d00) && (comp->prt0 & 0x10)) {
			comp->dosen = 1;
			comp->hw->mapMem(comp);
		}
	}
	return memRd(comp->mem,adr);
}

void evoMWr(ZXComp* comp, Z80EX_WORD adr, Z80EX_BYTE val) {
	if (comp->evo.evoBF & 4) comp->vid->font[adr & 0x7ff] = val;	// PentEvo: write font byte
	memWr(comp->mem,adr,val);
}

void evoSetBank(ZXComp* comp, int bank, memEntry me) {
	unsigned char page = me.page ^ 0xff;
	if (me.flag & 0x80) {
		if (me.flag & 0x40) {
			if (comp->prt2 & 4) {
				page = (page & 0xf8) | (comp->prt0 & 7);				// mix with b0..2 (7FFD) - 128K mode
			} else {
				page = (page & 0xc0) | (comp->prt0 & 7) | ((comp->prt0 & 0xe0) >> 2);	// mix with b0..2,5..7 (7FFD) - P1024 mode
			}
		} else {
			page = (page & 0x3e) | (comp->dosen ? 1 : 0);				// mix with dosen
		}
	}
	memSetBank(comp->mem,bank,(me.flag & 0x40) ? MEM_RAM : MEM_ROM, page);
}

void evoMapMem(ZXComp* comp) {
	if (comp->prt1 & 0x20) {		// A8.xx77
		int adr = (comp->prt0 & 0x10) ? 4 : 0;
		evoSetBank(comp,MEM_BANK0,comp->memMap[adr]);
		evoSetBank(comp,MEM_BANK1,comp->memMap[adr+1]);
		evoSetBank(comp,MEM_BANK2,comp->memMap[adr+2]);
		evoSetBank(comp,MEM_BANK3,comp->memMap[adr+3]);
	} else {
		comp->dosen = 1;
		memSetBank(comp->mem,MEM_BANK0,MEM_ROM,0xff);
		memSetBank(comp->mem,MEM_BANK1,MEM_ROM,0xff);
		memSetBank(comp->mem,MEM_BANK2,MEM_ROM,0xff);
		memSetBank(comp->mem,MEM_BANK3,MEM_ROM,0xff);
	}
	if (comp->prt2 & 8) memSetBank(comp->mem,MEM_BANK0,MEM_RAM,0x00);	// b3.EFF7: ram0 @ 0x0000 : high priority
}

int evoGetPort(Z80EX_WORD port, int bdiz) {
	if ((port & 0x00ff) == 0x00be) return 0xbe;	// configuration read
	if ((port & 0x00f7) == 0x00f6) return 0xfe;		// FE, A3 = border bright
	if ((port & 0x00ff) == 0x00bf) return 0xbf;
	if ((port & 0x00ff) == 0x00fb) return 0xfb;	// covox
	if (port == 0x7ffd) return 0x7ffd;	// std.mem
	if (port == 0xfadf) return 0xfadf;	// mouse
	if (port == 0xfbdf) return 0xfbdf;
	if (port == 0xffdf) return 0xffdf;
	if (port == 0xbffd) return 0xbffd;	// ay
	if (port == 0xbefd) return 0xbffd;	// WUUUUUT?
	if (port == 0xfffd) return 0xfffd;
	if (((port & 0x1f) == 0x10) || ((port & 0xff) == 0x11) || ((port & 0xff) == 0xc8)) return (port & 0xff);	// nemo
	if (bdiz) {
		if ((port & 0x00ff) == 0x001f) return 0x1f;	// bdi
		if ((port & 0x00ff) == 0x003f) return 0x3f;
		if ((port & 0x00ff) == 0x005f) return 0x5f;
		if ((port & 0x00ff) == 0x007f) return 0x7f;
		if ((port & 0x00ff) == 0x00ff) return 0xff;	// bdi.sys / palette
		if ((port & 0x00ff) == 0x002f) return 0x2f;	// extend bdi ports
		if ((port & 0x00ff) == 0x004f) return 0x4f;
		if ((port & 0x00ff) == 0x006f) return 0x6f;
		if ((port & 0x00ff) == 0x008f) return 0x8f;
		if ((port & 0x00ff) == 0x0077) return 0x77;	// xx77
		if (port == 0xdef7) return 0xdef7;		// nvram
		if (port == 0xbef7) return 0xbef7;
		if ((port & 0x07f7) == 0x07f7) return 0xf7;	// xxF7, A11 = 1:ATM2 / 0:PentEvo
	} else {
		if ((port & 0x00ff) == 0x001f) return 0x1f;	// kempston
		if ((port & 0x00ff) == 0x0057) return 0x57;	// 57,77 - SD-card ports
		if ((port & 0x00ff) == 0x0077) return 0x77;
		if (port == 0xeff7) return 0xeff7;
		if (port == 0xdff7) return 0xdff7;		// nvram
		if (port == 0xbff7) return 0xbff7;
	}
	return 0;
}

void evoOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int bdiz) {
	int adr;
	if (comp->evo.evoBF & 0x01) bdiz = 1;
	int ptype = evoGetPort(port,bdiz);
#ifdef ISDEBUG
//	printf("evo out %.4X (%.4X),%.2X\n",ptype,port,val);
#endif
	switch (ptype) {
/*
		case 0x10:
			ideOut(comp->ide,comp->evo.hiTrig ? 0x11 : 0x10,val,0);
			comp->evo.hiTrig ^= 1;		// change hi-low trigger
			break;
		case 0x11:
			ideOut(comp->ide,0x11,val,0);
			comp->evo.hiTrig = 0;		// next byte is low
			break;
		case 0x30:
		case 0x50:
		case 0x70:
		case 0x90:
		case 0xB0:
		case 0xD0:
		case 0xF0:
		case 0xC8:
			ideOut(comp->ide,ptype & 0xf0,val,0);
			comp->evo.hiTrig = 1;		// next byte is hi (low for in)
#ifdef ISDEBUG
//			comp->flag |= ZX_BREAK;
			//printf("PentEvo: Nemo out %.2X, %.2X\n",ptype,val);
#endif
			break;
*/
		case 0x1f: if (bdiz) bdiOut(comp->bdi,FDC_COM,val); break;
		case 0x3f: if (bdiz) bdiOut(comp->bdi,FDC_TRK,val); break;
		case 0x5f: if (bdiz) bdiOut(comp->bdi,FDC_SEC,val); break;
		case 0x7f: if (bdiz) bdiOut(comp->bdi,FDC_DATA,val); break;

		case 0x2f: if (bdiz) comp->evo.evo2F = val; break;
		case 0x4f: if (bdiz) comp->evo.evo4F = val; break;
		case 0x6f: if (bdiz) comp->evo.evo6F = val; break;
		case 0x8f: if (bdiz) comp->evo.evo8F = val; break;

		case 0x57:
			if (bdiz) {
				comp->sdc->flag &= ~0x03;
				comp->sdc->flag |= val;
				// SDcard CS control
			} else {
				sdcWrite(comp->sdc,val);
				// SDcard data write
			}
			break;
		case 0x77:
			if (bdiz) {
				comp->prt1 = ((port & 0x4000) >> 7) | ((port & 0x0300) >> 3) | (val & 0x0f);	// a14.a9.a8.0.b3.b2.b1.b0
				zxSetFrq(comp,(val & 0x08) ? 14.0 : ((comp->prt2 & 0x10) ? 3.5 : 7.0));
				evoSetVideoMode(comp);
				evoMapMem(comp);
			} else {
				comp->sdc->flag &= ~0x03;
				comp->sdc->flag |= val;
				// SDcard CS control
			}
			break;
		case 0xbf: comp->evo.evoBF = val; break;
		case 0xf7:
			if (!bdiz) break;
			adr = ((comp->prt0 & 0x10) ? 4 : 0) | ((port & 0xc000) >> 14);
			if (port & 0x0800) {
				comp->memMap[adr].flag = val & 0xc0;
				comp->memMap[adr].page = val | 0xc0;
			} else {
				comp->memMap[adr].flag |= 0x40;		// ram
				comp->memMap[adr].page = val;
			}
			evoMapMem(comp);
			break;
		case 0xfb: sdrvOut(comp->sdrv,0xfb,val);
			break;
		case 0xfe:
			comp->vid->nextbrd = (val & 0x07) | (~port & 8);
			if (!comp->vid->border4t) comp->vid->brdcol = comp->vid->nextbrd;
			comp->beeplev = val & 0x10;
			comp->tape->outsig = (val & 0x08) ? 1 : 0;
			break;
		case 0xff:
			if (!bdiz) break;
			bdiOut(comp->bdi,BDI_SYS,val);
			if (~comp->prt1 & 0x80) {
				val ^= 0xff;	// inverse colors
				comp->colMap[comp->vid->brdcol & 0x0f] =		// grbG--RB to -grb-GRB
					(val & 0x03) | ((val & 0x10) >> 2) | ((val & 0xe0) >> 1);
				comp->palchan = 1; // comp->flag |= ZX_PALCHAN;
			}
			break;
		case 0x7ffd:
			if ((comp->prt2 & 4) && (comp->prt0 & 0x20)) break;
			comp->prt0 = val;
			comp->vid->curscr = (val & 0x08) ? 7 : 5;
			evoMapMem(comp);
			break;
		case 0xbef7: if (bdiz) cmsWr(comp,val); break;
		case 0xdef7: if (bdiz) comp->cmos.adr = val; break;
		case 0xdff7:
			if (!bdiz && (comp->prt2 & 0x80)) comp->cmos.adr = val; break;
			break;
		case 0xbff7:
			if (!bdiz && (comp->prt2 & 0x80)) cmsWr(comp,val); break;
			break;
		case 0xeff7: if (bdiz) break;
			comp->prt2 = val;
			zxSetFrq(comp,(comp->prt1 & 0x08) ? 14.0 : (val & 0x08) ? 7.0 : 3.5);
			evoSetVideoMode(comp);
			evoMapMem(comp);
			break;
		case 0xbffd:
		case 0xfffd:
			tsOut(comp->ts,ptype,val);
			break;
		default:
//			if (ideOut(comp->ide,port,val,0)) break;
//			if (gsOut(comp->gs,port,val) == GS_OK) break;
#ifdef ISDEBUG
			printf("PentEvo out %.4X (%.4X.%i),%.2X\n",port,ptype,bdiz,val);
			comp->flag |= ZX_BREAK;
//			assert(0);
#endif
			break;
	}
}

Z80EX_BYTE evoIn(ZXComp* comp, Z80EX_WORD port, int bdiz) {
	Z80EX_BYTE res = 0xff;
	if (comp->evo.evoBF & 1) bdiz = 1;
	int ptype = evoGetPort(port,bdiz);
	switch (ptype) {
/*
		case 0x10:
			ideIn(comp->ide,comp->evo.hiTrig ? 0x10 : 0x11,&res,0);
			comp->evo.hiTrig ^= 1;
			break;
		case 0x11:
			ideIn(comp->ide,0x11,&res,0);
			comp->evo.hiTrig = 1;		// next byte is low
			break;
		case 0x30:
		case 0x50:
		case 0x70:
		case 0x90:
		case 0xB0:
		case 0xD0:
		case 0xF0:
		case 0xC8:
			ideIn(comp->ide,ptype & 0xf0,&res,0);
			comp->evo.hiTrig = 1;		// next byte is low
#ifdef ISDEBUG
			//printf("PentEvo: Nemo in %X\n",ptype);
			//comp->flag |= ZX_BREAK;
#endif
			break;
*/
		case 0x1f: res = bdiz ? bdiIn(comp->bdi,FDC_STATE) : joyInput(comp->joy); break;
		case 0x3f: res = bdiz ? bdiIn(comp->bdi,FDC_TRK) : 0xff; break;
		case 0x5f: res = bdiz ? bdiIn(comp->bdi,FDC_SEC) : 0xff; break;
		case 0x7f: res = bdiz ? bdiIn(comp->bdi,FDC_DATA) : 0xff; break;
		case 0xff: res = bdiz ? bdiIn(comp->bdi,BDI_SYS) : 0xff; break;

		case 0x2f: res = bdiz ? comp->evo.evo2F : 0xff; break;
		case 0x4f: res = bdiz ? comp->evo.evo4F : 0xff; break;
		case 0x6f: res = bdiz ? comp->evo.evo6F : 0xff; break;
		case 0x8f: res = bdiz ? comp->evo.evo8F : 0xff; break;

		case 0x57:			// TODO: sdcard data rd
			res = bdiz ? 0xff : sdcRead(comp->sdc);
			break;
		case 0x77:
			if (bdiz) {
				res = 0xff;
			} else {
				res = 0x02;		// rd only
				if (comp->sdc->image != NULL) res |= 0x01;	// inserted
			}
			break;
		case 0xbe:
			if ((port & 0xf800) == 0x0000) {
				res = comp->memMap[(port & 0x0700) >> 8].page;
			} else {
				switch (port & 0xff00) {
					case 0x0800:
						res = 0;
						for (bdiz = 0; bdiz < 8; bdiz++) {
							res = (res >> 1);
							if (comp->memMap[bdiz].flag & 0x40) res |= 0x80;
						}
						break;
					case 0x0900:
						res = 0;
						for (bdiz = 0; bdiz < 8; bdiz++) {
							res = (res >> 1);
							if (comp->memMap[bdiz].flag & 0x80) res |= 0x80;
						}
						break;
					case 0x0a00: res = comp->prt0; break;
					case 0x0b00: res = comp->prt2; break;
					case 0x0c00: res = comp->prt1 | ((comp->dosen & 1) ? 0x10 : 0x00); break;
					default:
//						printf("PentEvo\tin %.4X.%i\n",port,bdiz);
//						assert(0);
						break;
				}
			}
			break;
		case 0xbf: res = comp->evo.evoBF; break;
		case 0xfe: res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00); break;
		case 0xbef7: res = (bdiz) ? cmsRd(comp) : 0xff; break;
		case 0xbff7: res = (!bdiz && (comp->prt2 & 0x80)) ? cmsRd(comp) : 0xff; break;
		case 0xfffd: res = tsIn(comp->ts,ptype); break;
		case 0xfadf: res = comp->mouse->enable ? comp->mouse->buttons : 0xff; break;
		case 0xfbdf: res = comp->mouse->enable ? comp->mouse->xpos : 0xff; break;
		case 0xffdf: res = comp->mouse->enable ? comp->mouse->ypos : 0xff; break;
		default:
//			if (ideIn(comp->ide,port,&res,comp->dosen & 1)) break;
//			if (gsIn(comp->gs,port,&res) == GS_OK) break;
#ifdef ISDEBUG
			printf("Pentevo: in %.4X (%.4X.%i)\n",port,ptype,bdiz);
			comp->flag |= ZX_BREAK;
//			assert(0);
#endif
			break;
	}
	return res;
}
