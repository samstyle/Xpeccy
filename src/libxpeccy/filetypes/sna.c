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

int loadSNA(ZXComp* zx, const char* name) {
	FILE* file = fopen(name,"rb");
	if (!file) return ERR_CANT_OPEN;
	unsigned char tmp, tmp2;
	unsigned short adr;
	char pageBuf[0x4000];
	char tmpgBuf[0x4000];

	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);
	zxReset(zx, (fileSize < 49180) ? RES_48 : RES_128);

	snaHead hd;
	fread((char*)&hd, sizeof(snaHead), 1, file);
	SETHL_(zx->cpu, (hd._h << 8) | hd._l);
	SETDE_(zx->cpu, (hd._d << 8) | hd._e);
	SETBC_(zx->cpu, (hd._b << 8) | hd._c);
	SETAF_(zx->cpu, (hd._a << 8) | hd._f);
	SETHL(zx->cpu, (hd.h << 8) | hd.l);
	SETDE(zx->cpu, (hd.d << 8) | hd.e);
	SETBC(zx->cpu, (hd.b << 8) | hd.c);
	SETAF(zx->cpu, (hd.a << 8) | hd.f);
	SETIX(zx->cpu, (hd.hx << 8) | hd.lx);
	SETIY(zx->cpu, (hd.hy << 8) | hd.ly);
	SETSP(zx->cpu, (hd.hsp << 8) | hd.lsp);
	SETI(zx->cpu, hd.i);
	SETR(zx->cpu, hd.r);
	SETIM(zx->cpu, hd.imod & 3);
	SETIFF1(zx->cpu, (hd.flag19 & 4) ? 1 : 0);
	zx->vid->brdcol = hd.border & 7;
	zx->vid->nextbrd = hd.border & 7;

	fread(pageBuf, 0x4000, 1, file);
	memSetPage(zx->mem,MEM_RAM,5,pageBuf);
	fread(pageBuf, 0x4000, 1, file);
	memSetPage(zx->mem,MEM_RAM,2,pageBuf);
	fread(tmpgBuf, 0x4000, 1, file);

	if (fileSize < 49180) {
		zx->prt0 = 0x10;
		zx->prt1 = 0x00;
		zx->dosen = 0;
		zx->hw->mapMem(zx);
		memSetBank(zx->mem, MEM_BANK3, MEM_RAM,0);
		memSetPage(zx->mem, MEM_RAM, 0, tmpgBuf);
		zx->vid->curscr = 5;
		adr = (hd.hsp << 8) | hd.lsp;
		tmp = memRd(zx->mem, adr++);
		tmp2 = memRd(zx->mem, adr++);
		SETSP(zx->cpu,adr);
		SETPC(zx->cpu,tmp | (tmp2 << 8));
	} else {
		adr = fgetc(file);
		adr |= fgetc(file) << 8;
		SETPC(zx->cpu, adr);
		tmp = fgetc(file);
		zx->hw->out(zx,0x7ffd,tmp,0);
		tmp2 = fgetc(file);
		zx->dosen = (tmp2 & 1) ? 1 : 0;
		for (tmp2 = 0; tmp2 < 8; tmp2++) {
			if ((tmp2 == 2) || (tmp2 == 5)) tmp2++;
			if ((tmp & 7) != tmp2) {
				fread(pageBuf, 0x4000, 1, file);
				memSetPage(zx->mem, MEM_RAM, tmp2, pageBuf);
			}
		}
		memSetPage(zx->mem, MEM_RAM, tmp & 7, tmpgBuf);
	}
	fclose(file);
	tsReset(zx->ts);
	return ERR_OK;
}

int saveSNA(ZXComp* zx, const char* name, int sna48) {
	FILE* file = fopen(name, "wb");
	if (!file) return ERR_CANT_OPEN;
	unsigned char bnk, i;
	unsigned short adr;
	char pageBuf[0x4000];
	if (sna48) {
		Z80EX_WORD pc = GETPC(zx->cpu);
		adr = GETSP(zx->cpu);
		memWr(zx->mem, --adr, (pc & 0xff00) >> 8);
		memWr(zx->mem, --adr, pc & 0xff);
		SETSP(zx->cpu, adr);
	}
	snaHead hd;
	adr = GETHL_(zx->cpu); hd._h = (adr & 0xff00) >> 8; hd._l = adr & 0xff;
	adr = GETDE_(zx->cpu); hd._d = (adr & 0xff00) >> 8; hd._e = adr & 0xff;
	adr = GETBC_(zx->cpu); hd._b = (adr & 0xff00) >> 8; hd._c = adr & 0xff;
	adr = GETAF_(zx->cpu); hd._a = (adr & 0xff00) >> 8; hd._f = adr & 0xff;
	adr = GETHL(zx->cpu); hd.h = (adr & 0xff00) >> 8; hd.l = adr & 0xff;
	adr = GETDE(zx->cpu); hd.d = (adr & 0xff00) >> 8; hd.e = adr & 0xff;
	adr = GETBC(zx->cpu); hd.b = (adr & 0xff00) >> 8; hd.c = adr & 0xff;
	adr = GETAF(zx->cpu); hd.a = (adr & 0xff00) >> 8; hd.f = adr & 0xff;
	adr = GETIX(zx->cpu); hd.hx = (adr & 0xff00) >> 8; hd.lx = adr & 0xff;
	adr = GETIY(zx->cpu); hd.hy = (adr & 0xff00) >> 8; hd.ly = adr & 0xff;
	adr = GETSP(zx->cpu); hd.hsp = (adr & 0xff00) >> 8; hd.lsp = adr & 0xff;
	hd.i = GETI(zx->cpu);
	hd.r = GETR(zx->cpu);
	hd.imod = GETIM(zx->cpu);
	hd.flag19 = GETIFF1(zx->cpu) ? 4 : 0;
	hd.border = zx->vid->brdcol & 7;
	fwrite((char*)&hd, sizeof(snaHead), 1, file);
	memGetPage(zx->mem, MEM_RAM, 5, pageBuf);
	fwrite(pageBuf, 0x4000, 1, file);
	memGetPage(zx->mem, MEM_RAM, 2, pageBuf);
	fwrite(pageBuf, 0x4000, 1, file);

	if (sna48) {
		memGetPage(zx->mem, MEM_RAM, 0, pageBuf);		// 0xc000 - 0xffff (48K: bank 0)
		fwrite(pageBuf, 0x4000, 1, file);
	} else {
		bnk = zx->mem->pt[3]->num & 7;
		memGetPage(zx->mem, MEM_RAM, bnk, pageBuf);		// current bank
		fwrite(pageBuf, 0x4000, 1, file);
		adr = GETPC(zx->cpu);
		fputc(adr & 0xff, file);
		fputc((adr & 0xff00) >> 8, file);
		fputc(zx->prt0, file);
		fputc(zx->dosen ? 0xff : 0x00, file);
		for (i = 0; i < 8; i++) {
			if ((i == 2) || (i == 5)) i++;
			if (i != bnk) {
				memGetPage(zx->mem, MEM_RAM, i, pageBuf);
				fwrite(pageBuf, 0x4000, 1, file);
			}
		}
	}

	fclose(file);
	return ERR_OK;
}
