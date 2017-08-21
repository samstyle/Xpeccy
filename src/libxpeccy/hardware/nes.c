#include "hardware.h"

#include <stdio.h>

// NOTE: NES is fukkeen trash. why did i start this?
// NOTE: Cartridges have potentially 255+ mappers

// PPU reads byte (except palette)
unsigned char nes_ppu_ext_rd(unsigned short adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	unsigned char res;
	if (adr & 0x2000) {	// nametable
//		adr &= comp->slot->ntmask;
//		adr |= comp->slot->ntorsk;
		adr = nes_nt_vadr(comp->slot, adr);
		res = comp->vid->ppu->mem[adr];
	} else {
		if (comp->slot->chrrom) {
			res = sltRead(comp->slot, SLT_CHR, adr);
		} else {
			res = comp->vid->ppu->mem[adr];
		}
	}
	return res;
}

// PPU writes byte (except palette)
void nes_ppu_ext_wr(unsigned short adr, unsigned char val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	if (adr & 0x2000) {
//		adr &= comp->slot->ntmask;
//		adr |= comp->slot->ntorsk;
		adr = nes_nt_vadr(comp->slot, adr);
		comp->vid->ppu->mem[adr] = val;
	} else {
		if (comp->slot->chrrom) {
			// CHR-RAM?
		} else {
			comp->vid->ppu->mem[adr & 0x1fff] = val;
		}
	}
}

// APU reads byte (DMC channel)
unsigned char nes_apu_ext_rd(unsigned short adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	return comp->hw->mrd(comp, adr, 0);
}

void nesReset(Computer* comp) {
	comp->vid->ppu->mrd = nes_ppu_ext_rd;
	comp->vid->ppu->mwr = nes_ppu_ext_wr;
	comp->vid->ppu->data = comp;
	ppuReset(comp->vid->ppu);
	vidSetMode(comp->vid, VID_NES);
/*
	comp->slot->memMap[0] = 0;	// prg 4x8K pages
	comp->slot->memMap[1] = 1;
	comp->slot->memMap[2] = comp->slot->prglast - 1;
	comp->slot->memMap[3] = comp->slot->prglast;
	for (int i = 0; i < 8; i++)	// chr 8x1K pages
		comp->slot->chrMap[i] = i;
*/
}

unsigned char nesMMrd(unsigned short adr, void* data) {
	Computer* comp = (Computer*)data;
	nesAPU* apu = comp->nesapu;
	unsigned char* mem = comp->mem->ramData;
	unsigned char res = 0xff;
	adr &= 0x7fff;
	switch(adr & 0xe000) {
		case 0x0000:		// 0000..1fff : SRAM 2K + mirrors
			res = mem[adr & 0x7ff];
			break;
		case 0x2000:		// 2000..3fff : 8 ppu registers + mirrors
			res = ppuRead(comp->vid->ppu, adr & 7);
			break;
		case 0x4000:		// 4000..5fff : IO mapping
			if (adr & 0x1000) {
				res = sltRead(comp->slot, SLT_EXT, adr);
			} else {
				switch (adr & 0x0fff) {
					case 0x0015:
						res = 0;
						if (apu->ch0.len && apu->ch0.en) res |= 0x01;
						if (apu->ch1.len && apu->ch1.en) res |= 0x02;
						if (apu->cht.len && apu->cht.en) res |= 0x04;
						if (apu->chn.len && apu->chn.en) res |= 0x08;
						if (apu->chd.len && apu->chd.en) res |= 0x10;
						if (apu->firq) res |= 0x40;
						break;
					case 0x0016:		// joystick 1
						res = comp->nes.priJoy & 1;
						comp->nes.priJoy >>= 1;
						break;
					case 0x0017:		// joystick 2
						res = comp->nes.secJoy & 1;
						comp->nes.secJoy >>= 1;
						break;
				}
			}
			break;
		case 0x6000:		// 6000..7fff : slot ram
			res = sltRead(comp->slot, SLT_RAM, adr);
			break;
	}
	return  res;
}

