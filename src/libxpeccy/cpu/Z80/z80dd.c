#include <stdlib.h>
#include "z80.h"
#include "z80_macro.h"
#include "z80_nop.h"

extern opCode ddcbTab[256];
extern opCode fdTab[256];
extern opCode edTab[256];

// 09	add ix,bc	11	wz = ix+1 before adding
void dd09(CPU* cpu) {
	cpu->regIX = z80_add16(cpu, cpu->regIX, cpu->regBC);
}

// 19	add ix,de	11	wz = ix+1 before adding
void dd19(CPU* cpu) {
	cpu->regIX = z80_add16(cpu, cpu->regIX, cpu->regDE);
}

// 21	ld ix,nn	4 3rd 3rd
void dd21(CPU* cpu) {
	cpu->regIXl = z80_mrd(cpu, cpu->regPC++);
	cpu->regIXh = z80_mrd(cpu, cpu->regPC++);
}

// 22	ld (nn),ix	4 3rd 3rd 3wr 3wr	wz = nn+1
void dd22(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	z80_mwr(cpu, cpu->regWZ++, cpu->regIXl);
	z80_mwr(cpu, cpu->regWZ, cpu->regIXh);
}

// 23	inc ix		6
void dd23(CPU* cpu) {
	cpu->regIX++;
}

// 24	inc hx		4
void dd24(CPU* cpu) {
	cpu->regIXh = z80_inc8(cpu, cpu->regIXh);
}

// 25	dec hx		4
void dd25(CPU* cpu) {
	cpu->regIXh = z80_dec8(cpu, cpu->regIXh);
}

// 26	ld hx,n		4 3rd
void dd26(CPU* cpu) {
	cpu->regIXh = z80_mrd(cpu, cpu->regPC++);
}

// 29	add ix,ix	11
void dd29(CPU* cpu) {
	cpu->regIX = z80_add16(cpu, cpu->regIX, cpu->regIX);
}

// 2A	ld ix,(nn)	4 3rd 3rd 3rd 3rd	wz = nn+1
void dd2A(CPU* cpu) {
	cpu->regWZl = z80_mrd(cpu, cpu->regPC++);
	cpu->regWZh = z80_mrd(cpu, cpu->regPC++);
	cpu->regIXl = z80_mrd(cpu, cpu->regWZ++);
	cpu->regIXh = z80_mrd(cpu, cpu->regWZ);
}

// 2B	dec ix		6
void dd2B(CPU* cpu) {
	cpu->regIX--;
}

// 2C	inc lx		4
void dd2C(CPU* cpu) {
	cpu->regIXl = z80_inc8(cpu, cpu->regIXl);
}

// 2D	dec lx		4
void dd2D(CPU* cpu) {
	cpu->regIXl = z80_dec8(cpu, cpu->regIXl);
}

// 2E	ld lx,n		4 3rd
void dd2E(CPU* cpu) {
	cpu->regIXl = z80_mrd(cpu, cpu->regPC++);
}

// 34	inc (ix+e)	4 3rd 5add 4rd 3wr	wz = ix+e
void dd34(CPU* cpu) {
	RDSHIFT(cpu->regIX);
	cpu->tmp = z80_mrd(cpu, cpu->regWZ);
	cpu->t++;
	cpu->tmp = z80_inc8(cpu, cpu->tmp);
	z80_mwr(cpu, cpu->regWZ, cpu->tmp);
}

// 35	dec (ix+e)	4 3rd 5add 4rd 3wr	wz = ix+e
void dd35(CPU* cpu) {
	RDSHIFT(cpu->regIX);
	cpu->tmp = z80_mrd(cpu, cpu->regWZ);
	cpu->t++;
	cpu->tmp = z80_dec8(cpu, cpu->tmp);
	z80_mwr(cpu, cpu->regWZ, cpu->tmp);
}

// 36	ld (ix+e),n	4 3rd {5add 3rd} 3wr	wz = ix+e
void dd36(CPU* cpu) {
	RDSHIFT(cpu->regIX);
	cpu->tmp = z80_mrd(cpu, cpu->regPC++); cpu->t -= 3;
	z80_mwr(cpu, cpu->regWZ, cpu->tmp);
}

