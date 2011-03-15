unsigned char xRLC(ZXBase* p,unsigned short ad) {
	p->cpu->mptr = ad + (signed char)p->cpu->dlt;
	p->cpu->x = p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->x].rlc.f; p->cpu->x = flag[p->cpu->x].rlc.r; p->mem->wr(p->cpu->mptr,p->cpu->x);
	return p->cpu->x;
}

unsigned char xRRC(ZXBase* p,unsigned short ad) {
	p->cpu->mptr = ad + (signed char)p->cpu->dlt;
	p->cpu->x = p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->x].rrc.f; p->cpu->x = flag[p->cpu->x].rrc.r; p->mem->wr(p->cpu->mptr,p->cpu->x);
	return p->cpu->x;
}

unsigned char xRL(ZXBase* p,unsigned short ad) {
	p->cpu->mptr = ad + (signed char)p->cpu->dlt;
	p->cpu->dlt = p->mem->rd(p->cpu->mptr); p->cpu->x = flag[p->cpu->dlt].rl[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->dlt].rl[p->cpu->f & FC].f;
	p->mem->wr(p->cpu->mptr,p->cpu->x);
	return p->cpu->x;
}

unsigned char xRR(ZXBase* p,unsigned short ad) {
	p->cpu->mptr = ad + (signed char)p->cpu->dlt;
	p->cpu->dlt = p->mem->rd(p->cpu->mptr); p->cpu->x = flag[p->cpu->dlt].rr[p->cpu->f & FC].r; p->cpu->f = flag[p->cpu->dlt].rr[p->cpu->f & FC].f;
	p->mem->wr(p->cpu->mptr,p->cpu->x);
	return p->cpu->x;
}

unsigned char xSLA(ZXBase* p,unsigned short ad) {
	p->cpu->mptr = ad + (signed char)p->cpu->dlt;
	p->cpu->dlt = p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->dlt].rl[0].f; p->cpu->x = flag[p->cpu->dlt].rl[0].r;
	p->mem->wr(p->cpu->mptr,p->cpu->x);
	return p->cpu->x;
}

unsigned char xSRA(ZXBase* p,unsigned short ad) {
	p->cpu->mptr = ad + (signed char)p->cpu->dlt;
	p->cpu->dlt = p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->dlt].sra.f; p->cpu->x = flag[p->cpu->dlt].sra.r;
	p->mem->wr(p->cpu->mptr,p->cpu->x);
	return p->cpu->x;
}

unsigned char xSLI(ZXBase* p,unsigned short ad) {
	p->cpu->mptr = ad + (signed char)p->cpu->dlt;
	p->cpu->dlt = p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->dlt].rl[1].f; p->cpu->x = flag[p->cpu->dlt].rl[1].r;
	p->mem->wr(p->cpu->mptr,p->cpu->x);
	return p->cpu->x;
}

unsigned char xSRL(ZXBase* p,unsigned short ad) {
	p->cpu->mptr = ad + (signed char)p->cpu->dlt;
	p->cpu->dlt = p->mem->rd(p->cpu->mptr); p->cpu->f = flag[p->cpu->dlt].rr[0].f; p->cpu->x = flag[p->cpu->dlt].rr[0].r;
	p->mem->wr(p->cpu->mptr,p->cpu->x);
	return p->cpu->x;
}

unsigned char xRES(ZXBase* p, unsigned short ad, unsigned char msk) {
	p->cpu->mptr = ad + (signed char)p->cpu->dlt;
	unsigned char res = p->mem->rd(p->cpu->mptr) & ~msk;
	p->mem->wr(p->cpu->mptr,res);
	return res;
}

unsigned char xSET(ZXBase* p, unsigned short ad, unsigned char msk) {
	p->cpu->mptr = ad + (signed char)p->cpu->dlt;
	unsigned char res = p->mem->rd(p->cpu->mptr) | msk;
	p->mem->wr(p->cpu->mptr,res);
	return res;
}

// sbc/adc: mptr = hl + 1 (before adding)
void rpSBC(ZXBase* p, unsigned short ar) {
	int dp = ((p->cpu->f & FC)?1:0);
	p->cpu->mptr = p->cpu->hl + 1;
	p->cpu->adr = p->cpu->hl - ar - dp;
	int urs = p->cpu->hl - ar - dp;
	int srs = (int)(signed short)p->cpu->hl - (int)(signed short)ar - dp;
	p->cpu->f = (p->cpu->hadr & (FS | F5 | F3)) | ((p->cpu->adr == 0)?FZ:0) | ((((p->cpu->hl & 0xfff) - (ar & 0xfff) - dp) < 0)?FH:0) | (((srs < -0x8000) || (srs > 0x7fff))?FP:0) | FN | ((urs < 0)?FC:0);
	p->cpu->hl = p->cpu->adr;
}

void rpADC(ZXBase* p, unsigned short ar) {
	int dp = ((p->cpu->f & FC)?1:0);
	p->cpu->mptr = p->cpu->hl + 1;
	p->cpu->adr = p->cpu->hl + ar + dp;
	int urs = p->cpu->hl + ar + dp;
	int srs = (int)(signed short)p->cpu->hl + (int)(signed short)ar + dp;
	p->cpu->f = (p->cpu->hadr & (FS | F5 | F3)) | ((p->cpu->adr == 0)?FZ:0) | ((((p->cpu->hl & 0xfff) + (ar & 0xfff) + dp) > 0xfff)?FH:0) | (((srs < -0x8000) || (srs > 0x7fff))?FP:0) | ((urs > 0xffff)?FC:0);
	p->cpu->hl = p->cpu->adr;
}

void addIX(ZXBase* p,int op) {
	int z = p->cpu->ix;
	p->cpu->mptr = p->cpu->ix + 1;
	p->cpu->ix += op;
	p->cpu->f = (p->cpu->f & (FS | FZ | FP)) | (p->cpu->hx & (F5 | F3)) | ((((z & 0xfff) + (op & 0xfff)) > 0xfff)?FH:0) | (((z + op) > 0xffff)?FC:0);
}

void addIY(ZXBase* p,int op) {
	int z = p->cpu->iy;
	p->cpu->mptr = p->cpu->iy + 1;
	p->cpu->iy += op;
	p->cpu->f = (p->cpu->f & (FS | FZ | FP)) | (p->cpu->hy & (F5 | F3)) | ((((z & 0xfff) + (op & 0xfff)) > 0xfff)?FH:0) | (((z + op) > 0xffff)?FC:0);
}

#include "pref_no.c"
#include "pref_ix.c"
#include "pref_iy.c"

#include "pref_cb.c"
#include "pref_cbx.c"
#include "pref_cby.c"

#include "pref_ed.c"