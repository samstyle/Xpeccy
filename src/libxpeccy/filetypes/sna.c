#include "filetypes.h"

#pragma pack (push, 1)

typedef struct {
	unsigned char i;
	unsigned char _l,_h,_e,_d,_c,_b,_f,_a;
	unsigned char l,h,e,d,c,b,ly,hy,lx,hx;
	unsigned char flag19;
	unsigned char r;
	unsigned char f,a,lsp,hsp;
	unsigned char imod,border;
} snaHead;

#pragma pack (pop)

int loadSNA(ZXComp* comp, const char* name) {
	FILE* file = fopen(name,"rb");
	if (!file) return ERR_CANT_OPEN;
	unsigned char tmp, tmp2;
	unsigned short adr;
	char pageBuf[0x4000];
	char tmpgBuf[0x4000];

	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);
	zxReset(comp, (fileSize < 49180) ? RES_48 : RES_128);

	snaHead hd;
	fread((char*)&hd, sizeof(snaHead), 1, file);
	comp->cpu->hl_ = (hd._h << 8) | hd._l;
	comp->cpu->de_ = (hd._d << 8) | hd._e;
	comp->cpu->bc_ = (hd._b << 8) | hd._c;
	comp->cpu->af_ = (hd._a << 8) | hd._f;
	comp->cpu->hl = (hd.h << 8) | hd.l;
	comp->cpu->de = (hd.d << 8) | hd.e;
	comp->cpu->bc = (hd.b << 8) | hd.c;
	comp->cpu->af = (hd.a << 8) | hd.f;
	comp->cpu->ix = (hd.hx << 8) | hd.lx;
	comp->cpu->iy = (hd.hy << 8) | hd.ly;
	comp->cpu->sp = (hd.hsp << 8) | hd.lsp;
	comp->cpu->i = hd.i;
	comp->cpu->r = hd.r;
	comp->cpu->r7 = hd.r & 0x80;
	comp->cpu->imode = hd.imod & 3;
	comp->cpu->iff1 = (hd.flag19 & 4) ? 1 : 0;
	comp->vid->brdcol = hd.border & 7;
	comp->vid->nextbrd = hd.border & 7;

	fread(pageBuf, 0x4000, 1, file);
	memSetPage(comp->mem,MEM_RAM,5,pageBuf);
	fread(pageBuf, 0x4000, 1, file);
	memSetPage(comp->mem,MEM_RAM,2,pageBuf);
	fread(tmpgBuf, 0x4000, 1, file);

	if (fileSize < 49180) {
		comp->p7FFD = 0x10;
		comp->pEFF7 = 0x00;
		comp->dos = 0;
		comp->hw->mapMem(comp);
		memSetBank(comp->mem, MEM_BANK3, MEM_RAM,0);
		memSetPage(comp->mem, MEM_RAM, 0, tmpgBuf);
		comp->vid->curscr = 5;
		adr = (hd.hsp << 8) | hd.lsp;
		tmp = memRd(comp->mem, adr++);
		tmp2 = memRd(comp->mem, adr++);
		comp->cpu->sp = adr;
		comp->cpu->pc = tmp | (tmp2 << 8);
	} else {
		adr = fgetc(file);
		adr |= fgetc(file) << 8;
		comp->cpu->pc = adr;
		tmp = fgetc(file);
		comp->hw->out(comp,0x7ffd,tmp,0);
		tmp2 = fgetc(file);
		comp->dos = (tmp2 & 1) ? 1 : 0;
		for (tmp2 = 0; tmp2 < 8; tmp2++) {
			if ((tmp2 == 2) || (tmp2 == 5)) tmp2++;
			if ((tmp & 7) != tmp2) {
				fread(pageBuf, 0x4000, 1, file);
				memSetPage(comp->mem, MEM_RAM, tmp2, pageBuf);
			}
		}
		memSetPage(comp->mem, MEM_RAM, tmp & 7, tmpgBuf);
	}
	fclose(file);
	tsReset(comp->ts);
	return ERR_OK;
}

int saveSNA(ZXComp* comp, const char* name, int sna48) {
	FILE* file = fopen(name, "wb");
	if (!file) return ERR_CANT_OPEN;
	unsigned char bnk, i;
	unsigned short adr;
	char pageBuf[0x4000];
	if (sna48) {
		unsigned short pc = comp->cpu->pc;
		memWr(comp->mem, --comp->cpu->sp, (pc & 0xff00) >> 8);
		memWr(comp->mem, --comp->cpu->sp, pc & 0xff);
	}
	snaHead hd;
	hd._h = comp->cpu->h_; hd._l = comp->cpu->l_;
	hd._d = comp->cpu->d_; hd._e = comp->cpu->e_;
	hd._b = comp->cpu->b_; hd._c = comp->cpu->c_;
	hd._a = comp->cpu->a_; hd._f = comp->cpu->f_;
	hd.h = comp->cpu->h; hd.l = comp->cpu->l;
	hd.d = comp->cpu->d; hd.e = comp->cpu->e;
	hd.b = comp->cpu->b; hd.c = comp->cpu->c;
	hd.a = comp->cpu->a; hd.f = comp->cpu->f;
	hd.hx = comp->cpu->hx; hd.lx = comp->cpu->lx;
	hd.hy = comp->cpu->hy; hd.ly = comp->cpu->ly;
	hd.hsp = comp->cpu->hsp; hd.lsp = comp->cpu->lsp;
	hd.i = comp->cpu->i;
	hd.r = comp->cpu->r;
	hd.imod = comp->cpu->imode;
	hd.flag19 = comp->cpu->iff1 ? 4 : 0;
	hd.border = comp->vid->brdcol & 7;
	fwrite((char*)&hd, sizeof(snaHead), 1, file);
	memGetPage(comp->mem, MEM_RAM, 5, pageBuf);
	fwrite(pageBuf, 0x4000, 1, file);
	memGetPage(comp->mem, MEM_RAM, 2, pageBuf);
	fwrite(pageBuf, 0x4000, 1, file);

	if (sna48) {
		memGetPage(comp->mem, MEM_RAM, 0, pageBuf);		// 0xc000 - 0xffff (48K: bank 0)
		fwrite(pageBuf, 0x4000, 1, file);
	} else {
		bnk = comp->mem->pt[3]->num & 7;
		memGetPage(comp->mem, MEM_RAM, bnk, pageBuf);		// current bank
		fwrite(pageBuf, 0x4000, 1, file);
		adr = comp->cpu->pc;
		fputc(adr & 0xff, file);
		fputc((adr & 0xff00) >> 8, file);
		fputc(comp->p7FFD, file);
		fputc(comp->dos ? 0xff : 0x00, file);
		for (i = 0; i < 8; i++) {
			if ((i == 2) || (i == 5)) i++;
			if (i != bnk) {
				memGetPage(comp->mem, MEM_RAM, i, pageBuf);
				fwrite(pageBuf, 0x4000, 1, file);
			}
		}
	}

	fclose(file);
	return ERR_OK;
}
