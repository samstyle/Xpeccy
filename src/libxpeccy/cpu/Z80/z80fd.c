#include <stdlib.h>
#include "../cpu.h"
#include "z80_macro.h"
#include "z80_nop.h"

extern opCode fdcbTab[256];
extern opCode ddTab[256];
extern opCode edTab[256];

// 09	add iy,bc	11	mptr = iy+1 before adding
void fd09(CPU* cpu) {
	ADD16(cpu->iy, cpu->bc);
}

// 19	add iy,de	11	mptr = iy+1 before adding
void fd19(CPU* cpu) {
	ADD16(cpu->iy,cpu->de);
}

// 21	ld iy,nn	4 3rd 3rd
void fd21(CPU* cpu) {
	cpu->ly = MEMRD(cpu->pc++,3);
	cpu->hy = MEMRD(cpu->pc++,3);
}

// 22	ld (nn),iy	4 3rd 3rd 3wr 3wr	mptr = nn+1
void fd22(CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	MEMWR(cpu->mptr++,cpu->ly,3);
	MEMWR(cpu->mptr,cpu->hy,3);
}

// 23	inc iy		6
void fd23(CPU* cpu) {
	cpu->iy++;
}

// 24	inc hy		4
void fd24(CPU* cpu) {
	INC(cpu->hy);
}

// 25	dec hy		4
void fd25(CPU* cpu) {
	DEC(cpu->hy);
}

// 26	ld hy,n		4 3rd
void fd26(CPU* cpu) {
	cpu->hy = MEMRD(cpu->pc++,3);
}

// 29	add iy,iy	11
void fd29(CPU* cpu) {
	ADD16(cpu->iy,cpu->iy);
}

// 2A	ld iy,(nn)	4 3rd 3rd 3rd 3rd	mptr = nn+1
void fd2A(CPU* cpu) {
	cpu->lptr = MEMRD(cpu->pc++,3);
	cpu->hptr = MEMRD(cpu->pc++,3);
	cpu->ly = MEMRD(cpu->mptr++,3);
	cpu->hy = MEMRD(cpu->mptr,3);
}

// 2B	dec iy		6
void fd2B(CPU* cpu) {
	cpu->iy--;
}

// 2C	inc ly		4
void fd2C(CPU* cpu) {
	INC(cpu->ly);
}

// 2D	dec ly		4
void fd2D(CPU* cpu) {
	DEC(cpu->ly);
}

// 2E	ld ly,n		4 3rd
void fd2E(CPU* cpu) {
	cpu->ly = MEMRD(cpu->pc++,3);
}

// 34	inc (iy+e)	4 3rd 5add 4rd 3wr
void fd34(CPU* cpu) {
	RDSHIFT(cpu->iy);
	cpu->tmp = MEMRD(cpu->mptr,4);
	INC(cpu->tmp);
	MEMWR(cpu->mptr,cpu->tmp,3);
}

// 35	dec (iy+e)	4 3rd 5add 4rd 3wr	mptr = iy+e
void fd35(CPU* cpu) {
	RDSHIFT(cpu->iy);
	cpu->tmp = MEMRD(cpu->mptr,4);
	DEC(cpu->tmp);
	MEMWR(cpu->mptr,cpu->tmp,3);
}

// 36	ld (iy+e),n	4 3rd {5add 3rd} 3wr	mptr = iy+e
void fd36(CPU* cpu) {
	RDSHIFT(cpu->iy);
	cpu->tmp = MEMRD(cpu->pc++,0);
	MEMWR(cpu->mptr,cpu->tmp,3);
}

// 39	add iy,sp	11
void fd39(CPU* cpu) {
	ADD16(cpu->iy,cpu->sp);
}

// ld r,r		4 [3rd 5add 3rd]
void fd44(CPU* cpu) {cpu->b = cpu->hy;}
void fd45(CPU* cpu) {cpu->b = cpu->ly;}
void fd46(CPU* cpu) {RDSHIFT(cpu->iy); cpu->b = MEMRD(cpu->mptr,3);}
void fd4C(CPU* cpu) {cpu->c = cpu->hy;}
void fd4D(CPU* cpu) {cpu->c = cpu->ly;}
void fd4E(CPU* cpu) {RDSHIFT(cpu->iy); cpu->c = MEMRD(cpu->mptr,3);}
void fd54(CPU* cpu) {cpu->d = cpu->hy;}
void fd55(CPU* cpu) {cpu->d = cpu->ly;}
void fd56(CPU* cpu) {RDSHIFT(cpu->iy); cpu->d = MEMRD(cpu->mptr,3);}
void fd5C(CPU* cpu) {cpu->e = cpu->hy;}
void fd5D(CPU* cpu) {cpu->e = cpu->ly;}
void fd5E(CPU* cpu) {RDSHIFT(cpu->iy); cpu->e = MEMRD(cpu->mptr,3);}

