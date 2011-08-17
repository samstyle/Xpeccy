#include "z80.h"

Z80::Z80(float fr) {
	t = tb = 0;
	frq = fr;
}

void Z80::reset() {
	err = block = false;
	pc = 0;
	iff1 = iff2 = false;
}
