#pragma once

#define regA regs[0].l
#define regE regs[2].l

#define MFLAGZN(_op) \
	/*cpu->f = (cpu->f & ~(MFN | MFZ)) | ((_op & 0x80) ? MFN : 0) | (_op ? 0 : MFZ)*/;\
	cpu->flgN = !!((_op) & 0x80);\
	cpu->flgZ = !(_op);

#define MJR(_e) \
	cpu->t++;\
	cpu->tmpb = cpu->regPCh;\
	cpu->regPC += (signed char)(_e);\
	if (cpu->tmpb != cpu->regPCh) cpu->t++;

// TODO: check V flag
#define MBIT(_op) \
	/*cpu->f = (cpu->f & ~(MFN | MFZ | MFV)) | ((cpu->a & _op) ? 0 : MFZ) | ((_op & 0x80) ? MFN : 0) | ((_op & 0x40) ? MFV : 0);*/\
	cpu->flgV = !!((_op) & 0x40);\
	cpu->flgN = !!((_op) & 0x80);\
	cpu->flgZ = !(cpu->regA & (_op));

// logic

/*
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
	cpu->flgN = !!(cpu->tmpw & 0x80);\
	cpu->flgC = !(cpu->tmpw & 0x100);\
	cpu->flgZ = !cpu->ltw;

// math
// NOTE: ADD set C flag if overflow occured, clears if not
// NOTE: BCD mode (flag MFD) doesn't affect NES
#define MADC(_ac, _op) \
	cpu->regE = cpu->flgC; \
	cpu->tmpw = _ac + _op + cpu->regE;\
	cpu->flgZ = !(cpu->tmpw & 0xff);\
	if ( cpu->flgD && !cpu->flgND) {\
		if ((_ac & 0x0f) + (_op & 0x0f) + cpu->regE > 9) cpu->tmpw += 6;\
		cpu->flgN = !!(cpu->tmpw & 0x80);\
		cpu->flgV = !((_ac ^ _op) & 0x80) && ((_ac ^ cpu->tmpw) & 0x80);\
		if (cpu->tmpw > 0x99) cpu->tmpw += 0x60;\
		cpu->flgC = !!(cpu->tmpw > 0x99);\
	} else {\
		cpu->flgN = !!(cpu->tmpw & 0x80);\
		cpu->flgV = !((_ac ^ _op) & 0x80) && ((_ac ^ cpu->tmpw) & 0x80);\
		cpu->flgC = !!(cpu->tmpw & 0xff00);\
	}\
	_ac = cpu->tmpw & 0xff;

#define MSBC(_ac, _op) \
	cpu->regE = !cpu->flgC;\
	cpu->tmpw = _ac - _op - cpu->regE;\
	cpu->flgN = !!(cpu->tmpw & 0x80);\
	cpu->flgZ = !(cpu->tmpw & 0xff);\
	cpu->flgV = ((cpu->tmpw ^ _ac) & 0x80) && ((_ac ^ _op) & 0x80);\
	if (cpu->flgD && !cpu->flgND) {\
		if (((_ac & 0x0f) - cpu->regE) < (_op & 0x0f)) cpu->tmpw -= 6;\
		if (cpu->tmpw > 0x99) cpu->tmpw -= 0x60;\
	}\
	cpu->flgC = !!(cpu->tmpw < 0x100);\
	_ac = cpu->tmpw & 0xff;
*/
// shift

/*

#define MASL(_op) \
	cpu->flgC = !!(_op & 0x80);\
	_op = (_op << 1) & 0xff;\
	MFLAGZN(_op);

#define MLSR(_op) \
	cpu->flgC = !!(_op & 0x01);\
	_op = _op >> 1;\
	cpu->flgN = 0;\
	cpu->flgZ = !(_op);

#define MROL(_op) \
	cpu->tmpw = _op & 0xff;\
	cpu->tmpw <<= 1;\
	if (cpu->flgC) cpu->tmpw |= 1;\
	cpu->flgC = !!(cpu->tmpw & 0x100);\
	_op = cpu->ltw;\
	MFLAGZN(_op);

#define MROR(_op) \
	cpu->tmpw = _op & 0xff;\
	if (cpu->flgC) cpu->tmpw |= 0x100;\
	cpu->flgC = !!(cpu->tmpw & 1);\
	cpu->tmpw >>= 1;\
	_op = cpu->ltw;\
	MFLAGZN(_op);

*/