void fd60(CPU* cpu) {cpu->hy = cpu->b;}
void fd61(CPU* cpu) {cpu->hy = cpu->c;}
void fd62(CPU* cpu) {cpu->hy = cpu->d;}
void fd63(CPU* cpu) {cpu->hy = cpu->e;}
void fd64(CPU* cpu) {}
void fd65(CPU* cpu) {cpu->hy = cpu->ly;}
void fd66(CPU* cpu) {RDSHIFT(cpu->iy); cpu->h = MEMRD(cpu->mptr,3);}
void fd67(CPU* cpu) {cpu->hy = cpu->a;}

void fd68(CPU* cpu) {cpu->ly = cpu->b;}
void fd69(CPU* cpu) {cpu->ly = cpu->c;}
void fd6A(CPU* cpu) {cpu->ly = cpu->d;}
void fd6B(CPU* cpu) {cpu->ly = cpu->e;}
void fd6C(CPU* cpu) {cpu->ly = cpu->hy;}
void fd6D(CPU* cpu) {}
void fd6E(CPU* cpu) {RDSHIFT(cpu->iy); cpu->l = MEMRD(cpu->mptr,3);}
void fd6F(CPU* cpu) {cpu->ly = cpu->a;}
// 70..77	ld (iy+e),r	4 3rd 5add 3wr
void fd70(CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->b,3);}
void fd71(CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->c,3);}
void fd72(CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->d,3);}
void fd73(CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->e,3);}
void fd74(CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->h,3);}
void fd75(CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->l,3);}
void fd77(CPU* cpu) {RDSHIFT(cpu->iy); MEMWR(cpu->mptr,cpu->a,3);}

void fd7C(CPU* cpu) {cpu->a = cpu->hy;}
void fd7D(CPU* cpu) {cpu->a = cpu->ly;}
void fd7E(CPU* cpu) {RDSHIFT(cpu->iy); cpu->a = MEMRD(cpu->mptr,3);}

// add x
void fd84(CPU* cpu) {ADD(cpu->hy);}
void fd85(CPU* cpu) {ADD(cpu->ly);}
void fd86(CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); ADD(cpu->tmpb);}
// adc x
void fd8C(CPU* cpu) {ADC(cpu->hy);}
void fd8D(CPU* cpu) {ADC(cpu->ly);}
void fd8E(CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); ADC(cpu->tmpb);}
// sub x
void fd94(CPU* cpu) {SUB(cpu->hy);}
void fd95(CPU* cpu) {SUB(cpu->ly);}
void fd96(CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); SUB(cpu->tmpb);}
// sbc x
void fd9C(CPU* cpu) {SBC(cpu->hy);}
void fd9D(CPU* cpu) {SBC(cpu->ly);}
void fd9E(CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); SBC(cpu->tmpb);}
// and x
void fdA4(CPU* cpu) {cpu->a &= cpu->hy; cpu->f = sz53pTab[cpu->a] | FH;}
void fdA5(CPU* cpu) {cpu->a &= cpu->ly; cpu->f = sz53pTab[cpu->a] | FH;}
void fdA6(CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); cpu->a &= cpu->tmpb; cpu->f = sz53pTab[cpu->a] | FH;}
// xor x
void fdAC(CPU* cpu) {cpu->a ^= cpu->hy; cpu->f = sz53pTab[cpu->a];}
void fdAD(CPU* cpu) {cpu->a ^= cpu->ly; cpu->f = sz53pTab[cpu->a];}
void fdAE(CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); cpu->a ^= cpu->tmpb; cpu->f = sz53pTab[cpu->a];}
// or x
void fdB4(CPU* cpu) {cpu->a |= cpu->hy; cpu->f = sz53pTab[cpu->a];}
void fdB5(CPU* cpu) {cpu->a |= cpu->ly; cpu->f = sz53pTab[cpu->a];}
void fdB6(CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); cpu->a |= cpu->tmpb; cpu->f = sz53pTab[cpu->a];}
// cp x
void fdBC(CPU* cpu) {CP(cpu->hy);}
void fdBD(CPU* cpu) {CP(cpu->ly);}
void fdBE(CPU* cpu) {RDSHIFT(cpu->iy); cpu->tmpb = MEMRD(cpu->mptr,3); CP(cpu->tmpb);}

