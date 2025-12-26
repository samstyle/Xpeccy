// for future: NEC PC98xx (9801 for start)
// CPU: 8086
// Hardware: almost IBM PC

// Memory map:
// 00000-9ffff - ram
// a0000-a7fff - text video mode data
// a8000-bffff - graphics video mode data
// c0000-c7fff - expansion rom (user)
// c8000-dffff - expansion rom (system)
// e0000-e7fff - video ram (optional)
// e8000-fffff - bios
// reset @ ffff:0000 = ffff0

#include "../spectrum.h"

// ram
int pc98xx_ram_rd(int adr, void* p) {
	Computer* comp = (Computer*)p;
	return (adr < comp->mem->ramSize) ? comp->mem->ramData[adr] : 0xff;
}

void pc98xx_ram_wr(int adr, int val, void* p) {
	Computer* comp = (Computer*)p;
	if (adr < comp->mem->ramSize)
		comp->mem->ramData[adr] = val & 0xff;
}

// video
int pc98xx_vid_rd(int adr, void* p) {
	return -1;
}

void pc98xx_vid_wr(int adr, int val, void* p) {

}

// bios
int pc98xx_bios_rd(int adr, void* p) {
	Computer* comp = (Computer*)p;
	adr -= 0xe8000;			// rom size is 96K (rounded to 128K), without this e8000 will be at 8000 in rom
	return comp->mem->romData[adr & comp->mem->romMask];
}

void pc98xx_map_mem(Computer* comp) {
	memSetBank(comp->mem, 0x00, MEM_RAM, 0, 0x10000, pc98xx_ram_rd, pc98xx_ram_wr, comp);	// ram
	memSetBank(comp->mem, 0x0a, MEM_EXT, 0, 0x200, pc98xx_vid_rd, pc98xx_vid_wr, comp);	// video
	memSetBank(comp->mem, 0xc0, MEM_EXT, 0, 0x100, NULL, NULL, comp);			// expand roms
	memSetBank(comp->mem, 0xe0, MEM_EXT, 0, 0x80, NULL, NULL, comp);			// video ram?
	memSetBank(comp->mem, 0xe8, MEM_ROM, 0, 0x180, pc98xx_bios_rd, NULL, comp);		// bios
}

void pc98xx_reset(Computer* comp) {
	cpu_reset(comp->cpu);
	pc98xx_map_mem(comp);
}

// uPD8251 - keyboard controller (r:41,rw:43)
// uPD8255 - misc data (rs232,soft reset,misc flags) (31,33,35,37)
// uPD4990 - clock/calendar (serial data transfer) (20,33)
// video(txt) - 60,62,64,68,6c / 70,72,74,76,78,7a / a1 a3 a5 a9

// uPD8253 - PIT (rw:71,73,75, w:77)
// adr 1..3 -> pit adr 0..2
int pc98xx_pit_rd(Computer* comp, int adr) {
	return pit_rd(comp->pit, (adr >> 1) & 7);
}

void pc98xx_pit_wr(Computer* comp, int adr, int val) {
	pit_wr(comp->pit, (adr >> 1) & 7, val);
}

// TODO: fill this table
// NOTE: run with --panic argument to exit on unknown i/o address
xPort pc98xx_io_map[] = {
	{0xff1, 0x071, 0, 0, 0, pc98xx_pit_rd, pc98xx_pit_wr},		// 71,73,75,77: PIT
	{0x000, 0x000, 0, 0, 0, NULL, NULL}
};

int pc98xx_iord(Computer* comp, int adr) {
	return hwIn(pc98xx_io_map, comp, adr);
}

void pc98xx_iowr(Computer* comp, int adr, int val) {
	hwOut(pc98xx_io_map, comp, adr, val, 0);
}

int pc98xx_mrd(Computer* comp, int adr, int m1) {
	return memRd(comp->mem, adr);
}

void pc98xx_mwr(Computer* comp, int adr, int val) {
	memWr(comp->mem, adr, val);
}

void pc98xx_irq(Computer* comp, int id) {
	switch(id) {
		case IRQ_PIT_CH0: break;	// ch0: interval
		case IRQ_PIT_CH1: break;	// ch1: speaker NOTE:9801/E/F/M for memory refresh
		case IRQ_PIT_CH2: break;	// ch2: rs232 timer
	}
}

void pc98xx_sync(Computer* comp, int ns) {
	pit_sync(comp->pit, ns);		// timers
}