// 39	add ix,sp	11
void dd39(CPU* cpu) {
	cpu->regIX = z80_add16(cpu, cpu->regIX, cpu->regSP);
}

// ld r,r		4 [3rd 5add 3rd]
void dd44(CPU* cpu) {cpu->regB = cpu->regIXh;}
void dd45(CPU* cpu) {cpu->regB = cpu->regIXl;}
void dd46(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->regB = z80_mrd(cpu, cpu->regWZ);}
void dd4C(CPU* cpu) {cpu->regC = cpu->regIXh;}
void dd4D(CPU* cpu) {cpu->regC = cpu->regIXl;}
void dd4E(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->regC = z80_mrd(cpu, cpu->regWZ);}
void dd54(CPU* cpu) {cpu->regD = cpu->regIXh;}
void dd55(CPU* cpu) {cpu->regD = cpu->regIXl;}
void dd56(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->regD = z80_mrd(cpu, cpu->regWZ);}
void dd5C(CPU* cpu) {cpu->regE = cpu->regIXh;}
void dd5D(CPU* cpu) {cpu->regE = cpu->regIXl;}
void dd5E(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->regE = z80_mrd(cpu, cpu->regWZ);}

void dd60(CPU* cpu) {cpu->regIXh = cpu->regB;}
void dd61(CPU* cpu) {cpu->regIXh = cpu->regC;}
void dd62(CPU* cpu) {cpu->regIXh = cpu->regD;}
void dd63(CPU* cpu) {cpu->regIXh = cpu->regE;}
void dd64(CPU* cpu) {}
void dd65(CPU* cpu) {cpu->regIXh = cpu->regIXl;}
void dd66(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->regH = z80_mrd(cpu, cpu->regWZ);}
void dd67(CPU* cpu) {cpu->regIXh = cpu->regA;}

void dd68(CPU* cpu) {cpu->regIXl = cpu->regB;}
void dd69(CPU* cpu) {cpu->regIXl = cpu->regC;}
void dd6A(CPU* cpu) {cpu->regIXl = cpu->regD;}
void dd6B(CPU* cpu) {cpu->regIXl = cpu->regE;}
void dd6C(CPU* cpu) {cpu->regIXl = cpu->regIXh;}
void dd6D(CPU* cpu) {}
void dd6E(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->regL = z80_mrd(cpu, cpu->regWZ);}
void dd6F(CPU* cpu) {cpu->regIXl = cpu->regA;}
// 70..77	ld (ix+e),r	4 3rd 5add 3wr
void dd70(CPU* cpu) {RDSHIFT(cpu->regIX); z80_mwr(cpu, cpu->regWZ, cpu->regB);}
void dd71(CPU* cpu) {RDSHIFT(cpu->regIX); z80_mwr(cpu, cpu->regWZ, cpu->regC);}
void dd72(CPU* cpu) {RDSHIFT(cpu->regIX); z80_mwr(cpu, cpu->regWZ, cpu->regD);}
void dd73(CPU* cpu) {RDSHIFT(cpu->regIX); z80_mwr(cpu, cpu->regWZ, cpu->regE);}
void dd74(CPU* cpu) {RDSHIFT(cpu->regIX); z80_mwr(cpu, cpu->regWZ, cpu->regH);}
void dd75(CPU* cpu) {RDSHIFT(cpu->regIX); z80_mwr(cpu, cpu->regWZ, cpu->regL);}
void dd77(CPU* cpu) {RDSHIFT(cpu->regIX); z80_mwr(cpu, cpu->regWZ, cpu->regA);}

void dd7C(CPU* cpu) {cpu->regA = cpu->regIXh;}
void dd7D(CPU* cpu) {cpu->regA = cpu->regIXl;}
void dd7E(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->regA = z80_mrd(cpu, cpu->regWZ);}

