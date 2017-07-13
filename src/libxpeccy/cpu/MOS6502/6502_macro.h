#ifndef _6502_MACRO_H
#define _6502_MACRO_H

#define MFLAGZN(_op) \
	cpu->f = (cpu->f & ~(MFN | MFZ)) | ((_op & 0x80) ? MFN : 0) | (_op ? 0 : MFZ);

#define MJR(_e) \
	cpu->t++;\
	cpu->tmpb = cpu->hpc;\
	cpu->pc += (signed char)(_e);\
	if (cpu->tmpb != cpu->hpc) cpu->t++;

#define MBIT(_op) \
	cpu->f = (cpu->f & ~(MFN | MFZ | MFV)) | ((cpu->a & _op) ? 0 : MFZ) | ((_op & 0x80) ? MFN : 0) | ((_op & 0x40) ? MFV : 0);

// logic

#define MORA(_op) \
	cpu->a |= _op;\
	MFLAGZN(cpu->a);

#define MAND(_op) \
	cpu->a &= _op;\
	MFLAGZN(cpu->a);

#define MEOR(_op) \
	cpu->a ^= _op;\
	MFLAGZN(cpu->a);

// NOTE: SUB & CMP clears C flag if overflow occured, set if not
#define MCMP(_ac, _op) \
	cpu->tmpw = _ac - _op;\
	cpu->f = (cpu->f & ~(MFN | MFC | MFZ)) | ((cpu->tmpw & 0x100) ? 0 : MFC) | ((cpu->tmpw & 0x80) ? MFN : 0) | (cpu->ltw ? 0 : MFZ);

// math
// NOTE: ADD set C flag if overflow occured, clears if not
#define MADC(_op) \
	cpu->e = (cpu->f & MFC) ? 1 : 0;\
	cpu->tmpw = cpu->a + _op + cpu->e;\
	cpu->f &= ~(MFV | MFC | MFZ | MFN);\
	if (cpu->f & MFD) {\
		cpu->l = (cpu->a & 0x0f) + (_op & 0x0f) + cpu->e;\
		cpu->h = (cpu->a >> 4) + (_op >> 4) + ((cpu->l >> 4) & 1);\
		if (cpu->l > 9) cpu->l += 6;\
		if (!cpu->ltw) cpu->f |= MFZ;\
		if (cpu->h & 8) cpu->f |= MFN;\
		if ((((cpu->h << 4) ^ cpu->a) & 0x80) && !((cpu->a ^ _op) & 0x80)) cpu->f |= MFV;\
		if (cpu->h > 9) cpu->h += 6;\
		if (cpu->h > 15) cpu->f |= MFC;\
		cpu->a = ((cpu->h & 0x0f) << 4) | (cpu->l & 0x0f);\
	} else {\
		cpu->tmpb = (cpu->a & 0x7f) + (_op & 0x7f) + cpu->e;\
		cpu->a = cpu->ltw;\
		if (cpu->tmpw & 0x100) cpu->f |= MFC;\
		if (!cpu->a) cpu->f |= MFZ;\
		if (cpu->a & 0x80) cpu->f |= MFN;\
		if (cpu->tmpb & 0x80) cpu->f |= MFV;\
	}

#define MSBC(_op) \
	cpu->e = (cpu->f & MFC) ? 0 : 1;\
	cpu->tmpw = cpu->a - _op - cpu->e;\
	cpu->f &= ~(MFV | MFC | MFZ | MFN);\
	if (((cpu->tmpw ^ _op) & 0x80) && ((cpu->a ^ _op) & 0x80)) cpu->f |= MFV;\
	if (cpu->f & MFD) {\
		cpu->l = (cpu->a & 15) - (_op & 15) - cpu->e;\
		if (cpu->l & 16) cpu->l -= 6;\
		cpu->h = (cpu->a >> 4) - (_op >> 4) - ((cpu->l & 16) ? 1 : 0);\
		if (cpu->h & 15) cpu->h -= 6;\
		if (cpu->tmpw < 0x100) cpu->f |= MFC;\
		if (!cpu->ltw) cpu->f |= MFZ;\
		if (cpu->tmpw & 0x80) cpu->f |= MFN;\
		cpu->a = ((cpu->h & 0x0f) << 4) | (cpu->l & 0x0f);\
	} else {\
		cpu->tmpb = (cpu->a & 0x7f) - (_op & 0x7f) - cpu->e;\
		cpu->a = cpu->ltw;\
		if (cpu->tmpw < 0x100) cpu->f |= MFC;\
		if (!cpu->a) cpu->f |= MFZ;\
		if (cpu->a & 0x80) cpu->f |= MFN;\
	}

// shift

#define MASL(_op) \
	if (_op & 0x80) {cpu->f |= MFC;} else {cpu->f &= ~MFC;}\
	_op = (_op << 1) & 0xff;\
	MFLAGZN(cpu->a);

#define MLSR(_op) \
	if (_op & 0x01) {cpu->f |= MFC;} else {cpu->f &= ~MFC;}\
	_op = _op >> 1;\
	cpu->f = (cpu->f & ~(MFN | MFZ)) | (_op ? 0 : MFZ);

#define MROL(_op) \
	cpu->tmpw = _op & 0xff;\
	cpu->tmpw <<= 1;\
	if (cpu->f & MFC) cpu->tmpw |= 1;\
	if (cpu->tmpw & 0x100) {cpu->f |= MFC;} else {cpu->f &= ~MFC;}\
	_op = cpu->ltw;\
	MFLAGZN(_op);

#define MROR(_op) \
	cpu->tmpw = _op & 0xff;\
	if (cpu->f & MFC) cpu->tmpw |= 0x100;\
	if (cpu->tmpw & 1) {cpu->f |= MFC;} else {cpu->f &= ~MFC;}\
	cpu->tmpw >>= 1;\
	_op = cpu->ltw;\
	MFLAGZN(_op);

#endif
