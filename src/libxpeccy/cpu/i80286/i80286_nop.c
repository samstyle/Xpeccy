#include "i80286.h"

unsigned char i286_mrd(CPU* cpu, int adr) {
	cpu->t++;
	return cpu->mrd(adr, 0, cpu->data) & 0xff;
}

void i286_mwr(CPU* cpu, int adr, int val) {
	cpu->t++;
	cpu->mwr(adr, val, cpu->data);
}

unsigned char i286_get_imm_byte(CPU* cpu) {
	unsigned char res = i286_mrd(cpu, (cpu->cs << 4) + cpu->pc);
	cpu->pc++;
	return res;
}

void i286_push(CPU* cpu, unsigned short w) {
	cpu->sp--;
	i286_mwr(cpu, (cpu->ss << 4) + cpu->sp, (w >> 8) & 0xff);
	cpu->sp--;
	i286_mwr(cpu, (cpu->ss << 4) + cpu->sp, w & 0xff);
}

unsigned short i286_pop(CPU* cpu) {
	unsigned short res = i286_mrd(cpu, (cpu->ss << 4) + cpu->sp) & 0xff;
	cpu->sp++;
	res |= (i286_mrd(cpu, (cpu->ss << 4) + cpu->sp) << 8) & 0xff00;
	cpu->sp++;
	return res;
}

void i286_interrupt(CPU* cpu, int n) {
	i286_push(cpu, cpu->flag);
	i286_push(cpu, cpu->pc);
	i286_push(cpu, cpu->cs);
	cpu->flag &= ~(I286_FI | I286_FT);
	cpu->tmpi = n << 2;
	cpu->ltw = i286_mrd(cpu, cpu->tmpi++);	// tmpw
	cpu->htw = i286_mrd(cpu, cpu->tmpi++);
	cpu->lwr = i286_mrd(cpu, cpu->tmpi++);	// twrd
	cpu->hwr = i286_mrd(cpu, cpu->tmpi);
	cpu->cs = cpu->tmpw;	// TODO: or opposite?
	cpu->pc = cpu->twrd;
}

int i286_get_reg(CPU* cpu, int wrd) {
	int res = -1;
	if (wrd) {
		switch((cpu->mod >> 3) & 7) {
			case 0: res = cpu->al; break;
			case 1: res = cpu->cl; break;
			case 2: res = cpu->dl; break;
			case 3: res = cpu->bl; break;
			case 4: res = cpu->ah; break;
			case 5: res = cpu->ch; break;
			case 6: res = cpu->dh; break;
			case 7: res = cpu->bh; break;
		}
		res &= 0xff;
	} else {
		switch((cpu->mod >> 3) & 7) {
			case 0: res = cpu->ax; break;
			case 1: res = cpu->cx; break;
			case 2: res = cpu->dx; break;
			case 3: res = cpu->bx; break;
			case 4: res = cpu->sp; break;
			case 5: res = cpu->bp; break;
			case 6: res = cpu->si; break;
			case 7: res = cpu->di; break;
		}
		res &= 0xffff;
	}
	return res;
}

void i286_set_reg(CPU* cpu, int val, int wrd) {
	if (wrd) {
		val &= 0xffff;
		switch((cpu->mod >> 3) & 7) {
			case 0: cpu->ax = val; break;
			case 1: cpu->cx = val; break;
			case 2: cpu->dx = val; break;
			case 3: cpu->bx = val; break;
			case 4: cpu->sp = val; break;
			case 5: cpu->bp = val; break;
			case 6: cpu->si = val; break;
			case 7: cpu->di = val; break;
		}
	} else {
		val &= 0xff;
		switch((cpu->mod >> 3) & 7) {
			case 0: cpu->al = val; break;
			case 1: cpu->cl = val; break;
			case 2: cpu->dl = val; break;
			case 3: cpu->bl = val; break;
			case 4: cpu->ah = val; break;
			case 5: cpu->ch = val; break;
			case 6: cpu->dh = val; break;
			case 7: cpu->bh = val; break;
		}
	}
}

// read mod, calculate effective address in cpu->tmpi, read byte/word from EA to cpu->tmpw, set register N to cpu->twrd
// modbyte: [7.6:mod][5.4.3:regN][2.1.0:adr/reg]
void i286_rd_ea(CPU* cpu, int wrd) {
	cpu->tmpw = 0;	// = disp
	cpu->mod = i286_get_imm_byte(cpu);
	cpu->twrd = i286_get_reg(cpu, wrd);
	if ((cpu->mod & 0xc0) == 0x40) {
		cpu->ltw = i286_get_imm_byte(cpu);
		cpu->htw = (cpu->ltw & 0x80) ? 0xff : 0x00;
	} else if ((cpu->mod & 0xc0) == 0x80) {
		cpu->ltw = i286_get_imm_byte(cpu);
		cpu->htw = i286_get_imm_byte(cpu);
	}
	if ((cpu->mod & 0xc0) == 0xc0) {
		if (wrd) {
			switch(cpu->mod & 7) {
				case 0: cpu->tmpw = cpu->ax; break;
				case 1: cpu->tmpw = cpu->cx; break;
				case 2: cpu->tmpw = cpu->dx; break;
				case 3: cpu->tmpw = cpu->bx; break;
				case 4: cpu->tmpw = cpu->sp; break;
				case 5: cpu->tmpw = cpu->bp; break;
				case 6: cpu->tmpw = cpu->si; break;
				case 7: cpu->tmpw = cpu->di; break;
			}
		} else {
			cpu->htw = 0;
			switch(cpu->mod & 7) {
				case 0: cpu->ltw = cpu->al; break;
				case 1: cpu->ltw = cpu->cl; break;
				case 2: cpu->ltw = cpu->dl; break;
				case 3: cpu->ltw = cpu->bl; break;
				case 4: cpu->ltw = cpu->ah; break;
				case 5: cpu->ltw = cpu->ch; break;
				case 6: cpu->ltw = cpu->dh; break;
				case 7: cpu->ltw = cpu->bh; break;
			}
		}
		cpu->tmpi = -1;		// reg
	} else {
		switch(cpu->mod & 0x07) {
			case 0: cpu->tmpi = cpu->bx + cpu->si + cpu->tmpw;
				if (cpu->seg < 0) cpu->seg = cpu->ds;
				break;
			case 1: cpu->tmpi = cpu->bx + cpu->di + cpu->tmpw;
				if (cpu->seg < 0) cpu->seg = cpu->ds;
				break;
			case 2: cpu->tmpi = cpu->bp + cpu->si + cpu->tmpw;
				if (cpu->seg < 0) cpu->seg = cpu->ss;
				break;
			case 3: cpu->tmpi = cpu->bp + cpu->di + cpu->tmpw;
				if (cpu->seg < 0) cpu->seg = cpu->ss;
				break;
			case 4: cpu->tmpi = cpu->si + cpu->tmpw;
				if (cpu->seg < 0) cpu->seg = cpu->ds;
				break;
			case 5: cpu->tmpi = cpu->di + cpu->tmpw;
				if (cpu->seg < 0) cpu->seg = cpu->ds;	// TODO: or es in some opcodes (not overrideable)
				break;
			case 6:	if (cpu->mod & 0xc0) {
					cpu->tmpi = cpu->bp + cpu->tmpw;
					if (cpu->seg < 0) cpu->seg = cpu->ss;
				} else {
					cpu->tmpi = cpu->tmpw;
					if (cpu->seg < 0) cpu->seg = cpu->ds;
				}
				break;
			case 7: cpu->tmpi = cpu->bx + cpu->tmpw;
				if (cpu->seg < 0) cpu->seg = cpu->ds;
				break;
		}
		cpu->tmpi = 0;		// memory address
		cpu->ea.seg = cpu->seg;
		cpu->ea.adr = cpu->tmpi & 0xffff;
		cpu->ltw = i286_mrd(cpu, (cpu->ea.seg << 4) + cpu->ea.adr);
		cpu->htw = wrd ? i286_mrd(cpu, (cpu->ea.seg << 4) + cpu->ea.adr + 1) : 0;
	}
}