// add x
void dd84(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regIXh, 0);}
void dd85(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regIXl, 0);}
void dd86(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->tmpb = z80_mrd(cpu, cpu->regWZ); cpu->regA = z80_add8(cpu, cpu->tmpb, 0);}
// adc x
void dd8C(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regIXh, cpu->flgC);}
void dd8D(CPU* cpu) {cpu->regA = z80_add8(cpu, cpu->regIXl, cpu->flgC);}
void dd8E(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->tmpb = z80_mrd(cpu, cpu->regWZ); cpu->regA = z80_add8(cpu, cpu->tmpb, cpu->flgC);}
// sub x
void dd94(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regIXh, 0);}
void dd95(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regIXl, 0);}
void dd96(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->tmpb = z80_mrd(cpu, cpu->regWZ); cpu->regA = z80_sub8(cpu, cpu->tmpb, 0);}
// sbc x
void dd9C(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regIXh, cpu->flgC);}
void dd9D(CPU* cpu) {cpu->regA = z80_sub8(cpu, cpu->regIXl, cpu->flgC);}
void dd9E(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->tmpb = z80_mrd(cpu, cpu->regWZ); cpu->regA = z80_sub8(cpu, cpu->tmpb, cpu->flgC);}
// and x
void ddA4(CPU* cpu) {z80_and8(cpu, cpu->regIXh);}
void ddA5(CPU* cpu) {z80_and8(cpu, cpu->regIXl);}
void ddA6(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->tmpb = z80_mrd(cpu, cpu->regWZ); z80_and8(cpu, cpu->tmpb);}
// xor x
void ddAC(CPU* cpu) {z80_xor8(cpu, cpu->regIXh);}
void ddAD(CPU* cpu) {z80_xor8(cpu, cpu->regIXl);}
void ddAE(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->tmpb = z80_mrd(cpu, cpu->regWZ); z80_xor8(cpu, cpu->tmpb);}
// or x
void ddB4(CPU* cpu) {z80_or8(cpu, cpu->regIXh);}
void ddB5(CPU* cpu) {z80_or8(cpu, cpu->regIXl);}
void ddB6(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->tmpb = z80_mrd(cpu, cpu->regWZ); z80_or8(cpu, cpu->tmpb);}
// cp x
void ddBC(CPU* cpu) {z80_cp8(cpu, cpu->regIXh);}
void ddBD(CPU* cpu) {z80_cp8(cpu, cpu->regIXl);}
void ddBE(CPU* cpu) {RDSHIFT(cpu->regIX); cpu->tmpb = z80_mrd(cpu, cpu->regWZ); z80_cp8(cpu, cpu->tmpb);}

// cb	ddcb prefix	4 3rd
void ddCB(CPU* cpu) {
	cpu->opTab = ddcbTab;
	cpu->tmp = z80_mrd(cpu, cpu->regPC++);		// shift
	cpu->com = z80_mrd(cpu, cpu->regPC++); cpu->t-=3;	// not M1, reading opcode & adding ix+e in parallel?
	cpu->op = &ddcbTab[cpu->com];
	cpu->op->exec(cpu);
}

// e1	pop ix		4 3rd 3rd
void ddE1(CPU* cpu) {
	cpu->regIX = z80_pop(cpu);
}

// e3	ex (sp),ix	4 3rd 4rd 3wr 5wr	wz = ix
void ddE3(CPU* cpu) {
	cpu->tmpw = z80_pop(cpu); cpu->t++;
	z80_push(cpu, cpu->regIX); cpu->t += 2;
	cpu->regIX = cpu->tmpw;
	cpu->regWZ = cpu->regIX;
}

// e5	push ix		5 3wr 3wr
void ddE5(CPU* cpu) {
	z80_push(cpu, cpu->regIX);
}

// e9	jp (ix)		4
void ddE9(CPU* cpu) {
	cpu->regPC = cpu->regIX;
}

// f9	ld sp,ix	6
void ddF9(CPU* cpu) {
	cpu->regSP = cpu->regIX;
}

// ======

