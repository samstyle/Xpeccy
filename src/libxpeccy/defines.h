#pragma once

#include <stdint.h>
#include <time.h>

#ifdef __WIN32
	#define EXPORTDLL __declspec(dllimport)
	#include <windows.h>
	#define dlopen(d1,d2) LoadLibrary(d1)
	#define dlclose FreeLibrary
	#define dlsym GetProcAddress
#else
	#include <dlfcn.h>
	#define EXPORTDLL
#endif

// compilation flags
#define USE_HOST_KEYBOARD	1

// interrupt types
enum {
	IRQ_BRK = 1,	// common: call deBUGa
	IRQ_VID_INT,	// video
	IRQ_VID_INT_E,
	IRQ_VID_FRAME,
	IRQ_VID_VBLANK,
	IRQ_VID_LINE,	// tsconf
	IRQ_RZX_INT,
	IRQ_DMA,
	IRQ_CPU_SYNC,	// sync cpu ticks
	IRQ_CPU_HALT,	// enter halt cycle
	IRQ_FDC,	// ibm
	IRQ_FDC_RD,
	IRQ_FDC_WR,
	IRQ_FDD_RDY,
	IRQ_HDD_PRI,
	IRQ_SLAVE_PIC,
	IRQ_MASTER_PIC,
	IRQ_COM1,	// uart8250 com1 = mouse
	IRQ_KBD,	// ps/2 controller kbd int || gbc buttons
	IRQ_MOUSE,	// ps/2 controller mouse int
	IRQ_MOUSE_DATA,	// mouse moving (mouse -> uart -> pic -> cpu)
	IRQ_MOUSE_ACK,
	IRQ_KBD_DATA,
	IRQ_KBD_ACK,
	IRQ_RESET,
	IRQ_PIT_CH0,
	IRQ_PIT_CH1,
	IRQ_PIT_CH2,
	IRQ_APU,	// nes
	IRQ_CIA1,	// commodore
	IRQ_CIA2,
	IRQ_VIC,
	IRQ_TAP_0,
	IRQ_TAP_1,
	IRQ_TAP_BLK
};

typedef void(*cbirq)(int, void*);
// external data ports rd/wr
typedef int(*cbxrd)(int, void*);
typedef void(*cbxwr)(int, int, void*);

// breakpoint type

enum {
	BRK_UNKNOWN = 0,
	BRK_IOPORT,
	BRK_CPUADR,
	BRK_MEMCELL,
	BRK_MEMRAM,
	BRK_MEMROM,
	BRK_MEMSLT,
	BRK_MEMEXT,
	BRK_IRQ,
	BRK_HBLANK
};

// 16/32-bits reg

#ifdef WORDS_BIG_ENDIAN
	#define PAIR(p,h,l) union{uint16_t p; struct {uint8_t h; uint8_t l;};}
	#define reg16(_w,_h,_l) union{uint16_t _w; struct{uint8_t h; uint8_t l;};}
	#define reg32(_i,_w,_h,_l) union{uint32_t _i; struct{uint16_t _i##h; PAIR(_w,_h,_l);};}
#else
	#define PAIR(p,h,l) union{uint16_t p; struct {uint8_t l; uint8_t h;};}
	#define reg16(_w,_h,_l) union{uint16_t _w; struct{uint8_t _l; uint8_t _h;};}
	#define reg32(_i,_w,_h,_l) union{uint32_t _i; struct{reg16(_w,_h,_l); uint16_t _i##h;};}
#endif

typedef PAIR(w,h,l) xpair;
typedef reg32(i,w,h,l) xreg32;
typedef reg16(w,h,l) xreg16;

// time
extern clock_t tClock;
#define EXECTIME (clock() - tClock)

// memory size
#define MEM_256	(1<<8)
#define MEM_512	(1<<9)
#define MEM_1K	(1<<10)
#define MEM_2K	(1<<11)
#define MEM_4K	(1<<12)
#define MEM_8K	(1<<13)
#define MEM_16K	(1<<14)
#define MEM_32K	(1<<15)
#define MEM_64K	(1<<16)
#define MEM_128K	(1<<17)
#define MEM_256K	(1<<18)
#define MEM_512K	(1<<19)
#define MEM_1M	(1<<20)
#define MEM_2M	(1<<21)
#define MEM_4M	(1<<22)
