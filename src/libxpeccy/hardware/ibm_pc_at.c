#include "hardware.h"

void ibm_init(Computer* comp) {}

void ibm_mem_map(Computer* comp) {}	// no hw memory mapping, it's work of cpu

void ibm_reset(Computer* comp) {}

// ibm pc/at
// 00000..9FFFF : ram
// A0000..BFFFF : video
// C0000..CFFFF : adapter roms
// D0000..DFFFF : ram pages
// E0000..EFFFF :
// F0000..FFFFF : bios
// 100000+	: ram

int ibm_mrd(Computer* comp, int adr, int m1) {
	int res = -1;
	if (adr < 0xa0000) {
		res = comp->mem->ramData[adr & comp->mem->ramMask];		// ram up to 640K
	} else if (adr < 0xc0000) {
		// videomem
	} else if (adr < 0xd0000) {
		// ext.bios
	} else if (adr < 0xe0000) {
		res = comp->mem->ramData[adr & comp->mem->ramMask];		// ram pages ?
	} else if (adr < 0x100000) {
		res = comp->mem->romData[adr & comp->mem->romMask];
	} else {
		res = comp->mem->ramData[adr & comp->mem->ramMask];		// ram 640K+
	}
	return res;
}

void ibm_mwr(Computer* comp, int adr, int val) {
	if (adr < 0xa0000) {
		comp->mem->ramData[adr & comp->mem->ramMask] = val & 0xff;
	} else if (adr < 0xc0000) {
		// video mem
	} else if (adr < 0xd0000) {
		// ext.bios
	} else if (adr < 0xe0000) {
		comp->mem->ramData[adr & comp->mem->ramMask] = val & 0xff;
	} else if (adr < 0x100000) {
		// bios
	} else {
		comp->mem->ramData[adr & comp->mem->ramMask] = val & 0xff;
	}
}

int ibm_iord(Computer* comp, int adr, int nonsence) {
	return -1;
}

void ibm_iowr(Computer* comp, int adr, int val, int nonsense) {

}
