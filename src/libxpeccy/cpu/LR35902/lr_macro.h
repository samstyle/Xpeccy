#pragma once

// shift

#define RLX(val) {\
	cpu->tmp = val;\
	val = (val << 1) | cpu->flgC;\
	cpu->flgN = 0;\
	cpu->flgH = 0;\
	cpu->flgC = !!(cpu->tmp & 0x80);\
	cpu->flgZ = !val;\
}

//	cpu->f = ((val & 1) ? FLC : 0) | (val ? 0 : FLZ);
#define RLCX(val) {\
	val = (val << 1) | (val >> 7);\
	cpu->flgN = 0;\
	cpu->flgH = 0;\
	cpu->flgC = val & 1;\
	cpu->flgZ = !val;\
}

//	cpu->f = ((cpu->tmp & 1) ? FLC : 0) | (val ? 0 : FLZ);
#define RRX(val) {\
	cpu->tmp = val;\
	val = (val >> 1) | (cpu->flgC ? 0x80 : 0);\
	cpu->flgN = 0;\
	cpu->flgH = 0;\
	cpu->flgC = val & 1;\
	cpu->flgZ = !val;\
}

#define RRCX(val) {\
	cpu->flgC = val & 1; /*cpu->f = (val & 1) ? FLC : 0;*/\
	val = (val >> 1) | (val << 7);\
	cpu->flgN = 0;\
	cpu->flgH = 0;\
	cpu->flgZ = !val;\
	/*cpu->f |= (val ? 0 : FLZ);*/\
}

#define SLAX(val) {\
	cpu->flgC = !!(val & 0x80); /*cpu->f = (val & 0x80) ? FLC : 0;*/\
	val <<= 1;\
	cpu->flgN = 0;\
	cpu->flgH = 0;\
	cpu->flgZ = !val;\
	/*cpu->f |= (val ? 0 : FLZ);*/\
}

#define SLLX(val) {\
	cpu->flgC = !!(val & 0x80);\
	val = (val << 1) | 0x01;\
	cpu->flgN = 0;\
	cpu->flgH = 0;\
	cpu->flgZ = !val;\
	/*cpu->f |= (val ? 0 : FLZ);*/\
}

#define SRAX(val) {\
	cpu->flgC = val & 1;\
	val = (val & 0x80) | (val >> 1);\
	cpu->flgN = 0;\
	cpu->flgH = 0;\
	cpu->flgZ = !val;\
	/*cpu->f |= (val ? 0 : FLZ);*/\
}

#define SRLX(val) {\
	cpu->flgC = val & 1;\
	val >>= 1;\
	cpu->flgN = 0;\
	cpu->flgH = 0;\
	cpu->flgZ = !val;\
	/*cpu->f |= (val ? 0 : FLZ);*/\
}

// bit

// cpu->f = (cpu->f & FLC) | FLH | ((val & (0x01 << bit)) ? 0 : FLZ);
#define BITL(bit,val) {\
	cpu->flgN = 0;\
	cpu->flgH = 1;\
	cpu->flgZ = !(val & (1 << bit));\
}