opCode ddTab[256]={
	{0,4,npr00,NULL,"nop"},
	{0,4,npr01,NULL,"ld bc,:2"},
	{0,4,npr02,NULL,"ld (bc),a"},
	{0,6,npr03,NULL,"inc bc"},
	{0,4,npr04,NULL,"inc b"},
	{0,4,npr05,NULL,"dec b"},
	{0,4,npr06,NULL,"ld b,:1"},
	{0,4,npr07,NULL,"rlca"},

	{0,4,npr08,NULL,"ex af,af'"},
	{0,11,dd09,NULL,"add ix,bc"},
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
	{0,11,dd19,NULL,"add ix,de"},
	{0,4,npr1A,NULL,"ld a,(de)"},
	{0,6,npr1B,NULL,"dec de"},
	{0,4,npr1C,NULL,"inc e"},
	{0,4,npr1D,NULL,"dec e"},
	{0,4,npr1E,NULL,"ld e,:1"},
	{0,4,npr1F,NULL,"rra"},

	{OF_RELJUMP,4,npr20,NULL,"jr nz,:3"},
	{0,4,dd21,NULL,"ld ix,:2"},
	{OF_MWORD | OF_MEMADR,4,dd22,NULL,"ld (:2),ix"},		// 4,3rd,3rd,3wr,3wr
	{0,6,dd23,NULL,"inc ix"},
	{0,4,dd24,NULL,"inc hx"},
	{0,4,dd25,NULL,"dec hx"},
	{0,4,dd26,NULL,"ld hx,:1"},
	{0,4,npr27,NULL,"daa"},

	{OF_RELJUMP,4,npr28,NULL,"jr z,:3"},
	{0,11,dd29,NULL,"add ix,ix"},
	{OF_MWORD | OF_MEMADR,4,dd2A,NULL,"ld ix,(:2)"},		// 4,3rd,3rd,3rd,3rd
	{0,6,dd2B,NULL,"dec ix"},
	{0,4,dd2C,NULL,"inc lx"},
	{0,4,dd2D,NULL,"dec lx"},
	{0,4,dd2E,NULL,"ld lx,:1"},
	{0,4,npr2F,NULL,"cpl"},

	{OF_RELJUMP,4,npr30,NULL,"jr nc,:3"},
	{0,4,npr31,NULL,"ld sp,:2"},
	{OF_MEMADR,4,npr32,NULL,"ld (:2),a"},		// 4,3rd,3rd,3wr
	{0,6,npr33,NULL,"inc sp"},
	{0,4,dd34,NULL,"inc (ix:4)"},
	{0,4,dd35,NULL,"dec (ix:4)"},
	{0,4,dd36,NULL,"ld (ix:4),:1"},
	{0,4,npr37,NULL,"scf"},

	{OF_RELJUMP,4,npr38,NULL,"jr c,:3"},
	{0,11,dd39,NULL,"add ix,sp"},
	{OF_MEMADR,4,npr3A,NULL,"ld a,(:2)"},		// 4,3rd,3rd,3rd
	{0,6,npr3B,NULL,"dec sp"},
	{0,4,npr3C,NULL,"inc a"},
	{0,4,npr3D,NULL,"dec a"},
	{0,4,npr3E,NULL,"ld a,:1"},
	{0,4,npr3F,NULL,"ccf"},

	{0,4,npr40,NULL,"ld b,b"},
	{0,4,npr41,NULL,"ld b,c"},
	{0,4,npr42,NULL,"ld b,d"},
	{0,4,npr43,NULL,"ld b,e"},
	{0,4,dd44,NULL,"ld b,hx"},
	{0,4,dd45,NULL,"ld b,lx"},
	{0,4,dd46,NULL,"ld b,(ix:4)"},
	{0,4,npr47,NULL,"ld b,a"},

	{0,4,npr48,NULL,"ld c,b"},
	{0,4,npr49,NULL,"ld c,c"},
	{0,4,npr4A,NULL,"ld c,d"},
	{0,4,npr4B,NULL,"ld c,e"},
	{0,4,dd4C,NULL,"ld c,hx"},
	{0,4,dd4D,NULL,"ld c,lx"},
	{0,4,dd4E,NULL,"ld c,(ix:4)"},
	{0,4,npr4F,NULL,"ld c,a"},

	{0,4,npr50,NULL,"ld d,b"},
	{0,4,npr51,NULL,"ld d,c"},
	{0,4,npr52,NULL,"ld d,d"},
	{0,4,npr53,NULL,"ld d,e"},
	{0,4,dd54,NULL,"ld d,hx"},
	{0,4,dd55,NULL,"ld d,lx"},
	{0,4,dd56,NULL,"ld d,(ix:4)"},
	{0,4,npr57,NULL,"ld d,a"},

	{0,4,npr58,NULL,"ld e,b"},
	{0,4,npr59,NULL,"ld e,c"},
	{0,4,npr5A,NULL,"ld e,d"},
	{0,4,npr5B,NULL,"ld e,e"},
	{0,4,dd5C,NULL,"ld e,hx"},
	{0,4,dd5D,NULL,"ld e,lx"},
	{0,4,dd5E,NULL,"ld e,(ix:4)"},
	{0,4,npr5F,NULL,"ld e,a"},

	{0,4,dd60,NULL,"ld hx,b"},
	{0,4,dd61,NULL,"ld hx,c"},
	{0,4,dd62,NULL,"ld hx,d"},
	{0,4,dd63,NULL,"ld hx,e"},
	{0,4,dd64,NULL,"ld hx,hx"},
	{0,4,dd65,NULL,"ld hx,lx"},
	{0,4,dd66,NULL,"ld h,(ix:4)"},
	{0,4,dd67,NULL,"ld hx,a"},

	{0,4,dd68,NULL,"ld lx,b"},
	{0,4,dd69,NULL,"ld lx,c"},
	{0,4,dd6A,NULL,"ld lx,d"},
	{0,4,dd6B,NULL,"ld lx,e"},
	{0,4,dd6C,NULL,"ld lx,hx"},
	{0,4,dd6D,NULL,"ld lx,lx"},
	{0,4,dd6E,NULL,"ld l,(ix:4)"},
	{0,4,dd6F,NULL,"ld lx,a"},

	{0,4,dd70,NULL,"ld (ix:4),b"},
	{0,4,dd71,NULL,"ld (ix:4),c"},
	{0,4,dd72,NULL,"ld (ix:4),d"},
	{0,4,dd73,NULL,"ld (ix:4),e"},
	{0,4,dd74,NULL,"ld (ix:4),h"},
	{0,4,dd75,NULL,"ld (ix:4),l"},
	{0,4,npr76,NULL,"halt"},
	{0,4,dd77,NULL,"ld (ix:4),a"},

	{0,4,npr78,NULL,"ld a,b"},
	{0,4,npr79,NULL,"ld a,c"},
	{0,4,npr7A,NULL,"ld a,d"},
	{0,4,npr7B,NULL,"ld a,e"},
	{0,4,dd7C,NULL,"ld a,hx"},
	{0,4,dd7D,NULL,"ld a,lx"},
	{0,4,dd7E,NULL,"ld a,(ix:4)"},
	{0,4,npr7F,NULL,"ld a,a"},

	{0,4,npr80,NULL,"add a,b"},
	{0,4,npr81,NULL,"add a,c"},
	{0,4,npr82,NULL,"add a,d"},
	{0,4,npr83,NULL,"add a,e"},
	{0,4,dd84,NULL,"add a,hx"},
	{0,4,dd85,NULL,"add a,lx"},
	{0,4,dd86,NULL,"add a,(ix:4)"},
	{0,4,npr87,NULL,"add a,a"},

	{0,4,npr88,NULL,"adc a,b"},
	{0,4,npr89,NULL,"adc a,c"},
	{0,4,npr8A,NULL,"adc a,d"},
	{0,4,npr8B,NULL,"adc a,e"},
	{0,4,dd8C,NULL,"adc a,hx"},
	{0,4,dd8D,NULL,"adc a,lx"},
	{0,4,dd8E,NULL,"adc a,(ix:4)"},
	{0,4,npr8F,NULL,"adc a,a"},

	{0,4,npr90,NULL,"sub b"},
	{0,4,npr91,NULL,"sub c"},
	{0,4,npr92,NULL,"sub d"},
	{0,4,npr93,NULL,"sub e"},
	{0,4,dd94,NULL,"sub hx"},
	{0,4,dd95,NULL,"sub lx"},
	{0,4,dd96,NULL,"sub (ix:4)"},
	{0,4,npr97,NULL,"sub a"},

	{0,4,npr98,NULL,"sbc a,b"},
	{0,4,npr99,NULL,"sbc a,c"},
	{0,4,npr9A,NULL,"sbc a,d"},
	{0,4,npr9B,NULL,"sbc a,e"},
	{0,4,dd9C,NULL,"sbc a,hx"},
	{0,4,dd9D,NULL,"sbc a,lx"},
	{0,4,dd9E,NULL,"sbc a,(ix:4)"},
	{0,4,npr9F,NULL,"sbc a,a"},

	{0,4,nprA0,NULL,"and b"},
	{0,4,nprA1,NULL,"and c"},
	{0,4,nprA2,NULL,"and d"},
	{0,4,nprA3,NULL,"and e"},
	{0,4,ddA4,NULL,"and hx"},
	{0,4,ddA5,NULL,"and lx"},
	{0,4,ddA6,NULL,"and (ix:4)"},
	{0,4,nprA7,NULL,"and a"},

	{0,4,nprA8,NULL,"xor b"},
	{0,4,nprA9,NULL,"xor c"},
	{0,4,nprAA,NULL,"xor d"},
	{0,4,nprAB,NULL,"xor e"},
	{0,4,ddAC,NULL,"xor hx"},
	{0,4,ddAD,NULL,"xor lx"},
	{0,4,ddAE,NULL,"xor (ix:4)"},
	{0,4,nprAF,NULL,"xor a"},

	{0,4,nprB0,NULL,"or b"},
	{0,4,nprB1,NULL,"or c"},
	{0,4,nprB2,NULL,"or d"},
	{0,4,nprB3,NULL,"or e"},
	{0,4,ddB4,NULL,"or hx"},
	{0,4,ddB5,NULL,"or lx"},
	{0,4,ddB6,NULL,"or (ix:4)"},
	{0,4,nprB7,NULL,"or a"},

	{0,4,nprB8,NULL,"cp b"},
	{0,4,nprB9,NULL,"cp c"},
	{0,4,nprBA,NULL,"cp d"},
	{0,4,nprBB,NULL,"cp e"},
	{0,4,ddBC,NULL,"cp hx"},
	{0,4,ddBD,NULL,"cp lx"},
	{0,4,ddBE,NULL,"cp (ix:4)"},
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
	{OF_PREFIX,4,ddCB,ddcbTab,"#CB"},
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
	{0,4,ddE1,NULL,"pop ix"},
	{0,4,nprE2,NULL,"jp po,:2"},
	{0,4,ddE3,NULL,"ex (sp),ix"},		// 4 3rd 4rd 3wr 5wr
	{OF_SKIPABLE,4,nprE4,NULL,"call po,:2"},
	{0,5,ddE5,NULL,"push ix"},
	{0,4,nprE6,NULL,"and :1"},
	{OF_SKIPABLE,5,nprE7,NULL,"rst #20"},

	{0,5,nprE8,NULL,"ret pe"},
	{0,4,ddE9,NULL,"jp (ix)"},
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
	{0,6,ddF9,NULL,"ld sp,ix"},
	{0,4,nprFA,NULL,"jp m,:2"},
	{0,4,nprFB,NULL,"ei"},
	{OF_SKIPABLE,4,nprFC,NULL,"call m,:2"},
	{OF_PREFIX,4,nprFD,fdTab,"#FD"},
	{0,4,nprFE,NULL,"cp :1"},
	{OF_SKIPABLE,5,nprFF,NULL,"rst #38"}
};
