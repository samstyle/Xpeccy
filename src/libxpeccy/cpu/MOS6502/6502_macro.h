#pragma once

#define MFLAGZN(_op) \
	/*cpu->f = (cpu->f & ~(MFN | MFZ)) | ((_op & 0x80) ? MFN : 0) | (_op ? 0 : MFZ)*/;\
	cpu->fm.n = !!((_op) & 0x80);\
	cpu->fm.z = !(_op);

#define MJR(_e) \
	cpu->t++;\
	cpu->tmpb = cpu->hpc;\
	cpu->pc += (signed char)(_e);\
	if (cpu->tmpb != cpu->hpc) cpu->t++;

// TODO: check V flag
#define MBIT(_op) \
	/*cpu->f = (cpu->f & ~(MFN | MFZ | MFV)) | ((cpu->a & _op) ? 0 : MFZ) | ((_op & 0x80) ? MFN : 0) | ((_op & 0x40) ? MFV : 0);*/\
	cpu->fm.v = !!((_op) & 0x40);\
	cpu->fm.n = !!(cpu->a & 0x80);\
	cpu->fm.z = !(cpu->a & (_op));

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
	/*cpu->f = (cpu->f & ~(MFN | MFC | MFZ)) | ((cpu->tmpw & 0x100) ? 0 : MFC) | ((cpu->tmpw & 0x80) ? MFN : 0) | (cpu->ltw ? 0 : MFZ)*/;\
	cpu->fm.n = !!(cpu->tmpw & 0x80);\
	cpu->fm.c = !(cpu->tmpw & 0x100);\
	cpu->fm.z = !cpu->ltw;

// math
// NOTE: ADD set C flag if overflow occured, clears if not
// NOTE: BCD mode (flag MFD) doesn't affect NES
#define MADC(_ac, _op) \
	cpu->e = cpu->fm.c; /*(cpu->f & MFC) ? 1 : 0;*/\
	cpu->tmpw = _ac + _op + cpu->e;\
	/*cpu->f &= ~(MFV | MFC | MFZ | MFN));*/\
	cpu->fm.z = !(cpu->tmpw & 0xff);/*if (!(cpu->tmpw & 0xff)) cpu->f |= MFZ;*/\
	if (/*(cpu->f & MFD)*/ cpu->fm.d && !cpu->nod) {\
		if ((_ac & 0x0f) + (_op & 0x0f) + cpu->e > 9) cpu->tmpw += 6;\
		cpu->fm.n = !!(cpu->tmpw & 0x80); /*if (cpu->tmpw & 0x80) cpu->f |= MFN;*/\
		cpu->fm.v = !((_ac ^ _op) & 0x80) && ((_ac ^ cpu->tmpw) & 0x80); /* cpu->f |= MFV;*/\
		if (cpu->tmpw > 0x99) cpu->tmpw += 0x60;\
		cpu->fm.c = !!(cpu->tmpw > 0x99); /*if (cpu->tmpw > 0x99) cpu->f |= MFC;*/\
	} else {\
		cpu->fm.n = !!(cpu->tmpw & 0x80); /*if (cpu->tmpw & 0x80) cpu->f |= MFN;*/\
		cpu->fm.v = !((_ac ^ _op) & 0x80) && ((_ac ^ cpu->tmpw) & 0x80); /* cpu->f |= MFV;*/\
		cpu->fm.c = !!(cpu->tmpw & 0xff00); /*if (cpu->tmpw & 0xff00) cpu->f |= MFC;*/\
	}\
	_ac = cpu->tmpw & 0xff;

#define MSBC(_ac, _op) \
	cpu->e = !cpu->fm.c; /*(cpu->f & MFC) ? 0 : 1;*/\
	cpu->tmpw = _ac - _op - cpu->e;\
	/*cpu->f &= ~(MFV | MFC | MFZ | MFN);*/\
	cpu->fm.n = !!(cpu->tmpw & 0x80); /*if (cpu->tmpw & 0x80) cpu->f |= MFN;*/\
	cpu->fm.z = !(cpu->tmpw & 0xff); /*if (!(cpu->tmpw & 0xff)) cpu->f |= MFZ;*/\
	cpu->fm.v = ((cpu->tmpw ^ _ac) & 0x80) && ((_ac ^ _op) & 0x80); /*cpu->f |= MFV;*/\
	if (/*(cpu->f & MFD)*/ cpu->fm.d && !cpu->nod) {\
		if (((_ac & 0x0f) - cpu->e) < (_op & 0x0f)) cpu->tmpw -= 6;\
		if (cpu->tmpw > 0x99) cpu->tmpw -= 0x60;\
	}\
	cpu->fm.c = !!(cpu->tmpw < 0x100);/*if (cpu->tmpw < 0x100) cpu->f |= MFC;*/\
	_ac = cpu->tmpw & 0xff;

// shift

#define MASL(_op) \
	cpu->fm.c = !!(_op & 0x80);\
	/*if (_op & 0x80) {cpu->f |= MFC;} else {cpu->f &= ~MFC;}*/\
	_op = (_op << 1) & 0xff;\
	MFLAGZN(_op);

#define MLSR(_op) \
	cpu->fm.c = !!(_op & 0x01);\
	/*if (_op & 0x01) {cpu->f |= MFC;} else {cpu->f &= ~MFC;}*/\
	_op = _op >> 1;\
	/*cpu->f = (cpu->f & ~(MFN | MFZ)) | (_op ? 0 : MFZ)*/;\
	cpu->fm.n = 0;\
	cpu->fm.z = !(_op);

#define MROL(_op) \
	cpu->tmpw = _op & 0xff;\
	cpu->tmpw <<= 1;\
	if (cpu->fm.c) cpu->tmpw |= 1;\
	cpu->fm.c = !!(cpu->tmpw & 0x100);\
	/*if (cpu->f & MFC) cpu->tmpw |= 1;\
	if (cpu->tmpw & 0x100) {cpu->f |= MFC;} else {cpu->f &= ~MFC;}*/\
	_op = cpu->ltw;\
	MFLAGZN(_op);

#define MROR(_op) \
	cpu->tmpw = _op & 0xff;\
	if (cpu->fm.c) cpu->tmpw |= 0x100;\
	cpu->fm.c = !!(cpu->tmpw & 1);\
	/*if (cpu->f & MFC) cpu->tmpw |= 0x100;\
	if (cpu->tmpw & 1) {cpu->f |= MFC;} else {cpu->f &= ~MFC;}*/\
	cpu->tmpw >>= 1;\
	_op = cpu->ltw;\
	MFLAGZN(_op);
