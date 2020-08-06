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

int loadSNA_f(Computer* comp, FILE* file, size_t fileSize) {

	unsigned char tmp, tmp2;
	unsigned short adr;
	char pageBuf[0x4000];
	char tmpgBuf[0x4000];

	compReset(comp, (fileSize < 49180) ? RES_48 : RES_128);

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
	comp->cpu->iff2 = 1;
	comp->cpu->inten = Z80_NMI | (comp->cpu->iff1 ? Z80_INT : 0);
	comp->vid->brdcol = hd.border & 7;
	comp->vid->nextbrd = hd.border & 7;

	if (comp->cpu->iff1) {
		comp->cpu->inten |= Z80_INT;
	} else {
		comp->cpu->inten &= ~Z80_INT;
	}

	fread(pageBuf, 0x4000, 1, file);
	memPutData(comp->mem,MEM_RAM,5,MEM_16K,pageBuf);
	fread(pageBuf, 0x4000, 1, file);
	memPutData(comp->mem,MEM_RAM,2,MEM_16K,pageBuf);
	fread(tmpgBuf, 0x4000, 1, file);

	if (fileSize < 49180) {
		comp->p7FFD = 0x10;
		comp->pEFF7 = 0x00;
		comp->rom = 1;		// set basic 48
		comp->dos = 0;
		comp->hw->mapMem(comp);
		memSetBank(comp->mem, 0xc0, MEM_RAM, 0, MEM_16K, NULL,NULL,NULL);
		memPutData(comp->mem, MEM_RAM, 0, MEM_16K, tmpgBuf);
		comp->vid->curscr = 5;
		adr =(hd.hsp << 8) | hd.lsp;
		tmp = memRd(comp->mem, adr++);
		tmp2 = memRd(comp->mem, adr++);
		comp->cpu->sp = adr;
		comp->cpu->pc = tmp | (tmp2 << 8);
	} else {
		comp->cpu->pc = fgetw(file);
		tmp = fgetc(file);		// byte out to 7ffd. b0..2 current page
		if (comp->hw->out)
			comp->hw->out(comp,0x7ffd,tmp,0);
		tmp2 = fgetc(file);
		comp->dos = (tmp2 & 1) ? 1 : 0;
		for (tmp2 = 0; tmp2 < 8; tmp2++) {
			if ((tmp2 == 2) || (tmp2 == 5)) tmp2++;
			if ((tmp & 7) != tmp2) {
				fread(pageBuf, 0x4000, 1, file);
				memPutData(comp->mem, MEM_RAM, tmp2, MEM_16K, pageBuf);
			}
		}
		memPutData(comp->mem, MEM_RAM, tmp & 7, MEM_16K, tmpgBuf);
	}
	tsReset(comp->ts);
	return ERR_OK;
}

int loadSNA(Computer* comp, const char* name, int drv) {
	FILE* file = fopen(name,"rb");
	if (!file) return ERR_CANT_OPEN;
	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);
	int res = loadSNA_f(comp, file, fileSize);
	fclose(file);
	if (res == ERR_OK)
		mem_set_path(comp->mem, name);
	return res;
}

int saveSNA(Computer* comp, const char* name, int drv) {
	FILE* file = fopen(name, "wb");
	if (!file) return ERR_CANT_OPEN;
	unsigned char bnk, i;
	int sna48 = (comp->mem->ramSize < MEM_128K) ? 1 : 0;
	if (sna48) {
		memWr(comp->mem, --comp->cpu->sp, comp->cpu->hpc);
		memWr(comp->mem, --comp->cpu->sp, comp->cpu->lpc);
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

	fwrite(comp->mem->ramData + ((5 << 14) & comp->mem->ramMask), MEM_16K, 1, file);	// page 5
	fwrite(comp->mem->ramData + ((2 << 14) & comp->mem->ramMask), MEM_16K, 1, file);	// page 2
	if (sna48) {
		fwrite(comp->mem->ramData, MEM_16K, 1, file);		// page 0
	} else {
		bnk = comp->p7FFD & 7;
		fwrite(comp->mem->ramData + ((bnk << 14) & comp->mem->ramMask), MEM_16K, 1, file);	// current page
		fputc(comp->cpu->lpc, file);				// pc
		fputc(comp->cpu->hpc, file);
		fputc(comp->p7FFD, file);				// 7ffd
		fputc(comp->dos ? 0xff : 0x00, file);			// trdos
		for (i = 0; i < 8; i++) {				// all others pages
			if ((i == 2) || (i == 5)) i++;
			if (i != bnk) {
				fwrite(comp->mem->ramData + ((i << 14) & comp->mem->ramMask), MEM_16K, 1, file);
			}
		}
	}
	fclose(file);
	mem_set_path(comp->mem, name);
	return ERR_OK;
}
