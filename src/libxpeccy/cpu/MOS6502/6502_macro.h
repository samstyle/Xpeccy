#pragma once

#define regA regs[0].l
#define regE regs[2].l

#define MFLAGZN(_op) \
	/*cpu->f = (cpu->f & ~(MFN | MFZ)) | ((_op & 0x80) ? MFN : 0) | (_op ? 0 : MFZ)*/;\
	cpu->f.n = !!((_op) & 0x80);\
	cpu->f.z = !(_op);

#define MJR(_e) \
	cpu->t++;\
	cpu->tmpb = cpu->regPCh;\
	cpu->regPC += (signed char)(_e);\
	if (cpu->tmpb != cpu->regPCh) cpu->t++;

// TODO: check V flag
#define MBIT(_op) \
	/*cpu->f = (cpu->f & ~(MFN | MFZ | MFV)) | ((cpu->a & _op) ? 0 : MFZ) | ((_op & 0x80) ? MFN : 0) | ((_op & 0x40) ? MFV : 0);*/\
	cpu->f.v = !!((_op) & 0x40);\
	cpu->f.n = !!(cpu->regA & 0x80);\
	cpu->f.z = !(cpu->regA & (_op));

// logic

#define MORA(_op) \
	cpu->regA |= _op;\
	MFLAGZN(cpu->regA);

#define MAND(_op) \
	cpu->regA &= _op;\
	MFLAGZN(cpu->regA);

#define MEOR(_op) \
	cpu->regA ^= _op;\
	MFLAGZN(cpu->regA);

// NOTE: SUB & CMP clears C flag if overflow occured, set if not
#define MCMP(_ac, _op) \
	cpu->tmpw = _ac - _op;\
	/*cpu->f = (cpu->f & ~(MFN | MFC | MFZ)) | ((cpu->tmpw & 0x100) ? 0 : MFC) | ((cpu->tmpw & 0x80) ? MFN : 0) | (cpu->ltw ? 0 : MFZ)*/;\
	cpu->f.n = !!(cpu->tmpw & 0x80);\
	cpu->f.c = !(cpu->tmpw & 0x100);\
	cpu->f.z = !cpu->ltw;

// math
// NOTE: ADD set C flag if overflow occured, clears if not
// NOTE: BCD mode (flag MFD) doesn't affect NES
#define MADC(_ac, _op) \
	cpu->regE = cpu->f.c; /*(cpu->f & MFC) ? 1 : 0;*/\
	cpu->tmpw = _ac + _op + cpu->regE;\
	/*cpu->f &= ~(MFV | MFC | MFZ | MFN));*/\
	cpu->f.z = !(cpu->tmpw & 0xff);/*if (!(cpu->tmpw & 0xff)) cpu->f |= MFZ;*/\
	if (/*(cpu->f & MFD)*/ cpu->f.d && !cpu->nod) {\
		if ((_ac & 0x0f) + (_op & 0x0f) + cpu->regE > 9) cpu->tmpw += 6;\
		cpu->f.n = !!(cpu->tmpw & 0x80); /*if (cpu->tmpw & 0x80) cpu->f |= MFN;*/\
		cpu->f.v = !((_ac ^ _op) & 0x80) && ((_ac ^ cpu->tmpw) & 0x80); /* cpu->f |= MFV;*/\
		if (cpu->tmpw > 0x99) cpu->tmpw += 0x60;\
		cpu->f.c = !!(cpu->tmpw > 0x99); /*if (cpu->tmpw > 0x99) cpu->f |= MFC;*/\
	} else {\
		cpu->f.n = !!(cpu->tmpw & 0x80); /*if (cpu->tmpw & 0x80) cpu->f |= MFN;*/\
		cpu->f.v = !((_ac ^ _op) & 0x80) && ((_ac ^ cpu->tmpw) & 0x80); /* cpu->f |= MFV;*/\
		cpu->f.c = !!(cpu->tmpw & 0xff00); /*if (cpu->tmpw & 0xff00) cpu->f |= MFC;*/\
	}\
	_ac = cpu->tmpw & 0xff;

#define MSBC(_ac, _op) \
	cpu->regE = !cpu->f.c; /*(cpu->f & MFC) ? 0 : 1;*/\
	cpu->tmpw = _ac - _op - cpu->regE;\
	/*cpu->f &= ~(MFV | MFC | MFZ | MFN);*/\
	cpu->f.n = !!(cpu->tmpw & 0x80); /*if (cpu->tmpw & 0x80) cpu->f |= MFN;*/\
	cpu->f.z = !(cpu->tmpw & 0xff); /*if (!(cpu->tmpw & 0xff)) cpu->f |= MFZ;*/\
	cpu->f.v = ((cpu->tmpw ^ _ac) & 0x80) && ((_ac ^ _op) & 0x80); /*cpu->f |= MFV;*/\
	if (/*(cpu->f & MFD)*/ cpu->f.d && !cpu->nod) {\
		if (((_ac & 0x0f) - cpu->regE) < (_op & 0x0f)) cpu->tmpw -= 6;\
		if (cpu->tmpw > 0x99) cpu->tmpw -= 0x60;\
	}\
	cpu->f.c = !!(cpu->tmpw < 0x100);/*if (cpu->tmpw < 0x100) cpu->f |= MFC;*/\
	_ac = cpu->tmpw & 0xff;

// shift

#define MASL(_op) \
	cpu->f.c = !!(_op & 0x80);\
	/*if (_op & 0x80) {cpu->f |= MFC;} else {cpu->f &= ~MFC;}*/\
	_op = (_op << 1) & 0xff;\
	MFLAGZN(_op);

#define MLSR(_op) \
	cpu->f.c = !!(_op & 0x01);\
	/*if (_op & 0x01) {cpu->f |= MFC;} else {cpu->f &= ~MFC;}*/\
	_op = _op >> 1;\
	/*cpu->f = (cpu->f & ~(MFN | MFZ)) | (_op ? 0 : MFZ)*/;\
	cpu->f.n = 0;\
	cpu->f.z = !(_op);

#define MROL(_op) \
	cpu->tmpw = _op & 0xff;\
	cpu->tmpw <<= 1;\
	if (cpu->f.c) cpu->tmpw |= 1;\
	cpu->f.c = !!(cpu->tmpw & 0x100);\
	/*if (cpu->f & MFC) cpu->tmpw |= 1;\
	if (cpu->tmpw & 0x100) {cpu->f |= MFC;} else {cpu->f &= ~MFC;}*/\
	_op = cpu->ltw;\
	MFLAGZN(_op);

#define MROR(_op) \
	cpu->tmpw = _op & 0xff;\
	if (cpu->f.c) cpu->tmpw |= 0x100;\
	cpu->f.c = !!(cpu->tmpw & 1);\
	/*if (cpu->f & MFC) cpu->tmpw |= 0x100;\
	if (cpu->tmpw & 1) {cpu->f |= MFC;} else {cpu->f &= ~MFC;}*/\
	cpu->tmpw >>= 1;\
	_op = cpu->ltw;\
	MFLAGZN(_op);
