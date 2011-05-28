// NOTE	CHECKED. NO MINES HERE

// ld rp,nn
void npr01(ZXBase* p) {p->cpu->c = p->mem->rd(p->cpu->pc++); p->cpu->b = p->mem->rd(p->cpu->pc++);}
void npr11(ZXBase* p) {p->cpu->e = p->mem->rd(p->cpu->pc++); p->cpu->d = p->mem->rd(p->cpu->pc++);}
void npr21(ZXBase* p) {p->cpu->l = p->mem->rd(p->cpu->pc++); p->cpu->h = p->mem->rd(p->cpu->pc++);}
void npr31(ZXBase* p) {p->cpu->lsp = p->mem->rd(p->cpu->pc++); p->cpu->hsp = p->mem->rd(p->cpu->pc++);}
// inc rp
void npr03(ZXBase* p) {p->cpu->bc++;}
void npr13(ZXBase* p) {p->cpu->de++;}
void npr23(ZXBase* p) {p->cpu->hl++;}
void npr33(ZXBase* p) {p->cpu->sp++;}
// dec rp
void npr0B(ZXBase* p) {p->cpu->bc--;}
void npr1B(ZXBase* p) {p->cpu->de--;}
void npr2B(ZXBase* p) {p->cpu->hl--;}
void npr3B(ZXBase* p) {p->cpu->sp--;}

// ld r,n
void npr06(ZXBase* p) {p->cpu->b = p->mem->rd(p->cpu->pc++);}
void npr0E(ZXBase* p) {p->cpu->c = p->mem->rd(p->cpu->pc++);}
void npr16(ZXBase* p) {p->cpu->d = p->mem->rd(p->cpu->pc++);}
void npr1E(ZXBase* p) {p->cpu->e = p->mem->rd(p->cpu->pc++);}
void npr26(ZXBase* p) {p->cpu->h = p->mem->rd(p->cpu->pc++);}
void npr2E(ZXBase* p) {p->cpu->l = p->mem->rd(p->cpu->pc++);}
void npr36(ZXBase* p) {p->mem->wr(p->cpu->hl,p->mem->rd(p->cpu->pc++));}
void npr3E(ZXBase* p) {p->cpu->a = p->mem->rd(p->cpu->pc++);}
// inc r
void npr04(ZXBase* p) {p->cpu->b++; p->cpu->f = flag[p->cpu->b].inc | (p->cpu->f & FC);}
void npr0C(ZXBase* p) {p->cpu->c++; p->cpu->f = flag[p->cpu->c].inc | (p->cpu->f & FC);}
void npr14(ZXBase* p) {p->cpu->d++; p->cpu->f = flag[p->cpu->d].inc | (p->cpu->f & FC);}
void npr1C(ZXBase* p) {p->cpu->e++; p->cpu->f = flag[p->cpu->e].inc | (p->cpu->f & FC);}
void npr24(ZXBase* p) {p->cpu->h++; p->cpu->f = flag[p->cpu->h].inc | (p->cpu->f & FC);}
void npr2C(ZXBase* p) {p->cpu->l++; p->cpu->f = flag[p->cpu->l].inc | (p->cpu->f & FC);}
void npr34(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->hl) + 1; p->mem->wr(p->cpu->hl,p->cpu->x); p->cpu->f = flag[p->cpu->x].inc | (p->cpu->f & FC);}
void npr3C(ZXBase* p) {p->cpu->a++; p->cpu->f = flag[p->cpu->a].inc | (p->cpu->f & FC);}
// dec r
void npr05(ZXBase* p) {p->cpu->b--; p->cpu->f = flag[p->cpu->b].dec | (p->cpu->f & FC);}
void npr0D(ZXBase* p) {p->cpu->c--; p->cpu->f = flag[p->cpu->c].dec | (p->cpu->f & FC);}
void npr15(ZXBase* p) {p->cpu->d--; p->cpu->f = flag[p->cpu->d].dec | (p->cpu->f & FC);}
void npr1D(ZXBase* p) {p->cpu->e--; p->cpu->f = flag[p->cpu->e].dec | (p->cpu->f & FC);}
void npr25(ZXBase* p) {p->cpu->h--; p->cpu->f = flag[p->cpu->h].dec | (p->cpu->f & FC);}
void npr2D(ZXBase* p) {p->cpu->l--; p->cpu->f = flag[p->cpu->l].dec | (p->cpu->f & FC);}
void npr35(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->hl) - 1; p->mem->wr(p->cpu->hl,p->cpu->x); p->cpu->f = flag[p->cpu->x].dec | (p->cpu->f & FC);}
void npr3D(ZXBase* p) {p->cpu->a--; p->cpu->f = flag[p->cpu->a].dec | (p->cpu->f & FC);}

// rlca,rrca,rla,rra
void npr07(ZXBase* p) {p->cpu->a = flag[p->cpu->a].rlc.r; p->cpu->f = (p->cpu->f & (FS | FZ | FP)) | (p->cpu->a & (F5 | F3 | FC));}
void npr0F(ZXBase* p) {p->cpu->a = flag[p->cpu->a].rrc.r; p->cpu->f = (p->cpu->f & (FS | FZ | FP)) | (p->cpu->a & (F5 | F3)) | ((p->cpu->a & 128)?FC:0);}
void npr17(ZXBase* p) {bool z = (p->cpu->a & 128); p->cpu->a = flag[p->cpu->a].rl[p->cpu->f & FC].r; p->cpu->f = (p->cpu->f & (FS | FZ | FP)) | (p->cpu->a & (F5 | F3)) | (z?FC:0);}
void npr1F(ZXBase* p) {bool z = (p->cpu->a & 1); p->cpu->a = flag[p->cpu->a].rr[p->cpu->f & FC].r; p->cpu->f = (p->cpu->f & (FS | FZ | FP)) | (p->cpu->a & (F5 | F3)) | (z?FC:0);}

// jr	mptr = addr when condition is true
void npr18(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->pc++); p->cpu->pc += (signed char)p->cpu->x; p->cpu->mptr = p->cpu->pc;}
void npr20(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->pc++); if (~p->cpu->f & FZ) {p->cpu->pc += (signed char)p->cpu->x; /*p->cpu->t += 5;*/ p->cpu->mptr = p->cpu->pc;}}
void npr28(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->pc++); if (p->cpu->f & FZ) {p->cpu->pc += (signed char)p->cpu->x; /*p->cpu->t += 5;*/ p->cpu->mptr = p->cpu->pc;}}
void npr30(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->pc++); if (~p->cpu->f & FC) {p->cpu->pc += (signed char)p->cpu->x; /*p->cpu->t += 5;*/ p->cpu->mptr = p->cpu->pc;}}
void npr38(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->pc++); if (p->cpu->f & FC) {p->cpu->pc += (signed char)p->cpu->x; /*p->cpu->t += 5;*/ p->cpu->mptr = p->cpu->pc;}}

