#include "hardware.h"

#include <stdio.h>

static vLayout nesNTSCLay = {{341,262},{1,1},{85,22},{256,240},{0,0},64};
static vLayout nesPALLay = {{341,312},{0,0},{85,72},{256,240},{0,0},64};

// NOTE: NES is fukkeen trash. why did i start this?
// NOTE: Cartridges have potentially 255+ mappers

// NES...
// NTSC	89342 dots/f	60fps	5360520 dot/sec	186.55 ns/dot
// PAL	106392 dots/f	50fps	5319600 dot/sec	188 ns/dot
// ~185 ns/dot	PAL:x3.2=592ns/tick	NTSC:x3=555ns/tick
// ~190 ns/dot	PAL:608 ns/tick		NTSC:570ns/tick (570.837)
// NTSC:base/14915
// PAL:base/12430
// NTSC: 113.5T/line

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
		comp->vid->ram[adr] = val & 0xff;
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
	switch (comp->slot->core->id) {
		case MAP_NES_MMC1:
			comp->slot->reg00 = 0x0c;	// MMC1
			comp->slot->reg06 = 0;
			comp->slot->reg07 = 1;
			break;
		case MAP_NES_063:
			comp->slot->reg00 = 0xff;
			comp->slot->reg01 = 0xff;
			break;
	}
	apuReset(comp->nesapu);
	ppuReset(comp->vid);
	vidSetMode(comp->vid, VID_NES);
}

// 0000..1fff : ram 2K + mirrors

int nes_ram_rd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	return comp->mem->ramData[adr & 0x7ff] & 0xff;
}

void nes_ram_wr(int adr, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	comp->mem->ramData[adr & 0x7ff] = val & 0xff;
}

// 2000..3fff : 8 PPU registers + mirrors

int nes_ppu_rd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	return ppuRead(comp->vid, adr & 7);
}

void nes_ppu_wr(int adr, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	ppuWrite(comp->vid, adr & 7, val);
}

// 4000..5fff : IO

int nes_io_rd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	int res = 0xff;
	if (adr > 0x401f) {
		res = sltRead(comp->slot, SLT_EXT, adr);
	} else {
		switch (adr & 0x1f) {
			case 0x15:
				res = 0;
				if (comp->nesapu->ch0.len) res |= 0x01;
				if (comp->nesapu->ch1.len) res |= 0x02;
				if (comp->nesapu->cht.len) res |= 0x04;
				if (comp->nesapu->chn.len) res |= 0x08;
				if (comp->nesapu->chd.len) res |= 0x10;
				if (comp->nesapu->firq) res |= 0x40;
				if (comp->nesapu->dirq) res |= 0x80;
				comp->nesapu->firq = 0;
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
	return res;
}

void nes_io_wr(int adr, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	if (adr < 0x4014) {	// 4000..4013 : APU
		apuWrite(comp->nesapu, adr & 0x1f, val);
	} else if (adr < 0x4020) {
		switch (adr & 0x1f) {
			case 0x14:		// OAMDMA
				adr = (val << 8);
				do {
					comp->vid->oam[comp->vid->oamadr & 0xff] = memRd(comp->mem, adr) & 0xff;
					comp->vid->oamadr++;
					adr++;
				} while (adr & 0xff);
				comp->cpu->t += 0x202;	// DMA eats 512+1+1 cpu ticks, cpu is halted during operation
				break;
			case 0x15:
				comp->nesapu->ch0.en = (val & 0x01) ? 1 : 0;
				if (!comp->nesapu->ch0.en) comp->nesapu->ch0.len = 0;
				comp->nesapu->ch1.en = (val & 0x02) ? 1 : 0;
				if (!comp->nesapu->ch1.en) comp->nesapu->ch1.len = 0;
				comp->nesapu->cht.en = (val & 0x04) ? 1 : 0;
				if (!comp->nesapu->cht.en) comp->nesapu->cht.len = 0;
				comp->nesapu->chn.en = (val & 0x08) ? 1 : 0;
				if (!comp->nesapu->chn.en) comp->nesapu->chn.len = 0;
				comp->nesapu->chd.en = (val & 0x10) ? 1 : 0;
				if (!comp->nesapu->ch0.en) comp->nesapu->chd.len = 0;
				comp->nesapu->dirq = 0;
				break;
			case 0x16:
				if (val & 1) {		// b0: 0-1-0 = reload gamepads state
					comp->nes.priJoy = comp->nes.priPadState;
					comp->nes.secJoy = comp->nes.secPadState;
				}
				break;
			case 0x17:
				comp->nesapu->irqen = (val & 0x80) ? 0 : 1;
				comp->nesapu->step5 = (val & 0x40) ? 1 : 0;
				comp->nesapu->tstp = 0;
				break;
			default:
				//printf("write %.4X,%.2X\n",adr,val);
				break;
		}
	} else {		// 4020+ -> cartrige IO
		sltWrite(comp->slot, SLT_EXT, adr, val);
	}
}

// 6000..7fff : cartrige ram

int nes_slot_ram_rd(int adr, void* data) {
	xCartridge* slot = (xCartridge*)data;
	return sltRead(slot, SLT_RAM, adr);
}

void nes_slot_ram_wr(int adr, int val, void* data) {
	xCartridge* slot = (xCartridge*)data;
	sltWrite(slot, SLT_RAM, adr, val);
}

// 8000..ffff :  cartrige prg rom

int nes_slot_prg_rd(int adr, void* data) {
	xCartridge* slot = (xCartridge*)data;
	return sltRead(slot, SLT_PRG, adr);
}

void nes_slot_prg_wr(int adr, int val, void* data) {
	xCartridge* slot = (xCartridge*)data;
	sltWrite(slot, SLT_PRG, adr, val);
}

void nesMaper(Computer* comp) {
	memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_8K, nes_ram_rd, nes_ram_wr, comp);	// 0000..1FFF ram
	memSetBank(comp->mem, 0x20, MEM_IO, 0, MEM_8K, nes_ppu_rd, nes_ppu_wr, comp);	// 2000..3FFF ppu
	memSetBank(comp->mem, 0x40, MEM_IO, 1, MEM_8K, nes_io_rd, nes_io_wr, comp);	// 4000..5FFF io
	memSetBank(comp->mem, 0x60, MEM_SLOT, 0, MEM_8K, nes_slot_ram_rd, nes_slot_ram_wr, comp->slot);	// 6000..7FFF slot (ram)
	memSetBank(comp->mem, 0x80, MEM_SLOT, 0, MEM_32K, nes_slot_prg_rd, nes_slot_prg_wr, comp->slot);// 8000..FFFF slot (prg)
}

