#include "../cpu.h"

#define I286_FC	0x0001	// carry
#define I286_FP 0x0004	// parity
#define I286_FA 0x0010	// half-carry
#define I286_FZ 0x0040	// zero
#define I286_FS 0x0080	// sign
#define I286_FT 0x0100	// trap
#define I286_FI 0x0200	// interrupt
#define I286_FD	0x0400	// direction
#define I286_FO 0x0800	// overflow
#define I286_FN	0x4000	// nested flag

enum {
	I286_MOD_REAL = 0,
	I286_MOD_PROT
};

enum {
	I286_REP_NONE = 0,
	I286_REPNZ,
	I286_REPZ,
};

void i80286_reset(CPU*);
int i80286_exec(CPU*);
