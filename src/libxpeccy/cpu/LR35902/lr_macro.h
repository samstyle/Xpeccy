#pragma once

// shift

#define RLX(val) {\
	cpu->tmp = val;\
	val = (val << 1) | cpu->f.c;\
	cpu->f.n = 0;\
	cpu->f.h = 0;\
	cpu->f.c = !!(cpu->tmp & 0x80);\
	cpu->f.z = !val;\
}

//	cpu->f = ((val & 1) ? FLC : 0) | (val ? 0 : FLZ);
#define RLCX(val) {\
	val = (val << 1) | (val >> 7);\
	cpu->f.n = 0;\
	cpu->f.h = 0;\
	cpu->f.c = val & 1;\
	cpu->f.z = !val;\
}

//	cpu->f = ((cpu->tmp & 1) ? FLC : 0) | (val ? 0 : FLZ);
#define RRX(val) {\
	cpu->tmp = val;\
	val = (val >> 1) | (cpu->f.c ? 0x80 : 0);\
	cpu->f.n = 0;\
	cpu->f.h = 0;\
	cpu->f.c = val & 1;\
	cpu->f.z = !val;\
}

#define RRCX(val) {\
	cpu->f.c = val & 1; /*cpu->f = (val & 1) ? FLC : 0;*/\
	val = (val >> 1) | (val << 7);\
	cpu->f.n = 0;\
	cpu->f.h = 0;\
	cpu->f.z = !val;\
	/*cpu->f |= (val ? 0 : FLZ);*/\
}

#define SLAX(val) {\
	cpu->f.c = !!(val & 0x80); /*cpu->f = (val & 0x80) ? FLC : 0;*/\
	val <<= 1;\
	cpu->f.n = 0;\
	cpu->f.h = 0;\
	cpu->f.z = !val;\
	/*cpu->f |= (val ? 0 : FLZ);*/\
}

#define SLLX(val) {\
	cpu->f.c = !!(val & 0x80);\
	val = (val << 1) | 0x01;\
	cpu->f.n = 0;\
	cpu->f.h = 0;\
	cpu->f.z = !val;\
	/*cpu->f |= (val ? 0 : FLZ);*/\
}

#define SRAX(val) {\
	cpu->f.c = val & 1;\
	val = (val & 0x80) | (val >> 1);\
	cpu->f.n = 0;\
	cpu->f.h = 0;\
	cpu->f.z = !val;\
	/*cpu->f |= (val ? 0 : FLZ);*/\
}

#define SRLX(val) {\
	cpu->f.c = val & 1;\
	val >>= 1;\
	cpu->f.n = 0;\
	cpu->f.h = 0;\
	cpu->f.z = !val;\
	/*cpu->f |= (val ? 0 : FLZ);*/\
}

// bit

// cpu->f = (cpu->f & FLC) | FLH | ((val & (0x01 << bit)) ? 0 : FLZ);
#define BITL(bit,val) {\
	cpu->f.n = 0;\
	cpu->f.h = 1;\
	cpu->f.z = !(val & (1 << bit));\
}