// ld (bc|de),a; ld a,(bc|de)
void npr02(ZXBase* p) {p->mem->wr(p->cpu->bc,p->cpu->a); p->cpu->lptr = p->cpu->c + 1; p->cpu->hptr = p->cpu->a;}		// mptr = (a << 8) + ((bc + 1) & 0xff)
void npr12(ZXBase* p) {p->mem->wr(p->cpu->de,p->cpu->a); p->cpu->lptr = p->cpu->e + 1; p->cpu->hptr = p->cpu->a;}		// mptr = (a << 8) + ((de + 1) & 0xff)
void npr0A(ZXBase* p) {p->cpu->a = p->mem->rd(p->cpu->bc); p->cpu->mptr = p->cpu->bc + 1;}			// mptr = bc + 1
void npr1A(ZXBase* p) {p->cpu->a = p->mem->rd(p->cpu->de); p->cpu->mptr = p->cpu->de + 1;}			// mptr = de + 1
// ld (nn),[hl|a]; ld [hl|a],(nn)
void npr22(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr++,p->cpu->l); p->mem->wr(p->cpu->mptr,p->cpu->h);}	// mptr = nn + 1
void npr2A(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->cpu->l = p->mem->rd(p->cpu->mptr++); p->cpu->h = p->mem->rd(p->cpu->mptr);}		// mptr = nn + 1
void npr32(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->mem->wr(p->cpu->mptr++,p->cpu->a); p->cpu->hptr = p->cpu->a;}				// mptr = (A << 8) + ((adr + 1) & 0xff)
void npr3A(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); p->cpu->a = p->mem->rd(p->cpu->mptr++);}						// mptr = nn + 1
// add hl,rp	mptr = hl + 1 (before adding)
void addHL(ZXBase* p,int op) {
	int z = p->cpu->hl;
	p->cpu->mptr = p->cpu->hl + 1;
	p->cpu->hl += op;
	p->cpu->f = (p->cpu->f & (FS | FZ | FP)) | (p->cpu->h & (F5 | F3)) | ((((z & 0xfff) + (op & 0xfff)) > 0xfff)?FH:0) | (((z + op) > 0xffff)?FC:0);
}
void npr09(ZXBase* p) {addHL(p,p->cpu->bc);}
void npr19(ZXBase* p) {addHL(p,p->cpu->de);}
void npr29(ZXBase* p) {addHL(p,p->cpu->hl);}
void npr39(ZXBase* p) {addHL(p,p->cpu->sp);}

// others (nop,exa,djnz,daa,cpl,scf,ccf
void npr00(ZXBase*) {}			// nop
void npr08(ZXBase* p) {int tmp = p->cpu->af; p->cpu->af = p->cpu->alt.af; p->cpu->alt.af = tmp;}	// ex af,af'
void npr10(ZXBase* p) {p->cpu->dlt = p->mem->rd(p->cpu->pc++); p->cpu->b--; if (p->cpu->b != 0) {p->cpu->pc += (signed char)p->cpu->dlt; /*p->cpu->t += 5;*/ p->cpu->mptr = p->cpu->pc;}}	// mptr = addr when b!=0
void npr27(ZXBase* p) {
	switch (p->cpu->f & (FH | FN | FC)) {
		case 0: if ((p->cpu->a & 15) < 10) {
				if (p->cpu->a > 0x9f) {p->cpu->a += 0x60; p->cpu->f |= FC;}
			} else {
				if (p->cpu->a < 0x90) {p->cpu->a += 6; p->cpu->f &= ~FC;} else {p->cpu->a += 0x66; p->cpu->f |= FC;}
			} break;
		case FH: if (p->cpu->a < 0xa0) {p->cpu->a += 6; p->cpu->f &= ~FC;} else {p->cpu->a += 0x66; p->cpu->f |= FC;} break;
		case FC: p->cpu->a += (((p->cpu->a & 15) > 9) ? 0x66 : 0x60); p->cpu->f |= FC; break;
		case (FH | FC): p->cpu->a += 0x66; p->cpu->f |= FC; break;
//		case FN: break;
		case (FH | FN): p->cpu->a -= 0x06; p->cpu->f &= ~FC; break;
		case (FN | FC): p->cpu->a -= 0x60; p->cpu->f |= FC; break;
		case (FH | FN | FC): p->cpu->a -= 0x66; p->cpu->f |= FC; break;
	}
	p->cpu->f = (p->cpu->f & (FN | FC)) | (p->cpu->a & (FS | F5 | F3)) | ((p->cpu->a == 0)?FZ:0) | (parity(p->cpu->a)?FP:0);
}
void npr2F(ZXBase* p) {p->cpu->a ^= 255; p->cpu->f = (p->cpu->f & ~(F5 | F3)) | (p->cpu->a & (F5 | F3)) | FH | FN;}
void npr37(ZXBase* p) {p->cpu->f = (p->cpu->f & ~(FH | FN)) | (p->cpu->a & (F5 | F3)) | FC;}
void npr3F(ZXBase* p) {p->cpu->f = ((p->cpu->f & ~(FH | FN)) | ((p->cpu->f & FC)?FH:0) | (p->cpu->a & (F5 | F3))) ^ FC;}

