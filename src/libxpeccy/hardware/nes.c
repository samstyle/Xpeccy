#include "hardware.h"

#include <stdio.h>

// NOTE: NES is fukkeen trash. why did i start this?
// NOTE: Cartridges have potentially 255+ mappers

// PPU reads byte (except palette)
int nes_ppu_ext_rd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	int res = -1;
	adr = nes_nt_vadr(comp->slot, adr);
	sltChecker(comp->slot, adr);
	if (adr & 0x2000) {	// nametable
		res = comp->vid->ram[adr] & 0xff;
	} else {
		if (comp->slot->chrrom) {
			res = sltRead(comp->slot, SLT_CHR, adr) & 0xff;
		} else {
			res = comp->vid->ram[adr] & 0xff;
		}
	}
	return res;
}

// PPU writes byte (except palette)
void nes_ppu_ext_wr(int adr, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	adr = nes_nt_vadr(comp->slot, adr);
	sltChecker(comp->slot, adr);
	if (adr & 0x2000) {
		comp->vid->ram[adr] = val;
	} else {
		if (comp->slot->chrrom) {
			// CHR-RAM?
		} else {
			comp->vid->ram[adr & 0x1fff] = val & 0xff;
		}
	}
}

// APU reads byte (DMC channel)
int nes_apu_ext_rd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	return comp->hw->mrd(comp, adr, 0);
}

void nesReset(Computer* comp) {
	comp->vid->mrd = nes_ppu_ext_rd;
	comp->vid->mwr = nes_ppu_ext_wr;
	comp->vid->data = comp;
	comp->slot->reg00 = 0x0c;	// MMC1
	comp->slot->reg06 = 0;
	comp->slot->reg07 = 1;
	apuReset(comp->nesapu);
	ppuReset(comp->vid);
	vidSetMode(comp->vid, VID_NES);
}