// cb	fdcb prefix	4 3rd
void fdCB(CPU* cpu) {
	cpu->opTab = fdcbTab;
	cpu->tmp = MEMRD(cpu->pc++,3);
	cpu->tmpb = MEMRD(cpu->pc++,0);		// not M1?
	cpu->op = &fdcbTab[cpu->tmpb];
	cpu->op->exec(cpu);
}

// e1	pop iy		4 3rd 3rd
void fdE1(CPU* cpu) {
	POP(cpu->hy,cpu->ly);
}

// e3	ex (sp),iy	4 3rd 4rd 3wr 5wr	mptr = iy
void fdE3(CPU* cpu) {
	POP(cpu->htw,cpu->ltw); cpu->t++;	// 3,3+1
	PUSH(cpu->hy, cpu->ly); cpu->t += 2;	// 3,3+2
	cpu->iy = cpu->tmpw;
	cpu->mptr = cpu->iy;
}

// e5	push iy		5 3wr 3wr
void fdE5(CPU* cpu) {
	PUSH(cpu->hy,cpu->ly);
}

// e9	jp (iy)		4
void fdE9(CPU* cpu) {
	cpu->pc = cpu->iy;
}

// f9	ld sp,iy	6
void fdF9(CPU* cpu) {
	cpu->sp = cpu->iy;
}

// ======