// ld b,r
void npr40(ZXBase*) {}
void npr41(ZXBase* p) {p->cpu->b = p->cpu->c;}
void npr42(ZXBase* p) {p->cpu->b = p->cpu->d;}
void npr43(ZXBase* p) {p->cpu->b = p->cpu->e;}
void npr44(ZXBase* p) {p->cpu->b = p->cpu->h;}
void npr45(ZXBase* p) {p->cpu->b = p->cpu->l;}
void npr46(ZXBase* p) {p->cpu->b = p->mem->rd(p->cpu->hl);}
void npr47(ZXBase* p) {p->cpu->b = p->cpu->a;}
// ld c,r
void npr48(ZXBase* p) {p->cpu->c = p->cpu->b;}
void npr49(ZXBase*) {}
void npr4A(ZXBase* p) {p->cpu->c = p->cpu->d;}
void npr4B(ZXBase* p) {p->cpu->c = p->cpu->e;}
void npr4C(ZXBase* p) {p->cpu->c = p->cpu->h;}
void npr4D(ZXBase* p) {p->cpu->c = p->cpu->l;}
void npr4E(ZXBase* p) {p->cpu->c = p->mem->rd(p->cpu->hl);}
void npr4F(ZXBase* p) {p->cpu->c = p->cpu->a;}
// ld d,r
void npr50(ZXBase* p) {p->cpu->d = p->cpu->b;}
void npr51(ZXBase* p) {p->cpu->d = p->cpu->c;}
void npr52(ZXBase*) {}
void npr53(ZXBase* p) {p->cpu->d = p->cpu->e;}
void npr54(ZXBase* p) {p->cpu->d = p->cpu->h;}
void npr55(ZXBase* p) {p->cpu->d = p->cpu->l;}
void npr56(ZXBase* p) {p->cpu->d = p->mem->rd(p->cpu->hl);}
void npr57(ZXBase* p) {p->cpu->d = p->cpu->a;}
// ld e,r
void npr58(ZXBase* p) {p->cpu->e = p->cpu->b;}
void npr59(ZXBase* p) {p->cpu->e = p->cpu->c;}
void npr5A(ZXBase* p) {p->cpu->e = p->cpu->d;}
void npr5B(ZXBase*) {}
void npr5C(ZXBase* p) {p->cpu->e = p->cpu->h;}
void npr5D(ZXBase* p) {p->cpu->e = p->cpu->l;}
void npr5E(ZXBase* p) {p->cpu->e = p->mem->rd(p->cpu->hl);}
void npr5F(ZXBase* p) {p->cpu->e = p->cpu->a;}
// ld h,r
void npr60(ZXBase* p) {p->cpu->h = p->cpu->b;}
void npr61(ZXBase* p) {p->cpu->h = p->cpu->c;}
void npr62(ZXBase* p) {p->cpu->h = p->cpu->d;}
void npr63(ZXBase* p) {p->cpu->h = p->cpu->e;}
void npr64(ZXBase*) {}
void npr65(ZXBase* p) {p->cpu->h = p->cpu->l;}
void npr66(ZXBase* p) {p->cpu->h = p->mem->rd(p->cpu->hl);}
void npr67(ZXBase* p) {p->cpu->h = p->cpu->a;}
// ld l,r
void npr68(ZXBase* p) {p->cpu->l = p->cpu->b;}
void npr69(ZXBase* p) {p->cpu->l = p->cpu->c;}
void npr6A(ZXBase* p) {p->cpu->l = p->cpu->d;}
void npr6B(ZXBase* p) {p->cpu->l = p->cpu->e;}
void npr6C(ZXBase* p) {p->cpu->l = p->cpu->h;}
void npr6D(ZXBase*) {}
void npr6E(ZXBase* p) {p->cpu->l = p->mem->rd(p->cpu->hl);}
void npr6F(ZXBase* p) {p->cpu->l = p->cpu->a;}
// ld (hl),r
void npr70(ZXBase* p) {p->mem->wr(p->cpu->hl,p->cpu->b);}
void npr71(ZXBase* p) {p->mem->wr(p->cpu->hl,p->cpu->c);}
void npr72(ZXBase* p) {p->mem->wr(p->cpu->hl,p->cpu->d);}
void npr73(ZXBase* p) {p->mem->wr(p->cpu->hl,p->cpu->e);}
void npr74(ZXBase* p) {p->mem->wr(p->cpu->hl,p->cpu->h);}
void npr75(ZXBase* p) {p->mem->wr(p->cpu->hl,p->cpu->l);}
void npr76(ZXBase* p) {if (!p->istrb) p->cpu->pc--;}
void npr77(ZXBase* p) {p->mem->wr(p->cpu->hl,p->cpu->a);}
// ld a,r
void npr78(ZXBase* p) {p->cpu->a = p->cpu->b;}
void npr79(ZXBase* p) {p->cpu->a = p->cpu->c;}
void npr7A(ZXBase* p) {p->cpu->a = p->cpu->d;}
void npr7B(ZXBase* p) {p->cpu->a = p->cpu->e;}
void npr7C(ZXBase* p) {p->cpu->a = p->cpu->h;}
void npr7D(ZXBase* p) {p->cpu->a = p->cpu->l;}
void npr7E(ZXBase* p) {p->cpu->a = p->mem->rd(p->cpu->hl);}
void npr7F(ZXBase*) {}

// add a,r
void npr80(ZXBase* p) {p->cpu->f = flag[p->cpu->a].add[p->cpu->b][0]; p->cpu->a += p->cpu->b;}
void npr81(ZXBase* p) {p->cpu->f = flag[p->cpu->a].add[p->cpu->c][0]; p->cpu->a += p->cpu->c;}
void npr82(ZXBase* p) {p->cpu->f = flag[p->cpu->a].add[p->cpu->d][0]; p->cpu->a += p->cpu->d;}
void npr83(ZXBase* p) {p->cpu->f = flag[p->cpu->a].add[p->cpu->e][0]; p->cpu->a += p->cpu->e;}
void npr84(ZXBase* p) {p->cpu->f = flag[p->cpu->a].add[p->cpu->h][0]; p->cpu->a += p->cpu->h;}
void npr85(ZXBase* p) {p->cpu->f = flag[p->cpu->a].add[p->cpu->l][0]; p->cpu->a += p->cpu->l;}
void npr86(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->hl); p->cpu->f = flag[p->cpu->a].add[p->cpu->x][0]; p->cpu->a += p->cpu->x;}
void npr87(ZXBase* p) {p->cpu->f = flag[p->cpu->a].add[p->cpu->a][0]; p->cpu->a += p->cpu->a;}
// adc a,r
void adcXX(ZXBase* p, unsigned char op) {
	int z = (p->cpu->f & FC)?1:0;
	p->cpu->f = flag[p->cpu->a].add[op][z];
	p->cpu->a += (op + z);
}
void npr88(ZXBase* p) {adcXX(p,p->cpu->b);}
void npr89(ZXBase* p) {adcXX(p,p->cpu->c);}
void npr8A(ZXBase* p) {adcXX(p,p->cpu->d);}
void npr8B(ZXBase* p) {adcXX(p,p->cpu->e);}
void npr8C(ZXBase* p) {adcXX(p,p->cpu->h);}
void npr8D(ZXBase* p) {adcXX(p,p->cpu->l);}
void npr8E(ZXBase* p) {adcXX(p,p->mem->rd(p->cpu->hl));}
void npr8F(ZXBase* p) {adcXX(p,p->cpu->a);}
// sub r
void npr90(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->b][0]; p->cpu->a -= p->cpu->b;}
void npr91(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->c][0]; p->cpu->a -= p->cpu->c;}
void npr92(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->d][0]; p->cpu->a -= p->cpu->d;}
void npr93(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->e][0]; p->cpu->a -= p->cpu->e;}
void npr94(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->h][0]; p->cpu->a -= p->cpu->h;}
void npr95(ZXBase* p) {p->cpu->f = flag[p->cpu->a].sub[p->cpu->l][0]; p->cpu->a -= p->cpu->l;}
void npr96(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->hl); p->cpu->f = flag[p->cpu->a].sub[p->cpu->x][0]; p->cpu->a -= p->cpu->x;}
void npr97(ZXBase* p) {p->cpu->f = FZ | FN; p->cpu->a = 0;}
// sbc a,r
void sbcXX(ZXBase* p,unsigned char op) {
	int z = (p->cpu->f & FC)?1:0;
	p->cpu->f = flag[p->cpu->a].sub[op][z];
	p->cpu->a -= (op + z);
}
void npr98(ZXBase* p) {sbcXX(p,p->cpu->b);}
void npr99(ZXBase* p) {sbcXX(p,p->cpu->c);}
void npr9A(ZXBase* p) {sbcXX(p,p->cpu->d);}
void npr9B(ZXBase* p) {sbcXX(p,p->cpu->e);}
void npr9C(ZXBase* p) {sbcXX(p,p->cpu->h);}
void npr9D(ZXBase* p) {sbcXX(p,p->cpu->l);}
void npr9E(ZXBase* p) {sbcXX(p,p->mem->rd(p->cpu->hl));}
void npr9F(ZXBase* p) {sbcXX(p,p->cpu->a);}

