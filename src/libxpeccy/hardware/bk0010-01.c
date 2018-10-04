#include "hardware.h"

// 0000..7FFF	RAM
// 8000..FEFF	ROM
// FF00..FFFF	IO

unsigned char bk_io_rd(unsigned short adr, void* ptr) {
	return 0xff;
}

void bk_io_wr(unsigned short adr, unsigned char val, void* ptr) {

}

void bk_mem_map(Computer* comp) {
	memSetBank(comp->mem, 0x00, MEM_RAM, 0, MEM_32K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0x80, MEM_ROM, 0, MEM_32K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0xff, MEM_IO, 0, MEM_256, bk_io_rd, bk_io_wr, comp);
}