void nesSync(Computer* comp, int ns) {
	if (comp->vid->vblank && !comp->nes.vblank && comp->vid->inten) {	// @ start of VBlank
		// comp->vid->vbstrb = 0;
		comp->cpu->intrq |= MOS6502_INT_NMI;				// request NMI...
	}
	comp->nes.vblank = comp->vid->vblank;

	apuSync(comp->nesapu, ns);
	// irq
	int irq = comp->nesapu->firq | comp->nesapu->dirq | comp->slot->irq;		// external irq signals
	comp->nesapu->firq = 0;
	comp->nesapu->dirq = 0;
	comp->slot->irq = 0;
	if (irq && !comp->nes.irq)
		comp->cpu->intrq |= MOS6502_INT_IRQ;
	comp->nes.irq = irq ? 1 : 0;
}

extern int res4;

int nesMemRd(Computer* comp, int adr, int m1) {
//	vidSync(comp->vid, (comp->cpu->t - res4) * comp->nsPerTick);
//	res4 = comp->cpu->t;
	return memRd(comp->mem, adr);
}

void nesMemWr(Computer* comp, int adr, int val) {
//	vidSync(comp->vid, (comp->cpu->t - res4) * comp->nsPerTick);
//	res4 = comp->cpu->t;
	memWr(comp->mem, adr, val);
}

// base frq (21.477MHz | 26.602MHz)
// cpu frq (base:12 | base:16)
// ppu frq (base:4 | base:5)
// apu frame (60Hz | 50Hz)
// apu frq = cpu / 2
// smallest wave period = cpu / 16

void nes_init(Computer* comp) {
	int perNoTurbo;
	switch(comp->nes.type) {
		case NES_PAL:
			comp->fps = 50;
			comp->cpuFrq = 1.66;
			perNoTurbo = 1e3 / comp->cpuFrq;		// ~601
			vidSetLayout(comp->vid, &nesPALLay);
			comp->vid->vbsline = 241;
			comp->vid->vbrline = 311;
			vidUpdateTimings(comp->vid, perNoTurbo / 3.2);		// 16 ticks = 5 dots
			comp->nesapu->wdiv = 3107;		// 5/6 = 3107? or 166/179 = 3458
			break;
		case NES_NTSC:
			comp->fps = 60;
			comp->cpuFrq = 1.79;
			perNoTurbo = 1e3 / comp->cpuFrq;		// ~559
			vidSetLayout(comp->vid, &nesNTSCLay);
			comp->vid->vbsline = 241;
			comp->vid->vbrline = 261;
			vidUpdateTimings(comp->vid, perNoTurbo / 3);		// 15 ticks = 5 dots
			comp->nesapu->wdiv = 3729;
			break;
		default:							// dendy
			comp->fps = 59;
			comp->cpuFrq = 1.77;
			perNoTurbo = 1e3 / comp->cpuFrq;
			vidSetLayout(comp->vid, &nesPALLay);
			comp->vid->vbsline = 291;
			comp->vid->vbrline = 311;
			vidUpdateTimings(comp->vid, perNoTurbo / 3);
			comp->nesapu->wdiv = 3729;
			break;
	}
	comp->nesapu->wper = perNoTurbo << 1;				// 1 APU tick = 2 CPU ticks	x2(Fcpu div) x8(duty)
	comp->cpu->nod = 1;	// no daa in cpu
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
			nes_init(comp);
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