// and r
void nprA0(ZXBase* p) {p->cpu->a &= p->cpu->b; p->cpu->f = flag[p->cpu->a].andf;}
void nprA1(ZXBase* p) {p->cpu->a &= p->cpu->c; p->cpu->f = flag[p->cpu->a].andf;}
void nprA2(ZXBase* p) {p->cpu->a &= p->cpu->d; p->cpu->f = flag[p->cpu->a].andf;}
void nprA3(ZXBase* p) {p->cpu->a &= p->cpu->e; p->cpu->f = flag[p->cpu->a].andf;}
void nprA4(ZXBase* p) {p->cpu->a &= p->cpu->h; p->cpu->f = flag[p->cpu->a].andf;}
void nprA5(ZXBase* p) {p->cpu->a &= p->cpu->l; p->cpu->f = flag[p->cpu->a].andf;}
void nprA6(ZXBase* p) {p->cpu->a &= p->mem->rd(p->cpu->hl); p->cpu->f = flag[p->cpu->a].andf;}
void nprA7(ZXBase* p) {p->cpu->f = flag[p->cpu->a].andf;}
// xor r
void nprA8(ZXBase* p) {p->cpu->a ^= p->cpu->b; p->cpu->f = flag[p->cpu->a].orf;}
void nprA9(ZXBase* p) {p->cpu->a ^= p->cpu->c; p->cpu->f = flag[p->cpu->a].orf;}
void nprAA(ZXBase* p) {p->cpu->a ^= p->cpu->d; p->cpu->f = flag[p->cpu->a].orf;}
void nprAB(ZXBase* p) {p->cpu->a ^= p->cpu->e; p->cpu->f = flag[p->cpu->a].orf;}
void nprAC(ZXBase* p) {p->cpu->a ^= p->cpu->h; p->cpu->f = flag[p->cpu->a].orf;}
void nprAD(ZXBase* p) {p->cpu->a ^= p->cpu->l; p->cpu->f = flag[p->cpu->a].orf;}
void nprAE(ZXBase* p) {p->cpu->a ^= p->mem->rd(p->cpu->hl); p->cpu->f = flag[p->cpu->a].orf;}
void nprAF(ZXBase* p) {p->cpu->a = 0; p->cpu->f = FZ | FP;}
// or r
void nprB0(ZXBase* p) {p->cpu->a |= p->cpu->b; p->cpu->f = flag[p->cpu->a].orf;}
void nprB1(ZXBase* p) {p->cpu->a |= p->cpu->c; p->cpu->f = flag[p->cpu->a].orf;}
void nprB2(ZXBase* p) {p->cpu->a |= p->cpu->d; p->cpu->f = flag[p->cpu->a].orf;}
void nprB3(ZXBase* p) {p->cpu->a |= p->cpu->e; p->cpu->f = flag[p->cpu->a].orf;}
void nprB4(ZXBase* p) {p->cpu->a |= p->cpu->h; p->cpu->f = flag[p->cpu->a].orf;}
void nprB5(ZXBase* p) {p->cpu->a |= p->cpu->l; p->cpu->f = flag[p->cpu->a].orf;}
void nprB6(ZXBase* p) {p->cpu->a |= p->mem->rd(p->cpu->hl); p->cpu->f = flag[p->cpu->a].orf;}
void nprB7(ZXBase* p) {p->cpu->f = flag[p->cpu->a].orf;}
// cp r
void nprB8(ZXBase* p) {p->cpu->f = flag[p->cpu->a].cp[p->cpu->b];}
void nprB9(ZXBase* p) {p->cpu->f = flag[p->cpu->a].cp[p->cpu->c];}
void nprBA(ZXBase* p) {p->cpu->f = flag[p->cpu->a].cp[p->cpu->d];}
void nprBB(ZXBase* p) {p->cpu->f = flag[p->cpu->a].cp[p->cpu->e];}
void nprBC(ZXBase* p) {p->cpu->f = flag[p->cpu->a].cp[p->cpu->h];}
void nprBD(ZXBase* p) {p->cpu->f = flag[p->cpu->a].cp[p->cpu->l];}
void nprBE(ZXBase* p) {p->cpu->f = flag[p->cpu->a].cp[p->mem->rd(p->cpu->hl)];}
void nprBF(ZXBase* p) {p->cpu->f = flag[p->cpu->a].cp[p->cpu->a];}		// flag[p->cpu->a].cp[p->cpu->a]

