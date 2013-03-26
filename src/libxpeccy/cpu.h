#ifndef _CPU_H
#define _CPU_H

#ifdef SELFZ80
	#include "z80/z80.h"
	typedef unsigned short Z80EX_WORD;
	typedef unsigned char Z80EX_BYTE;
	typedef Z80CPU CPU;

	#define CPUCONT
	#define KILLCPU(prc) cpuDestroy(prc)
	#define RESETCPU(prc) cpuReset(prc)
	#define EXECCPU(prc,rz) rz = cpuExec(prc)
	#define INTCPU(prc) cpuINT(prc)
	#define NMICPU(prc) cpuNMI(prc)
	#define TCPU(prc) prc->t

	#define GETPC(prc) prc->pc
	#define GETSP(prc) prc->sp
	#define GETIX(prc) prc->ix
	#define GETIY(prc) prc->iy
	#define GETAF(prc) prc->af
	#define GETBC(prc) prc->bc
	#define GETDE(prc) prc->de
	#define GETHL(prc) prc->hl
	#define GETAF_(prc) prc->af_
	#define GETBC_(prc) prc->bc_
	#define GETDE_(prc) prc->de_
	#define GETHL_(prc) prc->hl_
	#define GETI(prc) prc->i
	#define GETR(prc) prc->r
	#define GETIFF1(prc) prc->iff1
	#define GETIFF2(prc) prc->iff2
	#define GETIM(prc) prc->imode

	#define SETAF(prc,num) prc->af=num
	#define SETBC(prc,num) prc->bc=num
	#define SETDE(prc,num) prc->de=num
	#define SETHL(prc,num) prc->hl=num
	#define SETAF_(prc,num) prc->af_=num
	#define SETBC_(prc,num) prc->bc_=num
	#define SETDE_(prc,num) prc->de_=num
	#define SETHL_(prc,num) prc->hl_=num
	#define SETIX(prc,num) prc->ix=num
	#define SETIY(prc,num) prc->iy=num
	#define SETPC(prc,num) prc->pc=num
	#define SETSP(prc,num) prc->sp=num
	#define SETI(prc,num) prc->i=num
	#define SETR(prc,num) prc->r=num;prc->r7=prc->r&0x80
	#define SETIM(prc,num) prc->imode=num;
	#define SETIFF1(prc,num) prc->iff1=num;
	#define SETIFF2(prc,num) prc->iff2=num;
#else
	#include "z80ex.h"
	typedef Z80EX_CONTEXT CPU;

	#define CPUCONT Z80EX_CONTEXT* cpu,
	#define KILLCPU(prc) z80ex_destroy(prc)
	#define RESETCPU(prc) z80ex_reset(prc)
	#define EXECCPU(prc,rz) {\
		rz = 0;\
		do {\
			rz += z80ex_step(prc);\
		} while (z80ex_last_op_type(prc) != 0);\
	}
	#define INTCPU(prc) z80ex_int(prc)
	#define NMICPU(prc) z80ex_nmi(prc)
	#define TCPU(prc) res2+z80ex_op_tstate(prc)-1

	#define GETPC(prc) z80ex_get_reg(prc,regPC)
	#define GETSP(prc) z80ex_get_reg(prc,regSP)
	#define GETIX(prc) z80ex_get_reg(prc,regIX)
	#define GETIY(prc) z80ex_get_reg(prc,regIY)
	#define GETAF(prc) z80ex_get_reg(prc,regAF)
	#define GETBC(prc) z80ex_get_reg(prc,regBC)
	#define GETDE(prc) z80ex_get_reg(prc,regDE)
	#define GETHL(prc) z80ex_get_reg(prc,regHL)
	#define GETAF_(prc) z80ex_get_reg(prc,regAF_)
	#define GETBC_(prc) z80ex_get_reg(prc,regBC_)
	#define GETDE_(prc) z80ex_get_reg(prc,regDE_)
	#define GETHL_(prc) z80ex_get_reg(prc,regHL_)
	#define GETI(prc) z80ex_get_reg(prc,regI)
	#define GETR(prc) z80ex_get_reg(prc,regR)
	#define GETIFF1(prc) z80ex_get_reg(prc,regIFF1)
	#define GETIFF2(prc) z80ex_get_reg(prc,regIFF2)
	#define GETIM(prc) z80ex_get_reg(prc,regIM)

	#define SETAF(prc,num) z80ex_set_reg(prc,regAF,num)
	#define SETBC(prc,num) z80ex_set_reg(prc,regBC,num)
	#define SETDE(prc,num) z80ex_set_reg(prc,regDE,num)
	#define SETHL(prc,num) z80ex_set_reg(prc,regHL,num)
	#define SETAF_(prc,num) z80ex_set_reg(prc,regAF_,num)
	#define SETBC_(prc,num) z80ex_set_reg(prc,regBC_,num)
	#define SETDE_(prc,num) z80ex_set_reg(prc,regDE_,num)
	#define SETHL_(prc,num) z80ex_set_reg(prc,regHL_,num)
	#define SETIX(prc,num) z80ex_set_reg(prc,regIX,num)
	#define SETIY(prc,num) z80ex_set_reg(prc,regIY,num)
	#define SETSP(prc,num) z80ex_set_reg(prc,regSP,num)
	#define SETPC(prc,num) z80ex_set_reg(prc,regPC,num)
	#define SETI(prc,num) z80ex_set_reg(prc,regI,num)
	#define SETR(prc,num) z80ex_set_reg(prc,regR,num);z80ex_set_reg(prc,regR7,(num) & 0x80);
	#define SETIFF1(prc,num) z80ex_set_reg(prc,regIFF1,num)
	#define SETIFF2(prc,num) z80ex_set_reg(prc,regIFF2,num)
	#define SETIM(prc,num) z80ex_set_reg(prc,regIM,num)
#endif

#endif