// taked from nesdev wiki
static int apuNoisePer[16] = {0x02,0x04,0x08,0x10,0x20,0x30,0x40,0x50,0x65,0x7F,0xBE,0xFE,0x17D,0x1FC,0x3F9,0x7F2};
//static int apuPcmPer[16] = {428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106,  84,  72,  54};		// this is in CPU ticks
static int apuPcmPer[16] = {54, 48, 42, 40, 36, 32, 28, 27, 24, 20, 18, 16, 13, 11, 9, 7};				// this is APU ticks (CPU/8)
static int apuLenPAL[8] = {5,10,20,40,80,30,7,13};
static int apuLenNTSC[8] = {6,12,24,48,96,36,8,16};
static int apuLenGeneral[16] = {127,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

int apuGetLC(unsigned char val) {
	int len;
	switch (val & 0x88) {
		case 0x00: len = apuLenPAL[(val >> 4) & 7]; break;
		case 0x80: len = apuLenNTSC[(val >> 4) & 7]; break;
		default: len = apuLenGeneral[(val >> 4) & 15]; break;
	}
	return len;
}

// write @ page 4000..7fff
// 4000..4fff : IO block
// 5000..5fff : ?
// 6000..7fff : cartrige ram
void nesMMwr(unsigned short adr, unsigned char val, void* data) {
	Computer* comp = (Computer*)data;
	nesAPU* apu = comp->nesapu;
	adr &= 0x7fff;
	switch (adr & 0xe000) {
		case 0x0000:			// 0000..1fff : 2K SRAM + mirrors
			comp->mem->ramData[adr & 0x7ff] = val;
			break;
		case 0x2000:			// 2000..3fff : 8 PPU registers + mirrors
			ppuWrite(comp->vid->ppu, adr & 7, val);
			break;
		case 0x4000:			// 4000..5fff : IO mappings
			if (adr > 0x401f) {	// 4020+ -> cartrige IO
				sltWrite(comp->slot, SLT_EXT, adr, val);
			} else {
			switch(adr) {
				case 0x4000:
					apu->ch0.duty = (val >> 6) & 3;
					apu->ch0.env = (val & 0x10) ? 0 : 1;
					apu->ch0.elen = (val & 0x20) ? 0 : 1;
					if (apu->ch0.env) {
						//printf("ch0 env %.2X\n",val);
						apu->ch0.eper = ((val & 15) + 1);
						apu->ch0.ecnt = apu->ch0.eper;
						apu->ch0.vol = 0x0f;
					} else {
						apu->ch0.vol = val & 15;
					}
					apuToneDuty(&apu->ch0);
					break;
				case 0x4001:
	//				printf("ch0 sweep %.2X\n",val);
					apu->ch0.sweep = (val & 0x80) ? 1 : 0;
					apu->ch0.sper = ((val >> 4) & 7) + 1;
					apu->ch0.sdir = (val & 0x08) ? 1 : 0;
					apu->ch0.sshi = val & 7;
					break;
				case 0x4002:
					apu->ch0.hper &= 0x700;
					apu->ch0.hper |= (val & 0xff);
					apuToneDuty(&apu->ch0);
					break;
				case 0x4003:
					//printf("wr 4003,%.2X\n",val);
					apu->ch0.hper &= 0xff;
					apu->ch0.hper |= ((val << 8) & 0x0700);
					switch (val & 0x88) {
						case 0x00: apu->ch0.len = apuLenPAL[(val >> 4) & 7]; break;
						case 0x80: apu->ch0.len = apuLenNTSC[(val >> 4) & 7]; break;
						default: apu->ch0.len = apuLenGeneral[(val >> 4) & 15]; break;
					}
					apuToneDuty(&apu->ch0);
					apu->ch0.pcnt = 0;
					apu->ch0.ecnt = 0;
					//printf("CH0 len = %i, p0 = %i, p1 = %i\n", apu->ch0.len, apu->ch0.per0, apu->ch0.per1);
					break;
				// ch1 : tone 1
				case 0x4004:
					apu->ch1.duty = (val >> 6) & 3;
					apu->ch1.env = (val & 0x10) ? 0 : 1;
					apu->ch1.elen = (val & 0x20) ? 0 : 1;
					if (apu->ch1.env) {
						apu->ch1.eper = ((val & 15) + 1);
						apu->ch1.ecnt = apu->ch1.eper;
						apu->ch1.vol = 0x0f;
					} else {
						apu->ch1.vol = val & 15;
					}
					apuToneDuty(&apu->ch1);
					break;
				case 0x4005:
					apu->ch1.sweep = (val & 0x80) ? 1 : 0;
					apu->ch1.sper = ((val >> 4) & 7) + 1;
					apu->ch1.sdir = (val & 0x08) ? 1 : 0;
					apu->ch1.sshi = val & 7;
					break;
				case 0x4006:
					apu->ch1.hper &= 0x700;
					apu->ch1.hper |= val;
					apuToneDuty(&apu->ch1);
					break;
				case 0x4007:
					apu->ch1.hper &= 0xff;
					apu->ch1.hper |= ((val << 8) & 0x0700);
					apu->ch1.len = apuGetLC(val);
					apuToneDuty(&apu->ch1);
					apu->ch1.pcnt = 0;
					apu->ch0.ecnt = 0;
					break;
				// ch2 : triangle
				case 0x4008:
					apu->cht.elen = (val & 0x80) ? 0 : 1;
					apu->cht.lcnt = (val & 0x7f);
					break;
				case 0x4009:
					break;
				case 0x400a:
					apu->cht.hper &= 0x700;
					apu->cht.hper |= val & 0xff;
					apu->cht.pcnt = apu->cht.hper;
					break;
				case 0x400b:
					apu->cht.hper &= 0xff;
					apu->cht.hper |= (val << 8) & 0x700;
					apu->cht.pcnt = 0;
					apu->cht.len = apuGetLC(val);
					apu->cht.vol = 8;
					apu->cht.dir = 1;
					break;
				// ch3 : noise
				case 0x400c:
					apu->chn.elen = (val & 0x20) ? 0 : 1;
					apu->chn.env = (val & 0x10) ? 0 : 1;
					if (apu->chn.env) {
						apu->chn.eper = (val & 15) + 1;
						apu->chn.ecnt = apu->chn.eper;
						apu->chn.vol = 0x0f;
					} else {
						apu->chn.vol = val & 15;
					}
					break;
				case 0x400d:
					break;
				case 0x400e:
					apu->chn.hper = apuNoisePer[val & 15];
					// b7:loop noise (short generator)
					break;
				case 0x400f:
					apu->chn.len = apuGetLC(val);
					break;
				// ch4 : dmc
				case 0x4010:
					apu->chd.env = (val & 0x80) ? 1 : 0;		// IRQ enable
					apu->chd.elen = (val & 0x40) ? 1 : 0;		// loop
					apu->chd.hper = apuPcmPer[val & 0x0f];		// period
					apu->chd.pcnt = 0;
					break;
				case 0x4011:
					apu->chd.vol = val & 0x7f;
					break;
				case 0x4012:
					apu->chd.sadr = 0xc000 | ((val << 6) & 0x3fc0);
					apu->chd.cadr = apu->chd.sadr;
					break;
				case 0x4013:
					apu->chd.lcnt = ((val << 4) & 0x0ff0) | 1;	// this is length in bytes
					apu->chd.len = apu->chd.lcnt;
					break;
				// ...
				case 0x4014:		// OAMDMA
					adr = (val << 8);
					do {
						comp->vid->ppu->oam[comp->vid->ppu->oamadr & 0xff] = memRd(comp->mem, adr);
						comp->vid->ppu->oamadr++;
						adr++;
					} while (adr & 0xff);
					comp->cpu->t += 0x201;	// DMA eats 512+1 cpu ticks, cpu is halted during operation
					break;
				case 0x4015:
					apu->ch0.en = (val & 0x01) ? 1 : 0;
					apu->ch1.en = (val & 0x02) ? 1 : 0;
					apu->cht.en = (val & 0x04) ? 1 : 0;
					apu->chn.en = (val & 0x08) ? 1 : 0;
					apu->chd.en = (val & 0x10) ? 1 : 0;
					break;
				case 0x4016:
					if (val & 1) {		// b0: 0-1-0 = reload gamepads state
						comp->nes.priJoy = comp->nes.priPadState;
						comp->nes.secJoy = comp->nes.secPadState;
					}
					break;
				case 0x4017:
					apu->irqen = (val & 0x80) ? 0 : 1;
					apu->step5 = (val & 0x40) ? 1 : 0;
					break;
				default:
					//printf("write %.4X,%.2X\n",adr,val);
					break;
			}
			}
			break;
		case 0x6000:			// 6000..7fff : slot RAM
			sltWrite(comp->slot, SLT_RAM, adr, val);
			break;
	}
}

unsigned char nesSLrd(unsigned short adr, void* data) {
	xCartridge* slot = (xCartridge*)data;
	return sltRead(slot, SLT_PRG, adr);
}

void nesSLwr(unsigned short adr, unsigned char val, void* data) {
	xCartridge* slot = (xCartridge*)data;
	sltWrite(slot, SLT_PRG, adr, val);
}

void nesMaper(Computer* comp) {
	memSetBank(comp->mem, MEM_BANK0, MEM_EXT, 0, nesMMrd, nesMMwr, comp);
	memSetBank(comp->mem, MEM_BANK1, MEM_EXT, 1, nesMMrd, nesMMwr, comp);
	memSetBank(comp->mem, MEM_BANK2, MEM_SLOT, 0, nesSLrd, nesSLwr, comp->slot);
	memSetBank(comp->mem, MEM_BANK3, MEM_SLOT, 1, nesSLrd, nesSLwr, comp->slot);
}

void nesSync(Computer* comp, int ns) {
	if (comp->vid->ppu->vblstrb && comp->vid->ppu->inten) {	// @ start of VBlank
		comp->vid->ppu->vblstrb = 0;
		comp->cpu->intrq |= MOS6502_INT_NMI;		// request NMI...
	}

	apuSync(comp->nesapu, ns);

	// MMC3 irq counter triggered @ HBlank (?)
	if ((comp->vid->hbstrb) && (comp->vid->ray.y < 240)) {
		comp->vid->hbstrb = 0;
		if (comp->slot->irqrl) {		// reload counter
			comp->slot->irqrl = 0;
			comp->slot->icnt = comp->slot->ival;
		} else {
			comp->slot->icnt--;
			if (comp->slot->icnt == 0) {
				comp->slot->icnt = comp->slot->reg03;
				comp->slot->irq = comp->slot->irqen;
			}
		}
	}

	int irq = comp->nesapu->firq | comp->nesapu->dirq | comp->slot->irq;		// external irq signals
	comp->nesapu->firq = 0;
	comp->nesapu->dirq = 0;
	comp->slot->irq = 0;

	if (irq && !(comp->cpu->f & MFI))
		comp->cpu->intrq |= MOS6502_INT_BRK;
}

unsigned char nesMemRd(Computer* comp, unsigned short adr, int m1) {
	return memRd(comp->mem, adr);
}

void nesMemWr(Computer* comp, unsigned short adr, unsigned char val) {
	memWr(comp->mem, adr, val);
}

// keypress

typedef struct {
	signed int id;
	int mask;
} nesKey;

static nesKey nesKeyMap[8] = {
	{XKEY_Z,1},		// A
	{XKEY_X,2},		// B
	{XKEY_SPACE,4},		// select
	{XKEY_ENTER,8},		// start
	{XKEY_UP,16},		// up
	{XKEY_DOWN,32},		// down
	{XKEY_LEFT,64},		// left
	{XKEY_RIGHT,128}	// right
};

int nesGetInputMask(signed int keyid) {
	int idx = 0;
	int mask = 0;
	while (idx < 8) {
		if (nesKeyMap[idx].id == keyid) {
			mask = nesKeyMap[idx].mask;
		}
		idx++;
	}
	return mask;
}

static char nesBgOn[] = " BG layer on ";
static char nesBgOff[] = " BG layer off ";
static char nesSpOn[] = " SPR layer on ";
static char nesSpOff[] = " SPR layer off ";
static char nesPAL[] = " PAL ";
static char nesNTSC[] = " NTSC ";
static char nesC0off[] = " CH0 off ";
static char nesC0on[] = " CH0 on ";
static char nesC1off[] = " CH1 off ";
static char nesC1on[] = " CH1 on ";
static char nesC2off[] = " CH2 off ";
static char nesC2on[] = " CH2 on ";
static char nesC3off[] = " CH3 off ";
static char nesC3on[] = " CH3 on ";
static char nesC4off[] = " CH4 off ";
static char nesC4on[] = " CH4 on ";

void nes_keyp(Computer* comp, keyEntry ent) {
	int mask = nesGetInputMask(ent.key);
	comp->nes.priPadState |= mask;
	switch (ent.key) {
		case XKEY_0:
			comp->nes.pal ^= 1;
			comp->msg = comp->nes.pal ? nesPAL : nesNTSC;
			compUpdateTimings(comp);
			break;
		case XKEY_1:
			comp->vid->ppu->bglock ^= 1;
			comp->msg = comp->vid->ppu->bglock ? nesBgOff : nesBgOn;
			break;
		case XKEY_2:
			comp->vid->ppu->splock ^= 1;
			comp->msg = comp->vid->ppu->splock ? nesSpOff : nesSpOn;
			break;
		case XKEY_3:
			comp->nesapu->ch0.off ^= 1;
			comp->msg = comp->nesapu->ch0.off ? nesC0off : nesC0on;
			break;
		case XKEY_4:
			comp->nesapu->ch1.off ^= 1;
			comp->msg = comp->nesapu->ch1.off ? nesC1off : nesC1on;
			break;
		case XKEY_5:
			comp->nesapu->cht.off ^= 1;
			comp->msg = comp->nesapu->cht.off ? nesC2off : nesC2on;
			break;
		case XKEY_6:
			comp->nesapu->chn.off ^= 1;
			comp->msg = comp->nesapu->chn.off ? nesC3off : nesC3on;
			break;
		case XKEY_7:
			comp->nesapu->chd.off ^= 1;
			comp->msg = comp->nesapu->chd.off ? nesC4off : nesC4on;
			break;
	}
}

void nes_keyr(Computer* comp, keyEntry ent) {
	int mask = nesGetInputMask(ent.key);
	if (mask == 0) return;
	comp->nes.priPadState &= ~mask;
}

sndPair nes_vol(Computer* comp) {
	sndPair vol = apuVolume(comp->nesapu);
	return  vol;
}