// must be called after i286_rd_ea, cpu->tmpi must be calculated, cpu->mod setted
void i286_wr_ea(CPU* cpu, int val, int wrd) {
	if (cpu->tmpi < 0) {		// this is a reg
		if (wrd) {
			switch(cpu->mod & 7) {
				case 0: cpu->ax = val & 0xffff; break;
				case 1: cpu->cx = val & 0xffff; break;
				case 2: cpu->dx = val & 0xffff; break;
				case 3: cpu->bx = val & 0xffff; break;
				case 4: cpu->sp = val & 0xffff; break;
				case 5: cpu->bp = val & 0xffff; break;
				case 6: cpu->si = val & 0xffff; break;
				case 7: cpu->di = val & 0xffff; break;
			}
		} else {
			switch(cpu->mod & 7) {
				case 0: cpu->al = val & 0xff; break;
				case 1: cpu->cl = val & 0xff; break;
				case 2: cpu->dl = val & 0xff; break;
				case 3: cpu->bl = val & 0xff; break;
				case 4: cpu->ah = val & 0xff; break;
				case 5: cpu->ch = val & 0xff; break;
				case 6: cpu->dh = val & 0xff; break;
				case 7: cpu->bh = val & 0xff; break;
			}
		}
	} else {
		i286_mwr(cpu, (cpu->ea.seg << 4) + cpu->ea.adr, val & 0xff);
		if (wrd) i286_mwr(cpu, (cpu->ea.seg << 4) + cpu->ea.adr + 1, (val >> 8) & 0xff);
	}
}

// add/adc

static const int i286_add_FO[8] = {0, 0, 0, 1, 1, 0, 0, 0};