// ret cc	mptr = ret.addr when condition is true
void nprC9(ZXBase* p) {p->cpu->lpc = p->mem->rd(p->cpu->sp++); p->cpu->hpc = p->mem->rd(p->cpu->sp++); p->cpu->mptr = p->cpu->pc;}
void nprC0(ZXBase* p) {if (!(p->cpu->f & FZ)) {nprC9(p); /*p->cpu->t += 6;*/}}
void nprC8(ZXBase* p) {if (p->cpu->f & FZ) {nprC9(p); /*p->cpu->t += 6;*/}}
void nprD0(ZXBase* p) {if (!(p->cpu->f & FC)) {nprC9(p); /*p->cpu->t += 6;*/}}
void nprD8(ZXBase* p) {if (p->cpu->f & FC) {nprC9(p); /*p->cpu->t += 6;*/}}
void nprE0(ZXBase* p) {if (!(p->cpu->f & FP)) {nprC9(p); /*p->cpu->t += 6;*/}}
void nprE8(ZXBase* p) {if (p->cpu->f & FP) {nprC9(p); /*p->cpu->t += 6;*/}}
void nprF0(ZXBase* p) {if (!(p->cpu->f & FS)) {nprC9(p); /*p->cpu->t += 6;*/}}
void nprF8(ZXBase* p) {if (p->cpu->f & FS) {nprC9(p); /*p->cpu->t += 6;*/}}
// jp cc
void jumpif(ZXBase* p, bool cnd) {p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++); if (cnd) p->cpu->pc = p->cpu->mptr;}
void nprC3(ZXBase* p) {jumpif(p, true);}
void nprC2(ZXBase* p) {jumpif (p, !(p->cpu->f & FZ));}
void nprCA(ZXBase* p) {jumpif(p, p->cpu->f & FZ);}
void nprD2(ZXBase* p) {jumpif(p, !(p->cpu->f & FC));}
void nprDA(ZXBase* p) {jumpif(p, p->cpu->f & FC);}
void nprE2(ZXBase* p) {jumpif(p, !(p->cpu->f & FP));}
void nprEA(ZXBase* p) {jumpif(p, p->cpu->f & FP);}
void nprF2(ZXBase* p) {jumpif(p, !(p->cpu->f & FS));}
void nprFA(ZXBase* p) {jumpif(p, p->cpu->f & FS);}
// call cc
void callif(ZXBase* p, bool cnd) {
	p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->hptr = p->mem->rd(p->cpu->pc++);
	if (cnd) {/*p->cpu->t += 7;*/ p->mem->wr(--p->cpu->sp,p->cpu->hpc); p->mem->wr(--p->cpu->sp,p->cpu->lpc); p->cpu->pc = p->cpu->mptr;}
}
void nprCD(ZXBase* p) {callif(p, true);}
void nprC4(ZXBase* p) {callif(p, !(p->cpu->f & FZ));}
void nprCC(ZXBase* p) {callif(p, p->cpu->f & FZ);}
void nprD4(ZXBase* p) {callif(p, !(p->cpu->f & FC));}
void nprDC(ZXBase* p) {callif(p, p->cpu->f & FC);}
void nprE4(ZXBase* p) {callif(p, !(p->cpu->f & FP));}
void nprEC(ZXBase* p) {callif(p, p->cpu->f & FP);}
void nprF4(ZXBase* p) {callif(p, !(p->cpu->f & FS));}
void nprFC(ZXBase* p) {callif(p, p->cpu->f & FS);}
// pop
void nprC1(ZXBase* p) {p->cpu->c = p->mem->rd(p->cpu->sp++); p->cpu->b = p->mem->rd(p->cpu->sp++);}
void nprD1(ZXBase* p) {p->cpu->e = p->mem->rd(p->cpu->sp++); p->cpu->d = p->mem->rd(p->cpu->sp++);}
void nprE1(ZXBase* p) {p->cpu->l = p->mem->rd(p->cpu->sp++); p->cpu->h = p->mem->rd(p->cpu->sp++);}
void nprF1(ZXBase* p) {p->cpu->f = p->mem->rd(p->cpu->sp++); p->cpu->a = p->mem->rd(p->cpu->sp++);}
// push
void nprC5(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->b); p->mem->wr(--p->cpu->sp,p->cpu->c);}
void nprD5(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->d); p->mem->wr(--p->cpu->sp,p->cpu->e);}
void nprE5(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->h); p->mem->wr(--p->cpu->sp,p->cpu->l);}
void nprF5(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->a); p->mem->wr(--p->cpu->sp,p->cpu->f);}
// ariphmetic / logic
void nprC6(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->pc++); p->cpu->f = flag[p->cpu->a].add[p->cpu->x][0]; p->cpu->a += p->cpu->x;}
void nprCE(ZXBase* p) {adcXX(p,p->mem->rd(p->cpu->pc++));}
void nprD6(ZXBase* p) {p->cpu->x = p->mem->rd(p->cpu->pc++); p->cpu->f = flag[p->cpu->a].sub[p->cpu->x][0]; p->cpu->a -= p->cpu->x;}
void nprDE(ZXBase* p) {sbcXX(p,p->mem->rd(p->cpu->pc++));}
void nprE6(ZXBase* p) {p->cpu->a &= p->mem->rd(p->cpu->pc++); p->cpu->f = flag[p->cpu->a].andf;}
void nprEE(ZXBase* p) {p->cpu->a ^= p->mem->rd(p->cpu->pc++); p->cpu->f = flag[p->cpu->a].orf;}
void nprF6(ZXBase* p) {p->cpu->a |= p->mem->rd(p->cpu->pc++); p->cpu->f = flag[p->cpu->a].orf;}
void nprFE(ZXBase* p) {p->cpu->f = flag[p->cpu->a].cp[p->mem->rd(p->cpu->pc++)];}
// rst n	mptr = adr
void nprC7(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->hpc); p->mem->wr(--p->cpu->sp,p->cpu->lpc); p->cpu->pc = 0x00; p->cpu->mptr = p->cpu->pc;}
void nprCF(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->hpc); p->mem->wr(--p->cpu->sp,p->cpu->lpc); p->cpu->pc = 0x08; p->cpu->mptr = p->cpu->pc;}
void nprD7(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->hpc); p->mem->wr(--p->cpu->sp,p->cpu->lpc); p->cpu->pc = 0x10; p->cpu->mptr = p->cpu->pc;}
void nprDF(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->hpc); p->mem->wr(--p->cpu->sp,p->cpu->lpc); p->cpu->pc = 0x18; p->cpu->mptr = p->cpu->pc;}
void nprE7(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->hpc); p->mem->wr(--p->cpu->sp,p->cpu->lpc); p->cpu->pc = 0x20; p->cpu->mptr = p->cpu->pc;}
void nprEF(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->hpc); p->mem->wr(--p->cpu->sp,p->cpu->lpc); p->cpu->pc = 0x28; p->cpu->mptr = p->cpu->pc;}
void nprF7(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->hpc); p->mem->wr(--p->cpu->sp,p->cpu->lpc); p->cpu->pc = 0x30; p->cpu->mptr = p->cpu->pc;}
void nprFF(ZXBase* p) {p->mem->wr(--p->cpu->sp,p->cpu->hpc); p->mem->wr(--p->cpu->sp,p->cpu->lpc); p->cpu->pc = 0x38; p->cpu->mptr = p->cpu->pc;}
// di,ei
void nprF3(ZXBase* p) {p->cpu->iff1 = p->cpu->iff2 = false;}
void nprFB(ZXBase* p) {p->cpu->nextei = true;}
// exchange
void nprD9(ZXBase* p) {
	p->cpu->adr = p->cpu->bc; p->cpu->bc = p->cpu->alt.bc; p->cpu->alt.bc = p->cpu->adr;
	p->cpu->adr = p->cpu->de; p->cpu->de = p->cpu->alt.de; p->cpu->alt.de = p->cpu->adr;
	p->cpu->adr = p->cpu->hl; p->cpu->hl = p->cpu->alt.hl; p->cpu->alt.hl = p->cpu->adr;}
void nprEB(ZXBase* p) {p->cpu->adr = p->cpu->de; p->cpu->de = p->cpu->hl; p->cpu->hl = p->cpu->adr;}
// other
void nprD3(ZXBase* p) {p->cpu->hptr = p->cpu->a; p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->io->out(p->cpu->mptr,p->cpu->a); p->cpu->lptr++;}	// mptr = (a << 8) + ((n + 1) & FF)
void nprDB(ZXBase* p) {p->cpu->hptr = p->cpu->a; p->cpu->lptr = p->mem->rd(p->cpu->pc++); p->cpu->a = p->io->in(p->cpu->mptr); p->cpu->mptr++;}	// mptr = (a << 8) + n + 1
void nprE3(ZXBase* p) {p->cpu->lptr = p->mem->rd(p->cpu->sp++); p->cpu->hptr = p->mem->rd(p->cpu->sp); p->mem->wr(p->cpu->sp--,p->cpu->h); p->mem->wr(p->cpu->sp,p->cpu->l); p->cpu->hl = p->cpu->mptr;}	// ex (sp),rp	mptr = rp after operation
void nprE9(ZXBase* p) {p->cpu->pc = p->cpu->hl;}
void nprF9(ZXBase* p) {p->cpu->sp = p->cpu->hl;}
// prefix
void nprDD(ZXBase* p) {p->cpu->mod = 1;}		// DD: ix
void nprFD(ZXBase* p) {p->cpu->mod = 2;}		// FD: iy
void nprED(ZXBase* p) {p->cpu->mod = 8;}		// ED
void nprCB(ZXBase* p) {				// CB (DDCB,FDCB)
	p->cpu->mod |= 4;
	if (p->cpu->mod != 4) {
		p->cpu->dlt = p->mem->rd(p->cpu->pc++);
		p->cpu->r = ((p->cpu->r - 1) & 127) | (p->cpu->r & 128);
	}
}

//==================