int nesMMrd(int adr, void* data) {
	Computer* comp = (Computer*)data;
	nesAPU* apu = comp->nesapu;
	unsigned char* mem = comp->mem->ramData;
	int res = -1;
	adr &= 0x7fff;
	switch(adr & 0xe000) {
		case 0x0000:		// 0000..1fff : SRAM 2K + mirrors
			res = mem[adr & 0x7ff];
			break;
		case 0x2000:		// 2000..3fff : 8 ppu registers + mirrors
			res = ppuRead(comp->vid, adr & 7);
			break;
		case 0x4000:		// 4000..5fff : IO mapping
			if (adr > 0x401f) {
				res = sltRead(comp->slot, SLT_EXT, adr);
			} else {
				switch (adr & 0x1f) {
					case 0x15:
						res = 0;
						if (apu->ch0.len) res |= 0x01;
						if (apu->ch1.len) res |= 0x02;
						if (apu->cht.len) res |= 0x04;
						if (apu->chn.len) res |= 0x08;
						if (apu->chd.len) res |= 0x10;
						if (apu->firq) res |= 0x40;
						if (apu->dirq) res |= 0x80;
						apu->firq = 0;
						break;
					case 0x16:		// joystick 1
						res = comp->nes.priJoy & 1;
						comp->nes.priJoy >>= 1;
						break;
					case 0x17:		// joystick 2
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

// write @ page 4000..7fff
// 4000..4fff : IO block
// 5000..5fff : ?
// 6000..7fff : cartrige ram
void nesMMwr(int adr, int val, void* data) {
	Computer* comp = (Computer*)data;
	nesAPU* apu = comp->nesapu;
	adr &= 0x7fff;
	switch (adr & 0xe000) {
		case 0x0000:			// 0000..1fff : 2K SRAM + mirrors
			comp->mem->ramData[adr & 0x7ff] = val & 0xff;
			break;
		case 0x2000:			// 2000..3fff : 8 PPU registers + mirrors
			ppuWrite(comp->vid, adr & 7, val);
			break;
		case 0x4000:			// 4000..5fff : IO mappings
			if (adr < 0x4014) {	// 4000..4013 : APU
				apuWrite(comp->nesapu, adr & 0x1f, val);
			} else if (adr < 0x4020) {
				switch (adr & 0x1f) {
					case 0x14:		// OAMDMA
						adr = (val << 8);
						do {
							comp->vid->oam[comp->vid->oamadr & 0xff] = memRd(comp->mem, adr);
							comp->vid->oamadr++;
							adr++;
						} while (adr & 0xff);
						comp->cpu->t += 0x201;	// DMA eats 512+1+1 cpu ticks, cpu is halted during operation
						break;
					case 0x15:
						apu->ch0.en = (val & 0x01) ? 1 : 0; if (!apu->ch0.en) apu->ch0.len = 0;
						apu->ch1.en = (val & 0x02) ? 1 : 0; if (!apu->ch1.en) apu->ch1.len = 0;
						apu->cht.en = (val & 0x04) ? 1 : 0; if (!apu->cht.en) apu->cht.len = 0;
						apu->chn.en = (val & 0x08) ? 1 : 0; if (!apu->chn.en) apu->chn.len = 0;
						apu->chd.en = (val & 0x10) ? 1 : 0; if (!apu->ch0.en) apu->chd.len = 0;
						apu->dirq = 0;
						break;
					case 0x16:
						if (val & 1) {		// b0: 0-1-0 = reload gamepads state
							comp->nes.priJoy = comp->nes.priPadState;
							comp->nes.secJoy = comp->nes.secPadState;
						}
						break;
					case 0x17:
						apu->irqen = (val & 0x80) ? 0 : 1;
						apu->step5 = (val & 0x40) ? 1 : 0;
						apu->tstp = 0;
						break;
					default:
						//printf("write %.4X,%.2X\n",adr,val);
						break;
				}
			} else {		// 4020+ -> cartrige IO
				sltWrite(comp->slot, SLT_EXT, adr, val);
			}
			break;
		case 0x6000:			// 6000..7fff : slot RAM
			sltWrite(comp->slot, SLT_RAM, adr, val);
			break;
	}
}

int nesSLrd(int adr, void* data) {
	xCartridge* slot = (xCartridge*)data;
	return sltRead(slot, SLT_PRG, adr);
}

void nesSLwr(int adr, int val, void* data) {
	xCartridge* slot = (xCartridge*)data;
	sltWrite(slot, SLT_PRG, adr, val);
}

void nesMaper(Computer* comp) {
	memSetBank(comp->mem, 0x00, MEM_EXT, 0, MEM_16K, nesMMrd, nesMMwr, comp);
	memSetBank(comp->mem, 0x40, MEM_EXT, 1, MEM_16K, nesMMrd, nesMMwr, comp);
	memSetBank(comp->mem, 0x80, MEM_SLOT, 0, MEM_16K, nesSLrd, nesSLwr, comp->slot);
	memSetBank(comp->mem, 0xc0, MEM_SLOT, 1, MEM_16K, nesSLrd, nesSLwr, comp->slot);
}

void nesSync(Computer* comp, int ns) {
	if (comp->vid->vbstrb && comp->vid->inten) {	// @ start of VBlank
		comp->vid->vbstrb = 0;
		comp->cpu->intrq |= MOS6502_INT_NMI;		// request NMI...
	}

	apuSync(comp->nesapu, ns);
	// irq
	int irq = comp->nesapu->firq | comp->nesapu->dirq | comp->slot->irq;		// external irq signals
	comp->nesapu->firq = 0;
	comp->nesapu->dirq = 0;
	comp->slot->irq = 0;
	if (irq && !(comp->cpu->f & MFI))
		comp->cpu->intrq |= MOS6502_INT_IRQ;
}

extern int res4;

int nesMemRd(Computer* comp, int adr, int m1) {
	vidSync(comp->vid, (comp->cpu->t - res4) * comp->nsPerTick);
	res4 = comp->cpu->t;
	return memRd(comp->mem, adr);
}

void nesMemWr(Computer* comp, int adr, int val) {
	vidSync(comp->vid, (comp->cpu->t - res4) * comp->nsPerTick);
	res4 = comp->cpu->t;
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
static char nesDendy[] = " Dendy ";
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
			switch(comp->nes.type) {
				case NES_NTSC:
					comp->nes.type = NES_PAL;
					comp->msg = nesPAL;
					break;
				case NES_PAL:
					comp->nes.type = NES_DENDY;
					comp->msg = nesDendy;
					break;
				default:
					comp->nes.type = NES_NTSC;
					comp->msg = nesNTSC;
					break;
			}
			compUpdateTimings(comp);
			break;
		case XKEY_1:
			comp->vid->bgblock ^= 1;
			comp->msg = comp->vid->bgblock ? nesBgOff : nesBgOn;
			break;
		case XKEY_2:
			comp->vid->sprblock ^= 1;
			comp->msg = comp->vid->sprblock ? nesSpOff : nesSpOn;
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

sndPair nes_vol(Computer* comp, sndVolume* sv) {
	return apuVolume(comp->nesapu);
}