unsigned short i286_add8(CPU* cpu, unsigned char p1, unsigned char p2, int rf) {
	cpu->flag &= ~(I286_FS | I286_FZ | I286_FP);
	if (rf) cpu->flag &= ~(I286_FO | I286_FC | I286_FA);
	unsigned short res = p1 + p2;
	cpu->tmp = ((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6) | ((res & 0x80) >> 5);
	if (i286_add_FO[cpu->tmp]) cpu->flag |= I286_FO;
	if (res & 0x80) cpu->flag |= I286_FS;
	if (!(res & 0xff)) cpu->flag |= I286_FZ;
	if ((p1 & 15) + (p2 & 15) > 15) cpu->flag |= I286_FA;
	if (parity(res & 0xff)) cpu->flag |= I286_FP;
	if (res > 0xff) cpu->flag |= I286_FC;
	return res & 0xff;
}

unsigned short i286_add16(CPU* cpu, unsigned short p1, unsigned short p2, int rf) {
	cpu->flag &= ~(I286_FS | I286_FZ | I286_FP);
	if (rf) cpu->flag &= ~(I286_FO | I286_FC | I286_FA);
	int res = p1 + p2;
	cpu->tmp = ((p1 & 0x8000) >> 15) | ((p2 & 0x8000) >> 14) | ((res & 0x8000) >> 13);
	if (i286_add_FO[cpu->tmp]) cpu->flag |= I286_FO;
	if (res & 0x8000) cpu->flag |= I286_FS;
	if (!(res & 0xffff)) cpu->flag |= I286_FZ;
	if ((p1 & 0xfff) + (p2 & 0xfff) > 0xfff) cpu->flag |= I286_FA;
	if (parity(res & 0xffff)) cpu->flag |= I286_FP;
	if (res > 0xffff) cpu->flag |= I286_FC;
	return res & 0xffff;
}

// 00,mod: add eb,rb	EA.byte += reg.byte
void i286_op00(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->tmpw = i286_add8(cpu, cpu->ltw, cpu->lwr, 1);
	i286_wr_ea(cpu, cpu->tmpw, 0);
}

// 01,mod: add ew,rw	EA.word += reg.word
void i286_op01(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_add16(cpu, cpu->tmpw, cpu->twrd, 1);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// 02,mod: add rb,eb	reg.byte += EA.byte
void i286_op02(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->tmpw = i286_add8(cpu, cpu->ltw, cpu->lwr, 1);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 03,mod: add rw,ew	reg.word += EA.word
void i286_op03(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_add16(cpu, cpu->tmpw, cpu->twrd, 1);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 04,db: add AL,db	AL += db
void i286_op04(CPU* cpu) {
	cpu->lwr = i286_get_imm_byte(cpu);
	cpu->al = i286_add8(cpu, cpu->al, cpu->lwr, 1);
}

// 05,dw: add AX,dw	AX += dw
void i286_op05(CPU* cpu) {
	cpu->lwr = i286_get_imm_byte(cpu);
	cpu->hwr = i286_get_imm_byte(cpu);
	cpu->ax = i286_add16(cpu, cpu->ax, cpu->twrd, 1);
}

// 06: push es
void i286_op06(CPU* cpu) {
	cpu->tmpw = cpu->es;
	i286_push(cpu, cpu->tmpw);
}

// 07: pop es
void i286_op07(CPU* cpu) {
	cpu->es = i286_pop(cpu);
}

// or

unsigned char i286_or8(CPU* cpu, unsigned char p1, unsigned char p2) {
	unsigned char res =  p1 | p2;
	cpu->flag &= ~(I286_FO | I286_FS | I286_FZ | I286_FP | I286_FC);
	if (res & 0x80) cpu->flag |= I286_FS;
	if (!res) cpu->flag |= I286_FZ;
	if (parity(res)) cpu->flag |= I286_FP;
	return res;
}

unsigned short i286_or16(CPU* cpu, unsigned short p1, unsigned short p2) {
	unsigned short res =  p1 | p2;
	cpu->flag &= ~(I286_FO | I286_FS | I286_FZ | I286_FP | I286_FC);
	if (res & 0x8000) cpu->flag |= I286_FS;
	if (!res) cpu->flag |= I286_FZ;
	if (parity(res)) cpu->flag |= I286_FP;
	return res;
}

// 08,mod: or eb,rb
void i286_op08(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_or8(cpu, cpu->ltw, cpu->lwr);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// 09,mod: or ew,rw
void i286_op09(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_or16(cpu, cpu->tmpw, cpu->twrd);
	i286_wr_ea(cpu, cpu->ltw, 1);
}

// 0a,mod: or rb,eb
void i286_op0A(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_or8(cpu, cpu->ltw, cpu->lwr);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 0b,mod: or rw,ew
void i286_op0B(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_or16(cpu, cpu->tmpw, cpu->twrd);
	i286_set_reg(cpu, cpu->ltw, 1);
}

// 0c,db: or al,db
void i286_op0C(CPU* cpu) {
	cpu->tmp = i286_get_imm_byte(cpu);
	cpu->al = i286_or8(cpu, cpu->al, cpu->tmp);
}

// 0d,dw: or ax,dw
void i286_op0D(CPU* cpu) {
	cpu->lwr = i286_get_imm_byte(cpu);
	cpu->hwr = i286_get_imm_byte(cpu);
	cpu->ax = i286_or16(cpu, cpu->ax, cpu->twrd);
}

// 0e: push cs
void i286_op0E(CPU* cpu) {
	i286_push(cpu, cpu->cs);
}

// 0f: prefix
void i286_op0F(CPU* cpu) {
	// TODO: cpu->opTab = i286_tab_of;
}

// 10,mod: adc eb,rb
void i286_op10(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->tmpb = cpu->flag & I286_FC;
	cpu->tmpw = i286_add8(cpu, cpu->ltw, cpu->lwr, 1);
	if (cpu->tmpb)
		cpu->tmpw = i286_add8(cpu, cpu->ltw, 1, 0);	// add 1 and not reset flags
	i286_wr_ea(cpu, cpu->tmpw, 0);
}

// 11,mod: adc ew,rw
void i286_op11(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpb = cpu->flag & I286_FC;
	cpu->tmpw = i286_add16(cpu, cpu->tmpw, cpu->twrd, 1);
	if (cpu->tmpb)
		cpu->tmpw = i286_add16(cpu, cpu->tmpw, 1, 0);
	i286_wr_ea(cpu, cpu->tmpw, 0);
}

// 12,mod: adc rb,eb
void i286_op12(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->tmpb = cpu->flag & I286_FC;
	cpu->tmpw = i286_add8(cpu, cpu->ltw, cpu->lwr, 1);
	if (cpu->tmpb)
		cpu->tmpw = i286_add8(cpu, cpu->ltw, 1, 0);	// add 1 and not reset flags
	i286_set_reg(cpu, cpu->tmpw, 0);
}

// 13,mod: adc rw,ew
void i286_op13(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpb = cpu->flag & I286_FC;
	cpu->tmpw = i286_add16(cpu, cpu->tmpw, cpu->twrd, 1);
	if (cpu->tmpb)
		cpu->tmpw = i286_add16(cpu, cpu->tmpw, 1, 0);
	i286_set_reg(cpu, cpu->tmpw, 0);
}

// 14,db: adc al,db
void i286_op14(CPU* cpu) {
	cpu->lwr = i286_get_imm_byte(cpu);
	cpu->tmpb = cpu->flag & I286_FC;
	cpu->al = i286_add8(cpu, cpu->al, cpu->lwr, 1);
	if (cpu->tmpb) cpu->al = i286_add8(cpu, cpu->al, 1, 0);
}

// 15,dw: adc ax,dw
void i286_op15(CPU* cpu) {
	cpu->lwr = i286_get_imm_byte(cpu);
	cpu->hwr = i286_get_imm_byte(cpu);
	cpu->tmpb = cpu->flag & I286_FC;
	cpu->ax = i286_add16(cpu, cpu->ax, cpu->twrd, 1);
	if (cpu->tmpb) cpu->ax = i286_add16(cpu, cpu->ax, 1, 0);
}

// 16: push ss
void i286_op16(CPU* cpu) {
	i286_push(cpu, cpu->ss);
}

// 17: pop ss
void i286_op17(CPU* cpu) {
	cpu->ss = i286_pop(cpu);
}

// sub/sbc

static const int i286_sub_FO[8] = {0, 1, 0, 0, 0, 0, 1, 0};

unsigned char i286_sub8(CPU* cpu, unsigned char p1, unsigned char p2, int rf) {
	cpu->flag &= ~(I286_FS | I286_FZ | I286_FP);
	if (rf) cpu->flag &= ~(I286_FO | I286_FC | I286_FA);
	unsigned short res = p1 - p2;
	cpu->tmp = ((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6) | ((res & 0x80) >> 5);
	if (i286_sub_FO[cpu->tmp & 7]) cpu->flag |= I286_FO;
	if (res & 0x80) cpu->flag |= I286_FS;
	if (!(res & 0xff)) cpu->flag |= I286_FZ;
	if (parity(res & 0xff)) cpu->flag |= I286_FP;
	if (res & 0xff00) cpu->flag |= I286_FC;
	if ((p1 & 0x0f) < (p2 & 0x0f)) cpu->flag |= I286_FA;
	return res & 0xff;
}

unsigned short i286_sub16(CPU* cpu, unsigned short p1, unsigned short p2, int rf) {
	cpu->flag &= ~(I286_FS | I286_FZ | I286_FP);
	if (rf) cpu->flag &= ~(I286_FO | I286_FC | I286_FA);
	int res = p1 - p2;
	cpu->tmp = ((p1 & 0x8000) >> 15) | ((p2 & 0x8000) >> 14) | ((res & 0x8000) >> 13);
	if (i286_sub_FO[cpu->tmp & 7]) cpu->flag |= I286_FO;
	if (res & 0x8000) cpu->flag |= I286_FS;
	if (!(res & 0xffff)) cpu->flag |= I286_FZ;
	if (parity(res & 0xffff)) cpu->flag |= I286_FP;
	if (p2 > p1) cpu->flag |= I286_FC;
	if ((p1 & 0x0fff) < (p2 & 0x0fff)) cpu->flag |= I286_FA;
	return res & 0xffff;
}

// 18,mod: sbb eb,rb	NOTE: sbc
void i286_op18(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->tmpb = cpu->flag & I286_FC;
	cpu->ltw = i286_sub8(cpu, cpu->ltw, cpu->lwr, 1);
	if (cpu->tmpb) cpu->ltw = i286_sub8(cpu, cpu->ltw, 1, 0);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// 19,mod: sbb ew,rw
void i286_op19(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpb = cpu->flag & I286_FC;
	cpu->tmpw = i286_sub16(cpu, cpu->tmpw, cpu->twrd, 1);
	if (cpu->tmpb) cpu->tmpw = i286_sub16(cpu, cpu->tmpw, 1, 0);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// 1a,mod: sbb rb,eb
void i286_op1A(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->tmpb = cpu->flag & I286_FC;
	cpu->ltw = i286_sub8(cpu, cpu->lwr, cpu->ltw, 1);
	if (cpu->tmpb) cpu->ltw = i286_sub8(cpu, cpu->ltw, 1, 0);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 1b,mod: sbb rw,ew
void i286_op1B(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpb = cpu->flag & I286_FC;
	cpu->tmpw = i286_sub16(cpu, cpu->twrd, cpu->tmpw, 1);
	if (cpu->tmpb) cpu->tmpw = i286_sub16(cpu, cpu->tmpw, 1, 0);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 1c,db: sbb al,db
void i286_op1C(CPU* cpu) {
	cpu->lwr = i286_get_imm_byte(cpu);
	cpu->tmpb = cpu->flag & I286_FC;
	cpu->al = i286_sub8(cpu, cpu->al, cpu->lwr, 1);
	if (cpu->tmpb) cpu->al = i286_sub8(cpu, cpu->al, 1, 0);
}

// 1d,dw: sbb ax,dw
void i286_op1D(CPU* cpu) {
	cpu->lwr = i286_get_imm_byte(cpu);
	cpu->hwr = i286_get_imm_byte(cpu);
	cpu->tmpb = cpu->flag & I286_FC;
	cpu->ax = i286_sub16(cpu, cpu->ax, cpu->twrd, 1);
	if (cpu->tmpb) cpu->ax = i286_sub16(cpu, cpu->ax, 1, 0);
}

// 1e: push ds
void i286_op1E(CPU* cpu) {
	i286_push(cpu, cpu->ds);
}

// 1f: pop ds
void i286_op1F(CPU* cpu) {
	cpu->ds = i286_pop(cpu);
}

// and

unsigned char i286_and8(CPU* cpu, unsigned char p1, unsigned char p2) {
	cpu->flag &= ~(I286_FO | I286_FS | I286_FP | I286_FZ | I286_FC);
	p1 &= p2;
	if (p1 & 0x80) cpu->flag |= I286_FS;
	if (!p1) cpu->flag |= I286_FZ;
	if (parity(p1)) cpu->flag |= I286_FP;
	return p1;
}

unsigned short i286_and16(CPU* cpu, unsigned short p1, unsigned short p2) {
	cpu->flag &= ~(I286_FO | I286_FS | I286_FP | I286_FZ | I286_FC);
	p1 &= p2;
	if (p1 & 0x8000) cpu->flag |= I286_FS;
	if (!p1) cpu->flag |= I286_FZ;
	if (parity(p1)) cpu->flag |= I286_FP;
	return p1;
}

// 20,mod: and eb,rb
void i286_op20(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_and8(cpu, cpu->ltw, cpu->lwr);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// 21,mod: and ew,rw
void i286_op21(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_and16(cpu, cpu->tmpw, cpu->twrd);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// 22,mod: and rb,eb
void i286_op22(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_and8(cpu, cpu->ltw, cpu->lwr);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 23,mod: and rw,ew
void i286_op23(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_and16(cpu, cpu->tmpw, cpu->twrd);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 24,db: and al,db
void i286_op24(CPU* cpu) {
	cpu->ltw = i286_get_imm_byte(cpu);
	cpu->al = i286_and8(cpu, cpu->al, cpu->ltw);
}

// 25,dw: and ax,dw
void i286_op25(CPU* cpu) {
	cpu->ltw = i286_get_imm_byte(cpu);
	cpu->htw = i286_get_imm_byte(cpu);
	cpu->ax = i286_and16(cpu, cpu->ax, cpu->tmpw);
}

// 26: ES segment override prefix
void i286_op26(CPU* cpu) {
	cpu->seg = cpu->es;
}

// 27: daa
void i286_op27(CPU* cpu) {
	if (((cpu->al & 0x0f) > 9) || (cpu->flag & I286_FA)) {
		cpu->al += 6;
		cpu->flag |= I286_FA;
	} else {
		cpu->flag &= ~I286_FA;
	}
	if ((cpu->al > 0x9f) || (cpu->flag & I286_FC)) {
		cpu->al += 0x60;
		cpu->flag |= I286_FC;
	} else {
		cpu->flag &= ~I286_FC;
	}
	cpu->flag &= ~(I286_FS | I286_FZ | I286_FP);
	if (cpu->al & 0x80) cpu->flag |= I286_FS;
	if (!cpu->al) cpu->flag |= I286_FZ;
	if (parity(cpu->al)) cpu->flag |= I286_FP;
}

// 28,mod: sub eb,rb
void i286_op28(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_sub8(cpu, cpu->ltw, cpu->lwr, 1);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// 29,mod: sub ew,rw
void i286_op29(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_sub16(cpu, cpu->tmpw, cpu->twrd, 1);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// 2a,mod: sub rb,eb
void i286_op2A(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_sub8(cpu, cpu->lwr, cpu->ltw, 1);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 2b,mod: sub rw,ew
void i286_op2B(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_sub16(cpu, cpu->twrd, cpu->tmpw, 1);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 2c,db: sub al,db
void i286_op2C(CPU* cpu) {
	cpu->ltw = i286_get_imm_byte(cpu);
	cpu->al =  i286_sub8(cpu, cpu->al, cpu->ltw, 1);
}

// 2d,dw: sub ax,dw
void i286_op2D(CPU* cpu) {
	cpu->ltw = i286_get_imm_byte(cpu);
	cpu->htw = i286_get_imm_byte(cpu);
	cpu->ax =  i286_sub16(cpu, cpu->ax, cpu->tmpw, 1);
}

// 2e: CS segment override prefix
void i286_op2E(CPU* cpu) {
	cpu->seg = cpu->cs;
}

// 2f: das
void i286_op2F(CPU* cpu) {
	if (((cpu->al & 15) > 9) || (cpu->flag & I286_FA)) {
		cpu->al -= 9;
		cpu->flag |= I286_FA;
	} else {
		cpu->flag &= ~I286_FA;
	}
	if ((cpu->al > 0x9f) || (cpu->flag & I286_FC)) {
		cpu->al -= 0x60;
		cpu->flag |= I286_FC;
	} else {
		cpu->flag &= ~I286_FC;
	}
	cpu->flag &= ~(I286_FS | I286_FZ | I286_FP);
	if (cpu->al & 0x80) cpu->flag |= I286_FS;
	if (!cpu->al) cpu->flag |= I286_FZ;
	if (parity(cpu->al)) cpu->flag |= I286_FP;
}

// xor

unsigned char i286_xor8(CPU* cpu, unsigned char p1, unsigned char p2) {
	p1 ^= p2;
	cpu->flag &= ~(I286_FO | I286_FS | I286_FZ | I286_FP | I286_FC);
	if (p1 & 0x80) cpu->flag |= I286_FS;
	if (!p1) cpu->flag |= I286_FZ;
	if (parity(p1)) cpu->flag |= I286_FP;
	return p1;
}

unsigned short i286_xor16(CPU* cpu, unsigned short p1, unsigned short p2) {
	p1 ^= p2;
	cpu->flag &= ~(I286_FO | I286_FS | I286_FZ | I286_FP | I286_FC);
	if (p1 & 0x8000) cpu->flag |= I286_FS;
	if (!p1) cpu->flag |= I286_FZ;
	if (parity(p1)) cpu->flag |= I286_FP;
	return p1;
}

// 30,mod: xor eb,rb
void i286_op30(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_xor8(cpu, cpu->ltw, cpu->lwr);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// 31,mod: xor ew,rw
void i286_op31(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_xor16(cpu, cpu->tmpw, cpu->twrd);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// 32,mod: xor rb,eb
void i286_op32(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_xor8(cpu, cpu->ltw, cpu->lwr);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 33,mod: xor rw,ew
void i286_op33(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_xor16(cpu, cpu->tmpw, cpu->twrd);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 34,db: xor al,db
void i286_op34(CPU* cpu) {
	cpu->ltw = i286_get_imm_byte(cpu);
	cpu->al = i286_xor8(cpu, cpu->al, cpu->ltw);
}

// 35,dw: xor ax,dw
void i286_op35(CPU* cpu) {
	cpu->ltw = i286_get_imm_byte(cpu);
	cpu->htw = i286_get_imm_byte(cpu);
	cpu->ax = i286_xor16(cpu, cpu->ax, cpu->tmpw);
}

// 36: SS segment override prefix
void i286_op36(CPU* cpu) {
	cpu->seg = cpu->ss;
}

// 37: aaa
void i286_op37(CPU* cpu) {
	if (((cpu->al & 0x0f) > 0x09) || (cpu->flag & I286_FA)) {
		cpu->al += 6;
		cpu->ah++;
		cpu->flag |= (I286_FA | I286_FC);
	} else {
		cpu->flag &= ~(I286_FA | I286_FC);
	}
}

// 38,mod: cmp eb,rb
void i286_op38(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->tmp = i286_sub8(cpu, cpu->ltw, cpu->lwr, 1);
}

// 39,mod: cmp ew,rw
void i286_op39(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_sub16(cpu, cpu->tmpw, cpu->twrd, 1);
}

// 3a,mod: cmp rb,eb
void i286_op3A(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->tmp = i286_sub8(cpu, cpu->lwr, cpu->ltw, 1);
}

// 3b,mod: cmp rw,ew
void i286_op3B(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_sub16(cpu, cpu->twrd, cpu->tmpw, 1);
}

// 3c,db: cmp al,db
void i286_op3C(CPU* cpu) {
	cpu->ltw = i286_get_imm_byte(cpu);
	cpu->ltw = i286_sub8(cpu, cpu->al, cpu->ltw, 1);
}

// 3d,dw: cmp ax,dw
void i286_op3D(CPU* cpu) {
	cpu->ltw = i286_get_imm_byte(cpu);
	cpu->htw = i286_get_imm_byte(cpu);
	cpu->tmpw = i286_sub16(cpu, cpu->ax, cpu->tmpw, 1);
}

// 3e: DS segment override prefix
void i286_op3E(CPU* cpu) {
	cpu->seg = cpu->ds;
}

// 3f: aas
void i286_op3F(CPU* cpu) {
	if (((cpu->al & 15) > 9) | (cpu->flag & I286_FA)) {
		cpu->al -= 6;
		cpu->ah--;
		cpu->flag |= (I286_FA | I286_FC);
	} else {
		cpu->flag &= ~(I286_FA | I286_FC);
	}
	cpu->al &= 15;
}

// inc16
unsigned short i286_inc16(CPU* cpu, unsigned short r) {
	r++;
	cpu->flag &= ~(I286_FO | I286_FS | I286_FZ | I286_FA | I286_FP);
	if (r == 0x80) cpu->flag |= I286_FO;
	if (r & 0x80) cpu->flag |= I286_FS;
	if (!r) cpu->flag |= I286_FZ;
	if ((r & 15) == 0) cpu->flag |= I286_FA;
	if (parity(r)) cpu->flag |= I286_FP;
	return r;
}

// 40: inc ax
void i286_op40(CPU* cpu) {
	cpu->ax = i286_inc16(cpu, cpu->ax);
}

// 41: inc cx
void i286_op41(CPU* cpu) {
	cpu->cx = i286_inc16(cpu, cpu->cx);
}

// 42: inc dx
void i286_op42(CPU* cpu) {
	cpu->dx = i286_inc16(cpu, cpu->dx);
}

// 43: inc bx
void i286_op43(CPU* cpu) {
	cpu->bx = i286_inc16(cpu, cpu->bx);
}

// 44: inc sp
void i286_op44(CPU* cpu) {
	cpu->sp = i286_inc16(cpu, cpu->sp);
}

// 45: inc bp
void i286_op45(CPU* cpu) {
	cpu->bp = i286_inc16(cpu, cpu->bp);
}

// 46: inc si
void i286_op46(CPU* cpu) {
	cpu->si = i286_inc16(cpu, cpu->si);
}

// 47: inc di
void i286_op47(CPU* cpu) {
	cpu->di = i286_inc16(cpu, cpu->di);
}

// dec16
unsigned short i286_dec16(CPU* cpu, unsigned short r) {
	r--;
	cpu->flag &= ~(I286_FO | I286_FS | I286_FZ | I286_FA | I286_FP);
	if (r == 0x7f) cpu->flag |= I286_FO;
	if (r & 0x80) cpu->flag |= I286_FS;
	if (!r) cpu->flag |= I286_FZ;
	if ((r & 15) == 15) cpu->flag |= I286_FA;
	if (parity(r)) cpu->flag |= I286_FP;
	return r;
}

// 48: dec ax
void i286_op48(CPU* cpu) {
	cpu->ax = i286_dec16(cpu, cpu->ax);
}

// 49: dec cx
void i286_op49(CPU* cpu) {
	cpu->cx = i286_dec16(cpu, cpu->cx);
}

// 4a: dec dx
void i286_op4A(CPU* cpu) {
	cpu->dx = i286_dec16(cpu, cpu->dx);
}

// 4b: dec bx
void i286_op4B(CPU* cpu) {
	cpu->bx = i286_dec16(cpu, cpu->bx);
}

// 4c: dec sp
void i286_op4C(CPU* cpu) {
	cpu->sp = i286_dec16(cpu, cpu->sp);
}

// 4d: dec bp
void i286_op4D(CPU* cpu) {
	cpu->bp = i286_dec16(cpu, cpu->bp);
}

// 4e: dec si
void i286_op4E(CPU* cpu) {
	cpu->si = i286_dec16(cpu, cpu->si);
}

// 4f: dec di
void i286_op4F(CPU* cpu) {
	cpu->di = i286_dec16(cpu, cpu->di);
}

// 50: push ax
void i286_op50(CPU* cpu) {
	i286_push(cpu, cpu->ax);
}

// 51: push cx
void i286_op51(CPU* cpu) {
	i286_push(cpu, cpu->cx);
}

// 52: push dx
void i286_op52(CPU* cpu) {
	i286_push(cpu, cpu->dx);
}

// 53: push bx
void i286_op53(CPU* cpu) {
	i286_push(cpu, cpu->bx);
}

// 54: push sp
void i286_op54(CPU* cpu) {
	i286_push(cpu, cpu->sp);
}

// 55: push bp
void i286_op55(CPU* cpu) {
	i286_push(cpu, cpu->bp);
}

// 56: push si
void i286_op56(CPU* cpu) {
	i286_push(cpu, cpu->si);
}

// 57: push di
void i286_op57(CPU* cpu) {
	i286_push(cpu, cpu->di);
}

// 58: pop ax
void i286_op58(CPU* cpu) {
	cpu->ax = i286_pop(cpu);
}

// 59: pop cx
void i286_op59(CPU* cpu) {
	cpu->cx = i286_pop(cpu);
}

// 5a: pop dx
void i286_op5A(CPU* cpu) {
	cpu->dx = i286_pop(cpu);
}

// 5b: pop bx
void i286_op5B(CPU* cpu) {
	cpu->bx = i286_pop(cpu);
}

// 5c: pop sp
void i286_op5C(CPU* cpu) {
	cpu->sp = i286_pop(cpu);
}

// 5d: pop bp
void i286_op5D(CPU* cpu) {
	cpu->bp = i286_pop(cpu);
}

// 5e: pop si
void i286_op5E(CPU* cpu) {
	cpu->si = i286_pop(cpu);
}

// 5f: pop di
void i286_op5F(CPU* cpu) {
	cpu->di = i286_pop(cpu);
}

// 60: pusha	(push ax,cx,dx,bx,orig.sp,bp,si,di)
void i286_op60(CPU* cpu) {
	cpu->tmpw = cpu->sp;
	i286_push(cpu, cpu->ax);
	i286_push(cpu, cpu->cx);
	i286_push(cpu, cpu->dx);
	i286_push(cpu, cpu->bx);
	i286_push(cpu, cpu->tmpw);
	i286_push(cpu, cpu->bp);
	i286_push(cpu, cpu->si);
	i286_push(cpu, cpu->di);
}

// 61: popa	(pop di,si,bp,(ignore sp),bx,dx,cx,ax)
void i286_op61(CPU* cpu) {
	cpu->di = i286_pop(cpu);
	cpu->si = i286_pop(cpu);
	cpu->bp = i286_pop(cpu);
	cpu->tmpw = i286_pop(cpu);
	cpu->bx = i286_pop(cpu);
	cpu->dx = i286_pop(cpu);
	cpu->cx = i286_pop(cpu);
	cpu->ax = i286_pop(cpu);
}

// 62,mod: bound rw,md		@eff.addr (md): 2words = min,max. check if min<=rw<=max, INT5 if not
void i286_op62(CPU* cpu) {
	i286_rd_ea(cpu, 1);	// twrd=rw, tmpw=min
	if (cpu->tmpi < 0) {	// interrupts. TODO: fix for protected mode
		i286_interrupt(cpu, 6);		// bad mod
	} else if (cpu->ea.seg == cpu->ss) {
		i286_interrupt(cpu, 12);	// segment SS (protected mode only)
	} else if (cpu->ea.adr >= 0xfffd) {
		i286_interrupt(cpu, 13);	// cross segment address
	} else if ((signed short)cpu->twrd < (signed short)cpu->tmpw) {	// not in bounds: INT5
		i286_interrupt(cpu, 5);
	} else {
		cpu->ltw = i286_mrd(cpu, (cpu->ea.seg << 4) + cpu->ea.adr + 2);
		cpu->htw = i286_mrd(cpu, (cpu->ea.seg << 4) + cpu->ea.adr + 3);
		if ((signed short)cpu->twrd > (signed short)cpu->tmpw) {
			i286_interrupt(cpu, 5);
		}
	}
}

// 63,mod: arpl ew,rw		adjust RPL of EW not less than RPL of RW
void i286_op63(CPU* cpu) {
	i286_interrupt(cpu, 6);	// real mode
	// TODO: protected mode
}

// 64: 386+?
void i286_op64(CPU* cpu) {}

// 65: 386+?
void i286_op65(CPU* cpu) {}

// 66: operand size override prefix
void i286_op66(CPU* cpu) {}

// 67: address size override prefix
void i286_op67(CPU* cpu) {}
void i286_op68(CPU* cpu) {}
void i286_op69(CPU* cpu) {}
void i286_op6A(CPU* cpu) {}
void i286_op6B(CPU* cpu) {}
void i286_op6C(CPU* cpu) {}
void i286_op6D(CPU* cpu) {}
void i286_op6E(CPU* cpu) {}
void i286_op6F(CPU* cpu) {}
void i286_op70(CPU* cpu) {}
void i286_op71(CPU* cpu) {}
void i286_op72(CPU* cpu) {}
void i286_op73(CPU* cpu) {}
void i286_op74(CPU* cpu) {}
void i286_op75(CPU* cpu) {}
void i286_op76(CPU* cpu) {}
void i286_op77(CPU* cpu) {}
void i286_op78(CPU* cpu) {}
void i286_op79(CPU* cpu) {}
void i286_op7A(CPU* cpu) {}
void i286_op7B(CPU* cpu) {}
void i286_op7C(CPU* cpu) {}
void i286_op7D(CPU* cpu) {}
void i286_op7E(CPU* cpu) {}
void i286_op7F(CPU* cpu) {}
void i286_op80(CPU* cpu) {}
void i286_op81(CPU* cpu) {}
void i286_op82(CPU* cpu) {}
void i286_op83(CPU* cpu) {}
void i286_op84(CPU* cpu) {}
void i286_op85(CPU* cpu) {}
void i286_op86(CPU* cpu) {}
void i286_op87(CPU* cpu) {}
void i286_op88(CPU* cpu) {}
void i286_op89(CPU* cpu) {}
void i286_op8A(CPU* cpu) {}
void i286_op8B(CPU* cpu) {}
void i286_op8C(CPU* cpu) {}
void i286_op8D(CPU* cpu) {}
void i286_op8E(CPU* cpu) {}
void i286_op8F(CPU* cpu) {}
void i286_op90(CPU* cpu) {}
void i286_op91(CPU* cpu) {}
void i286_op92(CPU* cpu) {}
void i286_op93(CPU* cpu) {}
void i286_op94(CPU* cpu) {}
void i286_op95(CPU* cpu) {}
void i286_op96(CPU* cpu) {}
void i286_op97(CPU* cpu) {}
void i286_op98(CPU* cpu) {}
void i286_op99(CPU* cpu) {}
void i286_op9A(CPU* cpu) {}
void i286_op9B(CPU* cpu) {}
void i286_op9C(CPU* cpu) {}
void i286_op9D(CPU* cpu) {}
void i286_op9E(CPU* cpu) {}
void i286_op9F(CPU* cpu) {}
void i286_opA0(CPU* cpu) {}
void i286_opA1(CPU* cpu) {}
void i286_opA2(CPU* cpu) {}
void i286_opA3(CPU* cpu) {}
void i286_opA4(CPU* cpu) {}
void i286_opA5(CPU* cpu) {}
void i286_opA6(CPU* cpu) {}
void i286_opA7(CPU* cpu) {}
void i286_opA8(CPU* cpu) {}
void i286_opA9(CPU* cpu) {}
void i286_opAA(CPU* cpu) {}
void i286_opAB(CPU* cpu) {}
void i286_opAC(CPU* cpu) {}
void i286_opAD(CPU* cpu) {}
void i286_opAE(CPU* cpu) {}
void i286_opAF(CPU* cpu) {}
void i286_opB0(CPU* cpu) {}
void i286_opB1(CPU* cpu) {}
void i286_opB2(CPU* cpu) {}
void i286_opB3(CPU* cpu) {}
void i286_opB4(CPU* cpu) {}
void i286_opB5(CPU* cpu) {}
void i286_opB6(CPU* cpu) {}
void i286_opB7(CPU* cpu) {}
void i286_opB8(CPU* cpu) {}
void i286_opB9(CPU* cpu) {}
void i286_opBA(CPU* cpu) {}
void i286_opBB(CPU* cpu) {}
void i286_opBC(CPU* cpu) {}
void i286_opBD(CPU* cpu) {}
void i286_opBE(CPU* cpu) {}
void i286_opBF(CPU* cpu) {}
void i286_opC0(CPU* cpu) {}
void i286_opC1(CPU* cpu) {}
void i286_opC2(CPU* cpu) {}
void i286_opC3(CPU* cpu) {}
void i286_opC4(CPU* cpu) {}
void i286_opC5(CPU* cpu) {}
void i286_opC6(CPU* cpu) {}
void i286_opC7(CPU* cpu) {}
void i286_opC8(CPU* cpu) {}
void i286_opC9(CPU* cpu) {}
void i286_opCA(CPU* cpu) {}
void i286_opCB(CPU* cpu) {}
void i286_opCC(CPU* cpu) {}
void i286_opCD(CPU* cpu) {}
void i286_opCE(CPU* cpu) {}

// iret
void i286_opCF(CPU* cpu) {
	cpu->pc = i286_pop(cpu);
	cpu->cs = i286_pop(cpu);
	cpu->flag = i286_pop(cpu);
}

void i286_opD0(CPU* cpu) {}
void i286_opD1(CPU* cpu) {}
void i286_opD2(CPU* cpu) {}
void i286_opD3(CPU* cpu) {}
void i286_opD4(CPU* cpu) {}
void i286_opD5(CPU* cpu) {}
void i286_opD6(CPU* cpu) {}
void i286_opD7(CPU* cpu) {}
void i286_opD8(CPU* cpu) {}
void i286_opD9(CPU* cpu) {}
void i286_opDA(CPU* cpu) {}
void i286_opDB(CPU* cpu) {}
void i286_opDC(CPU* cpu) {}
void i286_opDD(CPU* cpu) {}
void i286_opDE(CPU* cpu) {}
void i286_opDF(CPU* cpu) {}
void i286_opE0(CPU* cpu) {}
void i286_opE1(CPU* cpu) {}
void i286_opE2(CPU* cpu) {}
void i286_opE3(CPU* cpu) {}
void i286_opE4(CPU* cpu) {}
void i286_opE5(CPU* cpu) {}
void i286_opE6(CPU* cpu) {}
void i286_opE7(CPU* cpu) {}
void i286_opE8(CPU* cpu) {}
void i286_opE9(CPU* cpu) {}
void i286_opEA(CPU* cpu) {}
void i286_opEB(CPU* cpu) {}
void i286_opEC(CPU* cpu) {}
void i286_opED(CPU* cpu) {}
void i286_opEE(CPU* cpu) {}
void i286_opEF(CPU* cpu) {}
void i286_opF0(CPU* cpu) {}
void i286_opF1(CPU* cpu) {}
void i286_opF2(CPU* cpu) {}
void i286_opF3(CPU* cpu) {}
void i286_opF4(CPU* cpu) {}
void i286_opF5(CPU* cpu) {}
void i286_opF6(CPU* cpu) {}
void i286_opF7(CPU* cpu) {}
void i286_opF8(CPU* cpu) {}
void i286_opF9(CPU* cpu) {}
void i286_opFA(CPU* cpu) {}
void i286_opFB(CPU* cpu) {}
void i286_opFC(CPU* cpu) {}
void i286_opFD(CPU* cpu) {}
void i286_opFE(CPU* cpu) {}
void i286_opFF(CPU* cpu) {}

// com.b0 = word operation
// :e - effective address/register
// :r - register (n)

opCode i80286_tab[256] = {
	{0, 1, i286_op00, 0, "add :e,:r"},
	{0, 1, i286_op01, 0, "add :e,:r"},
	{0, 1, i286_op02, 0, "add :r,:e"},
	{0, 1, i286_op03, 0, "add :r,:e"},
	{0, 1, i286_op04, 0, "add al,:1"},
	{0, 1, i286_op05, 0, "add ax,:2"},
	{0, 1, i286_op06, 0, "push es"},
	{0, 1, i286_op07, 0, "pop es"},
	{0, 1, i286_op08, 0, "or :e,:r"},
	{0, 1, i286_op09, 0, "or :e,:r"},
	{0, 1, i286_op0A, 0, "or :r,:e"},
	{0, 1, i286_op0B, 0, "or :r,:e"},
	{0, 1, i286_op0C, 0, "or al,:1"},
	{0, 1, i286_op0D, 0, "or ax,:2"},
	{0, 1, i286_op0E, 0, "push cs"},
	{OF_PREFIX, 1, i286_op0F, 0, "prefix 0F"},
	{0, 1, i286_op10, 0, "adc :e,:r"},
	{0, 1, i286_op11, 0, "adc :e,:r"},
	{0, 1, i286_op12, 0, "adc :r,:e"},
	{0, 1, i286_op13, 0, "adc :r,:e"},
	{0, 1, i286_op14, 0, "adc al,:1"},
	{0, 1, i286_op15, 0, "adc ax,:2"},
	{0, 1, i286_op16, 0, "push ss"},
	{0, 1, i286_op17, 0, "pop ss"},
	{0, 1, i286_op18, 0, "sbb :e,:r"},
	{0, 1, i286_op19, 0, "sbb :e,:r"},
	{0, 1, i286_op1A, 0, "sbb :r,:e"},
	{0, 1, i286_op1B, 0, "sbb :r,:e"},
	{0, 1, i286_op1C, 0, "sbb al,:1"},
	{0, 1, i286_op1D, 0, "sbb ax,:2"},
	{0, 1, i286_op1E, 0, "push ds"},
	{0, 1, i286_op1F, 0, "pop ds"},
	{0, 1, i286_op20, 0, "and :e,:r"},
	{0, 1, i286_op21, 0, "and :e,:r"},
	{0, 1, i286_op22, 0, "and :r,:e"},
	{0, 1, i286_op23, 0, "and :r,:e"},
	{0, 1, i286_op24, 0, "and al,:1"},
	{0, 1, i286_op25, 0, "and ax,:2"},
	{OF_PREFIX, 1, i286_op26, 0, "segment ES"},
	{0, 1, i286_op27, 0, "daa"},
	{0, 1, i286_op28, 0, "sub :e,:r"},
	{0, 1, i286_op29, 0, "sub :e,:r"},
	{0, 1, i286_op2A, 0, "sub :r,:e"},
	{0, 1, i286_op2B, 0, "sub :r,:e"},
	{0, 1, i286_op2C, 0, "sub al,:1"},
	{0, 1, i286_op2D, 0, "sub ax,:2"},
	{OF_PREFIX, 1, i286_op2E, 0, "segment CS"},
	{0, 1, i286_op2F, 0, "das"},
	{0, 1, i286_op30, 0, "xor :e,:r"},
	{0, 1, i286_op31, 0, "xor :e,:r"},
	{0, 1, i286_op32, 0, "xor :r,:e"},
	{0, 1, i286_op33, 0, "xor :r,:e"},
	{0, 1, i286_op34, 0, "xor al,:1"},
	{0, 1, i286_op35, 0, "xor ax,:2"},
	{OF_PREFIX, 1, i286_op36, 0, "segment SS"},
	{0, 1, i286_op37, 0, "aaa"},
	{0, 1, i286_op38, 0, "cmp :e,:r"},
	{0, 1, i286_op39, 0, "cmp :e,:r"},
	{0, 1, i286_op3A, 0, "cmp :r,:e"},
	{0, 1, i286_op3B, 0, "cmp :r,:e"},
	{0, 1, i286_op3C, 0, "cmp al,:1"},
	{0, 1, i286_op3D, 0, "cmp ax,:2"},
	{OF_PREFIX, 1, i286_op3E, 0, "segment DS"},
	{0, 1, i286_op3F, 0, "inc aas"},
	{0, 1, i286_op40, 0, "inc ax"},
	{0, 1, i286_op41, 0, "inc cx"},
	{0, 1, i286_op42, 0, "inc dx"},
	{0, 1, i286_op43, 0, "inc bx"},
	{0, 1, i286_op44, 0, "inc sp"},
	{0, 1, i286_op45, 0, "inc bp"},
	{0, 1, i286_op46, 0, "inc si"},
	{0, 1, i286_op47, 0, "inc di"},
	{0, 1, i286_op48, 0, "dec ax"},
	{0, 1, i286_op49, 0, "dec cx"},
	{0, 1, i286_op4A, 0, "dec dx"},
	{0, 1, i286_op4B, 0, "dec bx"},
	{0, 1, i286_op4C, 0, "dec sp"},
	{0, 1, i286_op4D, 0, "dec bp"},
	{0, 1, i286_op4E, 0, "dec si"},
	{0, 1, i286_op4F, 0, "dec di"},
	{0, 1, i286_op50, 0, "push ax"},
	{0, 1, i286_op51, 0, "push cx"},
	{0, 1, i286_op52, 0, "push dx"},
	{0, 1, i286_op53, 0, "push bx"},
	{0, 1, i286_op54, 0, "push sp"},
	{0, 1, i286_op55, 0, "push bp"},
	{0, 1, i286_op56, 0, "push si"},
	{0, 1, i286_op57, 0, "push di"},
	{0, 1, i286_op58, 0, "pop ax"},
	{0, 1, i286_op59, 0, "pop cx"},
	{0, 1, i286_op5A, 0, "pop dx"},
	{0, 1, i286_op5B, 0, "pop bx"},
	{0, 1, i286_op5C, 0, "pop sp"},
	{0, 1, i286_op5D, 0, "pop bp"},
	{0, 1, i286_op5E, 0, "pop si"},
	{0, 1, i286_op5F, 0, "pop di"},
	{0, 1, i286_op60, 0, "pusha"},
	{0, 1, i286_op61, 0, "popa"},
	{0, 1, i286_op62, 0, ""},
	{0, 1, i286_op63, 0, ""},
	{0, 1, i286_op64, 0, ""},
	{0, 1, i286_op65, 0, ""},
	{0, 1, i286_op66, 0, ""},
	{0, 1, i286_op67, 0, ""},
	{0, 1, i286_op68, 0, ""},
	{0, 1, i286_op69, 0, ""},
	{0, 1, i286_op6A, 0, ""},
	{0, 1, i286_op6B, 0, ""},
	{0, 1, i286_op6C, 0, ""},
	{0, 1, i286_op6D, 0, ""},
	{0, 1, i286_op6E, 0, ""},
	{0, 1, i286_op6F, 0, ""},
	{0, 1, i286_op70, 0, ""},
	{0, 1, i286_op71, 0, ""},
	{0, 1, i286_op72, 0, ""},
	{0, 1, i286_op73, 0, ""},
	{0, 1, i286_op74, 0, ""},
	{0, 1, i286_op75, 0, ""},
	{0, 1, i286_op76, 0, ""},
	{0, 1, i286_op77, 0, ""},
	{0, 1, i286_op78, 0, ""},
	{0, 1, i286_op79, 0, ""},
	{0, 1, i286_op7A, 0, ""},
	{0, 1, i286_op7B, 0, ""},
	{0, 1, i286_op7C, 0, ""},
	{0, 1, i286_op7D, 0, ""},
	{0, 1, i286_op7E, 0, ""},
	{0, 1, i286_op7F, 0, ""},
	{0, 1, i286_op80, 0, ""},
	{0, 1, i286_op81, 0, ""},
	{0, 1, i286_op82, 0, ""},
	{0, 1, i286_op83, 0, ""},
	{0, 1, i286_op84, 0, ""},
	{0, 1, i286_op85, 0, ""},
	{0, 1, i286_op86, 0, ""},
	{0, 1, i286_op87, 0, ""},
	{0, 1, i286_op88, 0, ""},
	{0, 1, i286_op89, 0, ""},
	{0, 1, i286_op8A, 0, ""},
	{0, 1, i286_op8B, 0, ""},
	{0, 1, i286_op8C, 0, ""},
	{0, 1, i286_op8D, 0, ""},
	{0, 1, i286_op8E, 0, ""},
	{0, 1, i286_op8F, 0, ""},
	{0, 1, i286_op90, 0, ""},
	{0, 1, i286_op91, 0, ""},
	{0, 1, i286_op92, 0, ""},
	{0, 1, i286_op93, 0, ""},
	{0, 1, i286_op94, 0, ""},
	{0, 1, i286_op95, 0, ""},
	{0, 1, i286_op96, 0, ""},
	{0, 1, i286_op97, 0, ""},
	{0, 1, i286_op98, 0, ""},
	{0, 1, i286_op99, 0, ""},
	{0, 1, i286_op9A, 0, ""},
	{0, 1, i286_op9B, 0, ""},
	{0, 1, i286_op9C, 0, ""},
	{0, 1, i286_op9D, 0, ""},
	{0, 1, i286_op9E, 0, ""},
	{0, 1, i286_op9F, 0, ""},
	{0, 1, i286_opA0, 0, ""},
	{0, 1, i286_opA1, 0, ""},
	{0, 1, i286_opA2, 0, ""},
	{0, 1, i286_opA3, 0, ""},
	{0, 1, i286_opA4, 0, ""},
	{0, 1, i286_opA5, 0, ""},
	{0, 1, i286_opA6, 0, ""},
	{0, 1, i286_opA7, 0, ""},
	{0, 1, i286_opA8, 0, ""},
	{0, 1, i286_opA9, 0, ""},
	{0, 1, i286_opAA, 0, ""},
	{0, 1, i286_opAB, 0, ""},
	{0, 1, i286_opAC, 0, ""},
	{0, 1, i286_opAD, 0, ""},
	{0, 1, i286_opAE, 0, ""},
	{0, 1, i286_opAF, 0, ""},
	{0, 1, i286_opB0, 0, ""},
	{0, 1, i286_opB1, 0, ""},
	{0, 1, i286_opB2, 0, ""},
	{0, 1, i286_opB3, 0, ""},
	{0, 1, i286_opB4, 0, ""},
	{0, 1, i286_opB5, 0, ""},
	{0, 1, i286_opB6, 0, ""},
	{0, 1, i286_opB7, 0, ""},
	{0, 1, i286_opB8, 0, ""},
	{0, 1, i286_opB9, 0, ""},
	{0, 1, i286_opBA, 0, ""},
	{0, 1, i286_opBB, 0, ""},
	{0, 1, i286_opBC, 0, ""},
	{0, 1, i286_opBD, 0, ""},
	{0, 1, i286_opBE, 0, ""},
	{0, 1, i286_opBF, 0, ""},
	{0, 1, i286_opC0, 0, ""},
	{0, 1, i286_opC1, 0, ""},
	{0, 1, i286_opC2, 0, ""},
	{0, 1, i286_opC3, 0, ""},
	{0, 1, i286_opC4, 0, ""},
	{0, 1, i286_opC5, 0, ""},
	{0, 1, i286_opC6, 0, ""},
	{0, 1, i286_opC7, 0, ""},
	{0, 1, i286_opC8, 0, ""},
	{0, 1, i286_opC9, 0, ""},
	{0, 1, i286_opCA, 0, ""},
	{0, 1, i286_opCB, 0, ""},
	{0, 1, i286_opCC, 0, ""},
	{0, 1, i286_opCD, 0, ""},
	{0, 1, i286_opCE, 0, ""},
	{0, 1, i286_opCF, 0, "iret"},
	{0, 1, i286_opD0, 0, ""},
	{0, 1, i286_opD1, 0, ""},
	{0, 1, i286_opD2, 0, ""},
	{0, 1, i286_opD3, 0, ""},
	{0, 1, i286_opD4, 0, ""},
	{0, 1, i286_opD5, 0, ""},
	{0, 1, i286_opD6, 0, ""},
	{0, 1, i286_opD7, 0, ""},
	{0, 1, i286_opD8, 0, ""},
	{0, 1, i286_opD9, 0, ""},
	{0, 1, i286_opDA, 0, ""},
	{0, 1, i286_opDB, 0, ""},
	{0, 1, i286_opDC, 0, ""},
	{0, 1, i286_opDD, 0, ""},
	{0, 1, i286_opDE, 0, ""},
	{0, 1, i286_opDF, 0, ""},
	{0, 1, i286_opE0, 0, ""},
	{0, 1, i286_opE1, 0, ""},
	{0, 1, i286_opE2, 0, ""},
	{0, 1, i286_opE3, 0, ""},
	{0, 1, i286_opE4, 0, ""},
	{0, 1, i286_opE5, 0, ""},
	{0, 1, i286_opE6, 0, ""},
	{0, 1, i286_opE7, 0, ""},
	{0, 1, i286_opE8, 0, ""},
	{0, 1, i286_opE9, 0, ""},
	{0, 1, i286_opEA, 0, ""},
	{0, 1, i286_opEB, 0, ""},
	{0, 1, i286_opEC, 0, ""},
	{0, 1, i286_opED, 0, ""},
	{0, 1, i286_opEE, 0, ""},
	{0, 1, i286_opEF, 0, ""},
	{0, 1, i286_opF0, 0, ""},
	{0, 1, i286_opF1, 0, ""},
	{0, 1, i286_opF2, 0, ""},
	{0, 1, i286_opF3, 0, ""},
	{0, 1, i286_opF4, 0, ""},
	{0, 1, i286_opF5, 0, ""},
	{0, 1, i286_opF6, 0, ""},
	{0, 1, i286_opF7, 0, ""},
	{0, 1, i286_opF8, 0, ""},
	{0, 1, i286_opF9, 0, ""},
	{0, 1, i286_opFA, 0, ""},
	{0, 1, i286_opFB, 0, ""},
	{0, 1, i286_opFC, 0, ""},
	{0, 1, i286_opFD, 0, ""},
	{0, 1, i286_opFE, 0, ""},
	{0, 1, i286_opFF, 0, ""},
};