ZOp nopref[256]={
	{4,	CND_NONE,0,0,	0,	&npr00,	"nop"},
	{10,	CND_NONE,0,0,	0,	&npr01,	"ld bc,:2"},
	{7,	CND_NONE,0,0,	0,	&npr02,	"ld (bc),a"},
	{6,	CND_NONE,0,0,	0,	&npr03,	"inc bc"},
	{4,	CND_NONE,0,0,	0,	&npr04,	"inc b"},
	{4,	CND_NONE,0,0,	0,	&npr05,	"dec b"},
	{7,	CND_NONE,0,0,	0,	&npr06,	"ld b,:1"},
	{4,	CND_NONE,0,0,	0,	&npr07,	"rlca"},

	{4,	CND_NONE,0,0,	0,	&npr08,	"ex af,af'"},
	{11,	CND_NONE,0,0,	0,	&npr09,	"add hl,bc"},
	{7,	CND_NONE,0,0,	0,	&npr0A,	"ld a,(bc)"},
	{6,	CND_NONE,0,0,	0,	&npr0B,	"dec bc"},
	{4,	CND_NONE,0,0,	0,	&npr0C,	"inc c"},
	{4,	CND_NONE,0,0,	0,	&npr0D,	"dec c"},
	{7,	CND_NONE,0,0,	0,	&npr0E,	"ld c,:1"},
	{4,	CND_NONE,0,0,	0,	&npr0F,	"rrca"},

	{8,	CND_DJNZ,5,0,	0,	&npr10,	"djnz :3"},
	{10,	CND_NONE,0,0,	0,	&npr11,	"ld de,:2"},
	{7,	CND_NONE,0,0,	0,	&npr12,	"ld (de),a"},
	{6,	CND_NONE,0,0,	0,	&npr13,	"inc de"},
	{4,	CND_NONE,0,0,	0,	&npr14,	"inc d"},
	{4,	CND_NONE,0,0,	0,	&npr15,	"dec d"},
	{7,	CND_NONE,0,0,	0,	&npr16,	"ld d,:1"},
	{4,	CND_NONE,0,0,	0,	&npr17,	"rla"},

	{12,	CND_NONE,0,0,	0,	&npr18,	"jr :3"},
	{11,	CND_NONE,0,0,	0,	&npr19,	"add hl,de"},
	{7,	CND_NONE,0,0,	0,	&npr1A,	"ld a,(de)"},
	{6,	CND_NONE,0,0,	0,	&npr1B,	"dec de"},
	{4,	CND_NONE,0,0,	0,	&npr1C,	"inc e"},
	{4,	CND_NONE,0,0,	0,	&npr1D,	"dec e"},
	{7,	CND_NONE,0,0,	0,	&npr1E,	"ld e,:1"},
	{4,	CND_NONE,0,0,	0,	&npr1F,	"rra"},

	{7,	CND_Z,0,5,	0,	&npr20,	"jr nz,:3"},
	{10,	CND_NONE,0,0,	0,	&npr21,	"ld hl,:2"},
	{16,	CND_NONE,0,0,	0,	&npr22,	"ld (:2),hl"},
	{6,	CND_NONE,0,0,	0,	&npr23,	"inc hl"},
	{4,	CND_NONE,0,0,	0,	&npr24,	"inc h"},
	{4,	CND_NONE,0,0,	0,	&npr25,	"dec h"},
	{7,	CND_NONE,0,0,	0,	&npr26,	"ld h,:1"},
	{4,	CND_NONE,0,0,	0,	&npr27,	"daa"},

	{7,	CND_Z,5,0,	0,	&npr28,	"jr z,:3"},
	{11,	CND_NONE,0,0,	0,	&npr29,	"add hl,hl"},
	{16,	CND_NONE,0,0,	0,	&npr2A,	"ld hl,(:2)"},
	{6,	CND_NONE,0,0,	0,	&npr2B,	"dec hl"},
	{4,	CND_NONE,0,0,	0,	&npr2C,	"inc l"},
	{4,	CND_NONE,0,0,	0,	&npr2D,	"dec l"},
	{7,	CND_NONE,0,0,	0,	&npr2E,	"ld l,:1"},
	{4,	CND_NONE,0,0,	0,	&npr2F,	"cpl"},

	{7,	CND_C,0,5,	0,	&npr30,	"jr nc,:3"},
	{10,	CND_NONE,0,0,	0,	&npr31,	"ld sp,:2"},
	{13,	CND_NONE,0,0,	0,	&npr32,	"ld (:2),a"},
	{6,	CND_NONE,0,0,	0,	&npr33,	"inc sp"},
	{11,	CND_NONE,0,0,	0,	&npr34,	"inc (hl)"},
	{11,	CND_NONE,0,0,	0,	&npr35,	"dec (hl)"},
	{10,	CND_NONE,0,0,	0,	&npr36,	"ld (hl),:1"},
	{4,	CND_NONE,0,0,	0,	&npr37,	"scf"},

	{7,	CND_C,5,0,	0,	&npr38,	"jr c,:3"},
	{11,	CND_NONE,0,0,	0,	&npr39,	"add hl,sp"},
	{13,	CND_NONE,0,0,	0,	&npr3A,	"ld a,(:2)"},
	{6,	CND_NONE,0,0,	0,	&npr3B,	"dec sp"},
	{4,	CND_NONE,0,0,	0,	&npr3C,	"inc a"},
	{4,	CND_NONE,0,0,	0,	&npr3D,	"dec a"},
	{7,	CND_NONE,0,0,	0,	&npr3E,	"ld a,:1"},
	{4,	CND_NONE,0,0,	0,	&npr3F,	"ccf"},

	{4,	CND_NONE,0,0,	0,	&npr40,	"ld b,b"},
	{4,	CND_NONE,0,0,	0,	&npr41,	"ld b,c"},
	{4,	CND_NONE,0,0,	0,	&npr42,	"ld b,d"},
	{4,	CND_NONE,0,0,	0,	&npr43,	"ld b,e"},
	{4,	CND_NONE,0,0,	0,	&npr44,	"ld b,h"},
	{4,	CND_NONE,0,0,	0,	&npr45,	"ld b,l"},
	{7,	CND_NONE,0,0,	0,	&npr46,	"ld b,(hl)"},
	{4,	CND_NONE,0,0,	0,	&npr47,	"ld b,a"},

	{4,	CND_NONE,0,0,	0,	&npr48,	"ld c,b"},
	{4,	CND_NONE,0,0,	0,	&npr49,	"ld c,c"},
	{4,	CND_NONE,0,0,	0,	&npr4A,	"ld c,d"},
	{4,	CND_NONE,0,0,	0,	&npr4B,	"ld c,e"},
	{4,	CND_NONE,0,0,	0,	&npr4C,	"ld c,h"},
	{4,	CND_NONE,0,0,	0,	&npr4D,	"ld c,l"},
	{7,	CND_NONE,0,0,	0,	&npr4E,	"ld c,(hl)"},
	{4,	CND_NONE,0,0,	0,	&npr4F,	"ld c,a"},

	{4,	CND_NONE,0,0,	0,	&npr50,	"ld d,b"},
	{4,	CND_NONE,0,0,	0,	&npr51,	"ld d,c"},
	{4,	CND_NONE,0,0,	0,	&npr52,	"ld d,d"},
	{4,	CND_NONE,0,0,	0,	&npr53,	"ld d,e"},
	{4,	CND_NONE,0,0,	0,	&npr54,	"ld d,h"},
	{4,	CND_NONE,0,0,	0,	&npr55,	"ld d,l"},
	{7,	CND_NONE,0,0,	0,	&npr56,	"ld d,(hl)"},
	{4,	CND_NONE,0,0,	0,	&npr57,	"ld d,a"},

	{4,	CND_NONE,0,0,	0,	&npr58,	"ld e,b"},
	{4,	CND_NONE,0,0,	0,	&npr59,	"ld e,c"},
	{4,	CND_NONE,0,0,	0,	&npr5A,	"ld e,d"},
	{4,	CND_NONE,0,0,	0,	&npr5B,	"ld e,e"},
	{4,	CND_NONE,0,0,	0,	&npr5C,	"ld e,h"},
	{4,	CND_NONE,0,0,	0,	&npr5D,	"ld e,l"},
	{7,	CND_NONE,0,0,	0,	&npr5E,	"ld e,(hl)"},
	{4,	CND_NONE,0,0,	0,	&npr5F,	"ld e,a"},

	{4,	CND_NONE,0,0,	0,	&npr60,	"ld h,b"},
	{4,	CND_NONE,0,0,	0,	&npr61,	"ld h,c"},
	{4,	CND_NONE,0,0,	0,	&npr62,	"ld h,d"},
	{4,	CND_NONE,0,0,	0,	&npr63,	"ld h,e"},
	{4,	CND_NONE,0,0,	0,	&npr64,	"ld h,h"},
	{4,	CND_NONE,0,0,	0,	&npr65,	"ld h,l"},
	{7,	CND_NONE,0,0,	0,	&npr66,	"ld h,(hl)"},
	{4,	CND_NONE,0,0,	0,	&npr67,	"ld h,a"},

	{4,	CND_NONE,0,0,	0,	&npr68,	"ld l,b"},
	{4,	CND_NONE,0,0,	0,	&npr69,	"ld l,c"},
	{4,	CND_NONE,0,0,	0,	&npr6A,	"ld l,d"},
	{4,	CND_NONE,0,0,	0,	&npr6B,	"ld l,e"},
	{4,	CND_NONE,0,0,	0,	&npr6C,	"ld l,h"},
	{4,	CND_NONE,0,0,	0,	&npr6D,	"ld l,l"},
	{7,	CND_NONE,0,0,	0,	&npr6E,	"ld l,(hl)"},
	{4,	CND_NONE,0,0,	0,	&npr6F,	"ld l,a"},

	{7,	CND_NONE,0,0,	0,	&npr70,	"ld (hl),b"},
	{7,	CND_NONE,0,0,	0,	&npr71,	"ld (hl),c"},
	{7,	CND_NONE,0,0,	0,	&npr72,	"ld (hl),d"},
	{7,	CND_NONE,0,0,	0,	&npr73,	"ld (hl),e"},
	{7,	CND_NONE,0,0,	0,	&npr74,	"ld (hl),h"},
	{7,	CND_NONE,0,0,	0,	&npr75,	"ld (hl),l"},
	{4,	CND_NONE,0,0,	0,	&npr76,	"halt"},
	{7,	CND_NONE,0,0,	0,	&npr77,	"ld (hl),a"},

	{4,	CND_NONE,0,0,	0,	&npr78,	"ld a,b"},
	{4,	CND_NONE,0,0,	0,	&npr79,	"ld a,c"},
	{4,	CND_NONE,0,0,	0,	&npr7A,	"ld a,d"},
	{4,	CND_NONE,0,0,	0,	&npr7B,	"ld a,e"},
	{4,	CND_NONE,0,0,	0,	&npr7C,	"ld a,h"},
	{4,	CND_NONE,0,0,	0,	&npr7D,	"ld a,l"},
	{7,	CND_NONE,0,0,	0,	&npr7E,	"ld a,(hl)"},
	{4,	CND_NONE,0,0,	0,	&npr7F,	"ld a,a"},

	{4,	CND_NONE,0,0,	0,	&npr80,	"add a,b"},
	{4,	CND_NONE,0,0,	0,	&npr81,	"add a,c"},
	{4,	CND_NONE,0,0,	0,	&npr82,	"add a,d"},
	{4,	CND_NONE,0,0,	0,	&npr83,	"add a,e"},
	{4,	CND_NONE,0,0,	0,	&npr84,	"add a,h"},
	{4,	CND_NONE,0,0,	0,	&npr85,	"add a,l"},
	{7,	CND_NONE,0,0,	0,	&npr86,	"add a,(hl)"},
	{4,	CND_NONE,0,0,	0,	&npr87,	"add a,a"},

	{4,	CND_NONE,0,0,	0,	&npr88,	"adc a,b"},
	{4,	CND_NONE,0,0,	0,	&npr89,	"adc a,c"},
	{4,	CND_NONE,0,0,	0,	&npr8A,	"adc a,d"},
	{4,	CND_NONE,0,0,	0,	&npr8B,	"adc a,e"},
	{4,	CND_NONE,0,0,	0,	&npr8C,	"adc a,h"},
	{4,	CND_NONE,0,0,	0,	&npr8D,	"adc a,l"},
	{7,	CND_NONE,0,0,	0,	&npr8E,	"adc a,(hl)"},
	{4,	CND_NONE,0,0,	0,	&npr8F,	"adc a,a"},

	{4,	CND_NONE,0,0,	0,	&npr90,	"sub b"},
	{4,	CND_NONE,0,0,	0,	&npr91,	"sub c"},
	{4,	CND_NONE,0,0,	0,	&npr92,	"sub d"},
	{4,	CND_NONE,0,0,	0,	&npr93,	"sub e"},
	{4,	CND_NONE,0,0,	0,	&npr94,	"sub h"},
	{4,	CND_NONE,0,0,	0,	&npr95,	"sub l"},
	{7,	CND_NONE,0,0,	0,	&npr96,	"sub (hl)"},
	{4,	CND_NONE,0,0,	0,	&npr97,	"sub a"},

	{4,	CND_NONE,0,0,	0,	&npr98,	"sbc a,b"},
	{4,	CND_NONE,0,0,	0,	&npr99,	"sbc a,c"},
	{4,	CND_NONE,0,0,	0,	&npr9A,	"sbc a,d"},
	{4,	CND_NONE,0,0,	0,	&npr9B,	"sbc a,e"},
	{4,	CND_NONE,0,0,	0,	&npr9C,	"sbc a,h"},
	{4,	CND_NONE,0,0,	0,	&npr9D,	"sbc a,l"},
	{7,	CND_NONE,0,0,	0,	&npr9E,	"sbc a,(hl)"},
	{4,	CND_NONE,0,0,	0,	&npr9F,	"sbc a,a"},

	{4,	CND_NONE,0,0,	0,	&nprA0,	"and b"},
	{4,	CND_NONE,0,0,	0,	&nprA1,	"and c"},
	{4,	CND_NONE,0,0,	0,	&nprA2,	"and d"},
	{4,	CND_NONE,0,0,	0,	&nprA3,	"and e"},
	{4,	CND_NONE,0,0,	0,	&nprA4,	"and h"},
	{4,	CND_NONE,0,0,	0,	&nprA5,	"and l"},
	{7,	CND_NONE,0,0,	0,	&nprA6,	"and (hl)"},
	{4,	CND_NONE,0,0,	0,	&nprA7,	"and a"},

	{4,	CND_NONE,0,0,	0,	&nprA8,	"xor b"},
	{4,	CND_NONE,0,0,	0,	&nprA9,	"xor c"},
	{4,	CND_NONE,0,0,	0,	&nprAA,	"xor d"},
	{4,	CND_NONE,0,0,	0,	&nprAB,	"xor e"},
	{4,	CND_NONE,0,0,	0,	&nprAC,	"xor h"},
	{4,	CND_NONE,0,0,	0,	&nprAD,	"xor l"},
	{7,	CND_NONE,0,0,	0,	&nprAE,	"xor (hl)"},
	{4,	CND_NONE,0,0,	0,	&nprAF,	"xor a"},

	{4,	CND_NONE,0,0,	0,	&nprB0,	"or b"},
	{4,	CND_NONE,0,0,	0,	&nprB1,	"or c"},
	{4,	CND_NONE,0,0,	0,	&nprB2,	"or d"},
	{4,	CND_NONE,0,0,	0,	&nprB3,	"or e"},
	{4,	CND_NONE,0,0,	0,	&nprB4,	"or h"},
	{4,	CND_NONE,0,0,	0,	&nprB5,	"or l"},
	{7,	CND_NONE,0,0,	0,	&nprB6,	"or (hl)"},
	{4,	CND_NONE,0,0,	0,	&nprB7,	"or a"},

	{4,	CND_NONE,0,0,	0,	&nprB8,	"cp b"},
	{4,	CND_NONE,0,0,	0,	&nprB9,	"cp c"},
	{4,	CND_NONE,0,0,	0,	&nprBA,	"cp d"},
	{4,	CND_NONE,0,0,	0,	&nprBB,	"cp e"},
	{4,	CND_NONE,0,0,	0,	&nprBC,	"cp h"},
	{4,	CND_NONE,0,0,	0,	&nprBD,	"cp l"},
	{7,	CND_NONE,0,0,	0,	&nprBE,	"cp (hl)"},
	{4,	CND_NONE,0,0,	0,	&nprBF,	"cp a"},

	{5,	CND_Z,0,6,	0,	&nprC0,	"ret nz"},
	{10,	CND_NONE,0,0,	0,	&nprC1,	"pop bc"},
	{10,	CND_NONE,0,0,	0,	&nprC2,	"jp nz,:2"},
	{10,	CND_NONE,0,0,	0,	&nprC3,	"jp :2"},
	{10,	CND_Z,0,7,	0,	&nprC4,	"call nz,:2"},
	{11,	CND_NONE,0,0,	0,	&nprC5,	"push bc"},
	{7,	CND_NONE,0,0,	0,	&nprC6,	"add a,:1"},
	{11,	CND_NONE,0,0,	0,	&nprC7,	"rst #00"},

	{5,	CND_Z,6,0,	0,	&nprC8,	"ret z"},
	{10,	CND_NONE,0,0,	0,	&nprC9,	"ret"},
	{10,	CND_NONE,0,0,	0,	&nprCA,	"jp z,:2"},
	{4,	CND_NONE,0,0,	ZPREF,	&nprCB,	"#CB"},
	{10,	CND_Z,7,0,	0,	&nprCC,	"call z,:2"},
	{17,	CND_NONE,0,0,	0,	&nprCD,	"call :2"},
	{7,	CND_NONE,0,0,	0,	&nprCE,	"adc a,:1"},
	{11,	CND_NONE,0,0,	0,	&nprCF,	"rst #08"},

	{5,	CND_C,0,6,	0,	&nprD0,	"ret nc"},
	{10,	CND_NONE,0,0,	0,	&nprD1,	"pop de"},
	{10,	CND_NONE,0,0,	0,	&nprD2,	"jp nc,:2"},
	{11,	CND_NONE,0,0,	0,	&nprD3,	"out (:1),a"},
	{10,	CND_C,0,7,	0,	&nprD4,	"call nc,:2"},
	{11,	CND_NONE,0,0,	0,	&nprD5,	"push de"},
	{7,	CND_NONE,0,0,	0,	&nprD6,	"sub :1"},
	{11,	CND_NONE,0,0,	0,	&nprD7,	"rst #10"},

	{5,	CND_C,6,0,	0,	&nprD8,	"ret c"},
	{4,	CND_NONE,0,0,	0,	&nprD9,	"exx"},
	{10,	CND_NONE,0,0,	0,	&nprDA,	"jp c,:2"},
	{11,	CND_NONE,0,0,	0,	&nprDB,	"in a,(:1)"},
	{10,	CND_C,7,0,	0,	&nprDC,	"call c,:2"},
	{4,	CND_NONE,0,0,	ZPREF,	&nprDD,	"#DD"},
	{7,	CND_NONE,0,0,	0,	&nprDE,	"sbc a,:1"},
	{11,	CND_NONE,0,0,	0,	&nprDF,	"rst #18"},

	{5,	CND_P,0,6,	0,	&nprE0,	"ret po"},
	{10,	CND_NONE,0,0,	0,	&nprE1,	"pop hl"},
	{10,	CND_NONE,0,0,	0,	&nprE2,	"jp po,:2"},
	{19,	CND_NONE,0,0,	0,	&nprE3,	"ex (sp),hl"},
	{10,	CND_P,0,7,	0,	&nprE4,	"call po,:2"},
	{11,	CND_NONE,0,0,	0,	&nprE5,	"push hl"},
	{7,	CND_NONE,0,0,	0,	&nprE6,	"and :1"},
	{11,	CND_NONE,0,0,	0,	&nprE7,	"rst #20"},

	{5,	CND_P,6,0,	0,	&nprE8,	"ret pe"},
	{4,	CND_NONE,0,0,	0,	&nprE9,	"jp (hl)"},
	{10,	CND_NONE,0,0,	0,	&nprEA,	"jp pe,:2"},
	{4,	CND_NONE,0,0,	0,	&nprEB,	"ex de,hl"},
	{10,	CND_P,7,0,	0,	&nprEC,	"call pe,:2"},
	{4,	CND_NONE,0,0,	ZPREF,	&nprED,	"#ED"},
	{7,	CND_NONE,0,0,	0,	&nprEE,	"xor :1"},
	{11,	CND_NONE,0,0,	0,	&nprEF,	"rst #28"},

	{5,	CND_S,0,6,	0,	&nprF0,	"ret p"},
	{10,	CND_NONE,0,0,	0,	&nprF1,	"pop af"},
	{10,	CND_NONE,0,0,	0,	&nprF2,	"jp p,:2"},
	{4,	CND_NONE,0,0,	0,	&nprF3,	"di"},
	{10,	CND_S,0,7,	0,	&nprF4,	"call p,:2"},
	{11,	CND_NONE,0,0,	0,	&nprF5,	"push af"},
	{7,	CND_NONE,0,0,	0,	&nprF6,	"or :1"},
	{11,	CND_NONE,0,0,	0,	&nprF7,	"rst #30"},

	{5,	CND_S,6,0,	0,	&nprF8,	"ret m"},
	{6,	CND_NONE,0,0,	0,	&nprF9,	"ld sp,hl"},
	{10,	CND_NONE,0,0,	0,	&nprFA,	"jp m,:2"},
	{4,	CND_NONE,0,0,	0,	&nprFB,	"ei"},
	{10,	CND_S,7,0,	0,	&nprFC,	"call m,:2"},
	{4,	CND_NONE,0,0,	ZPREF,	&nprFD,	"#FD"},
	{7,	CND_NONE,0,0,	0,	&nprFE,	"cp :1"},
	{11,	CND_NONE,0,0,	0,	&nprFF,	"rst #38"}
};