opCode fdTab[256]={
	{0,4,npr00,NULL,"nop"},
	{0,4,npr01,NULL,"ld bc,:2"},
	{0,4,npr02,NULL,"ld (bc),a"},
	{0,6,npr03,NULL,"inc bc"},
	{0,4,npr04,NULL,"inc b"},
	{0,4,npr05,NULL,"dec b"},
	{0,4,npr06,NULL,"ld b,:1"},
	{0,4,npr07,NULL,"rlca"},

	{0,4,npr08,NULL,"ex af,af'"},
	{0,11,fd09,NULL,"add iy,bc"},
	{0,4,npr0A,NULL,"ld a,(bc)"},
	{0,6,npr0B,NULL,"dec bc"},
	{0,4,npr0C,NULL,"inc c"},
	{0,4,npr0D,NULL,"dec c"},
	{0,4,npr0E,NULL,"ld c,:1"},
	{0,4,npr0F,NULL,"rrca"},

	{OF_SKIPABLE | OF_RELJUMP,5,npr10,NULL,"djnz :3"},
	{0,4,npr11,NULL,"ld de,:2"},
	{0,4,npr12,NULL,"ld (de),a"},
	{0,6,npr13,NULL,"inc de"},
	{0,4,npr14,NULL,"inc d"},
	{0,4,npr15,NULL,"dec d"},
	{0,4,npr16,NULL,"ld d,:1"},
	{0,4,npr17,NULL,"rla"},

	{OF_RELJUMP,4,npr18,NULL,"jr :3"},
	{0,11,fd19,NULL,"add iy,de"},
	{0,4,npr1A,NULL,"ld a,(de)"},
	{0,6,npr1B,NULL,"dec de"},
	{0,4,npr1C,NULL,"inc e"},
	{0,4,npr1D,NULL,"dec e"},
	{0,4,npr1E,NULL,"ld e,:1"},
	{0,4,npr1F,NULL,"rra"},

	{OF_RELJUMP,4,npr20,NULL,"jr nz,:3"},
	{0,4,fd21,NULL,"ld iy,:2"},
	{OF_MWORD,4,fd22,NULL,"ld (:2),iy"},		// 4,3rd,3rd,3wr,3wr
	{0,6,fd23,NULL,"inc iy"},
	{0,4,fd24,NULL,"inc hy"},
	{0,4,fd25,NULL,"dec hy"},
	{0,4,fd26,NULL,"ld hy,:1"},
	{0,4,npr27,NULL,"daa"},

	{OF_RELJUMP,4,npr28,NULL,"jr z,:3"},
	{0,11,fd29,NULL,"add iy,iy"},
	{OF_MWORD,4,fd2A,NULL,"ld iy,(:2)"},		// 4,3rd,3rd,3rd,3rd
	{0,6,fd2B,NULL,"dec iy"},
	{0,4,fd2C,NULL,"inc ly"},
	{0,4,fd2D,NULL,"dec ly"},
	{0,4,fd2E,NULL,"ld ly,:1"},
	{0,4,npr2F,NULL,"cpl"},

	{OF_RELJUMP,4,npr30,NULL,"jr nc,:3"},
	{0,4,npr31,NULL,"ld sp,:2"},
	{0,4,npr32,NULL,"ld (:2),a"},		// 4,3rd,3rd,3wr
	{0,6,npr33,NULL,"inc sp"},
	{0,4,fd34,NULL,"inc (iy:4)"},
	{0,4,fd35,NULL,"dec (iy:4)"},
	{0,4,fd36,NULL,"ld (iy:4),:1"},
	{0,4,npr37,NULL,"scf"},

	{OF_RELJUMP,4,npr38,NULL,"jr c,:3"},
	{0,11,fd39,NULL,"add iy,sp"},
	{0,4,npr3A,NULL,"ld a,(:2)"},		// 4,3rd,3rd,3rd
	{0,6,npr3B,NULL,"dec sp"},
	{0,4,npr3C,NULL,"inc a"},
	{0,4,npr3D,NULL,"dec a"},
	{0,4,npr3E,NULL,"ld a,:1"},
	{0,4,npr3F,NULL,"ccf"},

	{0,4,npr40,NULL,"ld b,b"},
	{0,4,npr41,NULL,"ld b,c"},
	{0,4,npr42,NULL,"ld b,d"},
	{0,4,npr43,NULL,"ld b,e"},
	{0,4,fd44,NULL,"ld b,hy"},
	{0,4,fd45,NULL,"ld b,ly"},
	{0,4,fd46,NULL,"ld b,(iy:4)"},
	{0,4,npr47,NULL,"ld b,a"},

	{0,4,npr48,NULL,"ld c,b"},
	{0,4,npr49,NULL,"ld c,c"},
	{0,4,npr4A,NULL,"ld c,d"},
	{0,4,npr4B,NULL,"ld c,e"},
	{0,4,fd4C,NULL,"ld c,hy"},
	{0,4,fd4D,NULL,"ld c,ly"},
	{0,4,fd4E,NULL,"ld c,(iy:4)"},
	{0,4,npr4F,NULL,"ld c,a"},

	{0,4,npr50,NULL,"ld d,b"},
	{0,4,npr51,NULL,"ld d,c"},
	{0,4,npr52,NULL,"ld d,d"},
	{0,4,npr53,NULL,"ld d,e"},
	{0,4,fd54,NULL,"ld d,hy"},
	{0,4,fd55,NULL,"ld d,ly"},
	{0,4,fd56,NULL,"ld d,(iy:4)"},
	{0,4,npr57,NULL,"ld d,a"},

	{0,4,npr58,NULL,"ld e,b"},
	{0,4,npr59,NULL,"ld e,c"},
	{0,4,npr5A,NULL,"ld e,d"},
	{0,4,npr5B,NULL,"ld e,e"},
	{0,4,fd5C,NULL,"ld e,hy"},
	{0,4,fd5D,NULL,"ld e,ly"},
	{0,4,fd5E,NULL,"ld e,(iy:4)"},
	{0,4,npr5F,NULL,"ld e,a"},

	{0,4,fd60,NULL,"ld hy,b"},
	{0,4,fd61,NULL,"ld hy,c"},
	{0,4,fd62,NULL,"ld hy,d"},
	{0,4,fd63,NULL,"ld hy,e"},
	{0,4,fd64,NULL,"ld hy,hy"},
	{0,4,fd65,NULL,"ld hy,ly"},
	{0,4,fd66,NULL,"ld h,(iy:4)"},
	{0,4,fd67,NULL,"ld hy,a"},

	{0,4,fd68,NULL,"ld ly,b"},
	{0,4,fd69,NULL,"ld ly,c"},
	{0,4,fd6A,NULL,"ld ly,d"},
	{0,4,fd6B,NULL,"ld ly,e"},
	{0,4,fd6C,NULL,"ld ly,hy"},
	{0,4,fd6D,NULL,"ld ly,ly"},
	{0,4,fd6E,NULL,"ld l,(iy:4)"},
	{0,4,fd6F,NULL,"ld ly,a"},

	{0,4,fd70,NULL,"ld (iy:4),b"},
	{0,4,fd71,NULL,"ld (iy:4),c"},
	{0,4,fd72,NULL,"ld (iy:4),d"},
	{0,4,fd73,NULL,"ld (iy:4),e"},
	{0,4,fd74,NULL,"ld (iy:4),h"},
	{0,4,fd75,NULL,"ld (iy:4),l"},
	{0,4,npr76,NULL,"halt"},
	{0,4,fd77,NULL,"ld (iy:4),a"},

	{0,4,npr78,NULL,"ld a,b"},
	{0,4,npr79,NULL,"ld a,c"},
	{0,4,npr7A,NULL,"ld a,d"},
	{0,4,npr7B,NULL,"ld a,e"},
	{0,4,fd7C,NULL,"ld a,hy"},
	{0,4,fd7D,NULL,"ld a,ly"},
	{0,4,fd7E,NULL,"ld a,(iy:4)"},
	{0,4,npr7F,NULL,"ld a,a"},

	{0,4,npr80,NULL,"add a,b"},
	{0,4,npr81,NULL,"add a,c"},
	{0,4,npr82,NULL,"add a,d"},
	{0,4,npr83,NULL,"add a,e"},
	{0,4,fd84,NULL,"add a,hy"},
	{0,4,fd85,NULL,"add a,ly"},
	{0,4,fd86,NULL,"add a,(iy:4)"},
	{0,4,npr87,NULL,"add a,a"},

	{0,4,npr88,NULL,"adc a,b"},
	{0,4,npr89,NULL,"adc a,c"},
	{0,4,npr8A,NULL,"adc a,d"},
	{0,4,npr8B,NULL,"adc a,e"},
	{0,4,fd8C,NULL,"adc a,hy"},
	{0,4,fd8D,NULL,"adc a,ly"},
	{0,4,fd8E,NULL,"adc a,(iy:4)"},
	{0,4,npr8F,NULL,"adc a,a"},

	{0,4,npr90,NULL,"sub b"},
	{0,4,npr91,NULL,"sub c"},
	{0,4,npr92,NULL,"sub d"},
	{0,4,npr93,NULL,"sub e"},
	{0,4,fd94,NULL,"sub hy"},
	{0,4,fd95,NULL,"sub ly"},
	{0,4,fd96,NULL,"sub (iy:4)"},
	{0,4,npr97,NULL,"sub a"},

	{0,4,npr98,NULL,"sbc a,b"},
	{0,4,npr99,NULL,"sbc a,c"},
	{0,4,npr9A,NULL,"sbc a,d"},
	{0,4,npr9B,NULL,"sbc a,e"},
	{0,4,fd9C,NULL,"sbc a,hy"},
	{0,4,fd9D,NULL,"sbc a,ly"},
	{0,4,fd9E,NULL,"sbc a,(iy:4)"},
	{0,4,npr9F,NULL,"sbc a,a"},

	{0,4,nprA0,NULL,"and b"},
	{0,4,nprA1,NULL,"and c"},
	{0,4,nprA2,NULL,"and d"},
	{0,4,nprA3,NULL,"and e"},
	{0,4,fdA4,NULL,"and hy"},
	{0,4,fdA5,NULL,"and ly"},
	{0,4,fdA6,NULL,"and (iy:4)"},
	{0,4,nprA7,NULL,"and a"},

	{0,4,nprA8,NULL,"xor b"},
	{0,4,nprA9,NULL,"xor c"},
	{0,4,nprAA,NULL,"xor d"},
	{0,4,nprAB,NULL,"xor e"},
	{0,4,fdAC,NULL,"xor hy"},
	{0,4,fdAD,NULL,"xor ly"},
	{0,4,fdAE,NULL,"xor (iy:4)"},
	{0,4,nprAF,NULL,"xor a"},

	{0,4,nprB0,NULL,"or b"},
	{0,4,nprB1,NULL,"or c"},
	{0,4,nprB2,NULL,"or d"},
	{0,4,nprB3,NULL,"or e"},
	{0,4,fdB4,NULL,"or hy"},
	{0,4,fdB5,NULL,"or ly"},
	{0,4,fdB6,NULL,"or (iy:4)"},
	{0,4,nprB7,NULL,"or a"},

	{0,4,nprB8,NULL,"cp b"},
	{0,4,nprB9,NULL,"cp c"},
	{0,4,nprBA,NULL,"cp d"},
	{0,4,nprBB,NULL,"cp e"},
	{0,4,fdBC,NULL,"cp hy"},
	{0,4,fdBD,NULL,"cp ly"},
	{0,4,fdBE,NULL,"cp (iy:4)"},
	{0,4,nprBF,NULL,"cp a"},

	{0,5,nprC0,NULL,"ret nz"},		// 5 [3rd] [3rd]
	{0,4,nprC1,NULL,"pop bc"},
	{0,4,nprC2,NULL,"jp nz,:2"},
	{0,4,nprC3,NULL,"jp :2"},
	{OF_SKIPABLE,4,nprC4,NULL,"call nz,:2"},		// 4 3rd 3(4)rd [3wr] [3wr]
	{0,5,nprC5,NULL,"push bc"},		// 5 3wr 3wr
	{0,4,nprC6,NULL,"add a,:1"},
	{OF_SKIPABLE,5,nprC7,NULL,"rst #00"},		// 5 3wr 3wr

	{0,5,nprC8,NULL,"ret z"},
	{0,4,nprC9,NULL,"ret"},
	{0,4,nprCA,NULL,"jp z,:2"},
	{OF_PREFIX,4,fdCB,fdcbTab,"#CB"},
	{OF_SKIPABLE,4,nprCC,NULL,"call z,:2"},
	{OF_SKIPABLE,4,nprCD,NULL,"call :2"},		// 4 3rd 4rd 3wr 3wr
	{0,4,nprCE,NULL,"adc a,:1"},
	{OF_SKIPABLE,5,nprCF,NULL,"rst #08"},

	{0,5,nprD0,NULL,"ret nc"},
	{0,4,nprD1,NULL,"pop de"},
	{0,4,nprD2,NULL,"jp nc,:2"},
	{0,4,nprD3,NULL,"out (:1),a"},
	{OF_SKIPABLE,4,nprD4,NULL,"call nc,:2"},
	{0,5,nprD5,NULL,"push de"},
	{0,4,nprD6,NULL,"sub :1"},
	{OF_SKIPABLE,5,nprD7,NULL,"rst #10"},

	{0,5,nprD8,NULL,"ret c"},
	{0,4,nprD9,NULL,"exx"},
	{0,4,nprDA,NULL,"jp c,:2"},
	{0,4,nprDB,NULL,"in a,(:1)"},
	{OF_SKIPABLE,4,nprDC,NULL,"call c,:2"},
	{OF_PREFIX,4,nprDD,ddTab,"#DD"},
	{0,4,nprDE,NULL,"sbc a,:1"},
	{OF_SKIPABLE,5,nprDF,NULL,"rst #18"},

	{0,5,nprE0,NULL,"ret po"},
	{0,4,fdE1,NULL,"pop iy"},
	{0,4,nprE2,NULL,"jp po,:2"},
	{0,4,fdE3,NULL,"ex (sp),iy"},		// 4 3rd 4rd 3wr 5wr
	{OF_SKIPABLE,4,nprE4,NULL,"call po,:2"},
	{0,5,fdE5,NULL,"push iy"},
	{0,4,nprE6,NULL,"and :1"},
	{OF_SKIPABLE,5,nprE7,NULL,"rst #20"},

	{0,5,nprE8,NULL,"ret pe"},
	{0,4,fdE9,NULL,"jp (iy)"},
	{0,4,nprEA,NULL,"jp pe,:2"},
	{0,4,nprEB,NULL,"ex de,hl"},
	{OF_SKIPABLE,4,nprEC,NULL,"call pe,:2"},
	{OF_PREFIX,4,nprED,edTab,"#ED"},
	{0,4,nprEE,NULL,"xor :1"},
	{OF_SKIPABLE,5,nprEF,NULL,"rst #28"},

	{0,5,nprF0,NULL,"ret p"},
	{0,4,nprF1,NULL,"pop af"},
	{0,4,nprF2,NULL,"jp p,:2"},
	{0,4,nprF3,NULL,"di"},
	{OF_SKIPABLE,4,nprF4,NULL,"call p,:2"},
	{0,5,nprF5,NULL,"push af"},
	{0,4,nprF6,NULL,"or :1"},
	{OF_SKIPABLE,5,nprF7,NULL,"rst #30"},

	{0,5,nprF8,NULL,"ret m"},
	{0,6,fdF9,NULL,"ld sp,iy"},
	{0,4,nprFA,NULL,"jp m,:2"},
	{0,4,nprFB,NULL,"ei"},
	{OF_SKIPABLE,4,nprFC,NULL,"call m,:2"},
	{OF_PREFIX,4,nprFD,fdTab,"#FD"},
	{0,4,nprFE,NULL,"cp :1"},
	{OF_SKIPABLE,5,nprFF,NULL,"rst #38"}
};
