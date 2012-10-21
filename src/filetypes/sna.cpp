#include "filetypes.h"

typedef struct {
	unsigned char i;
	unsigned char _l,_h,_e,_d,_c,_b,_f,_a;
	unsigned char l,h,e,d,c,b,lx,hx,ly,hy;
	unsigned char flag19;
	unsigned char r;
	unsigned char f,a,lsp,hsp;
	unsigned char imod,border;
} snaHead;

int loadSNA(ZXComp* zx, const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;

	unsigned char tmp,tmp2;
	unsigned short adr;
	char* pageBuf = new char[0x4000];
	char* tmpgBuf = new char[0x4000];
	Z80EX_CONTEXT* cpu = zx->cpu;

	file.seekg(0,std::ios_base::end);	// get filesize
	size_t fileSize = file.tellg();
	file.seekg(0);

	snaHead hd;
	file.read((char*)&hd,sizeof(snaHead));

	z80ex_set_reg(cpu,regR7,hd.r & 0x7f);
	z80ex_set_reg(cpu,regHL_,(hd._h << 8) | hd._l);
	z80ex_set_reg(cpu,regDE_,(hd._d << 8) | hd._e);
	z80ex_set_reg(cpu,regBC_,(hd._b << 8) | hd._c);
	z80ex_set_reg(cpu,regAF_,(hd._a << 8) | hd._f);
	z80ex_set_reg(cpu,regHL,(hd.h << 8) | hd.l);
	z80ex_set_reg(cpu,regDE,(hd.d << 8) | hd.e);
	z80ex_set_reg(cpu,regBC,(hd.b << 8) | hd.c);
	z80ex_set_reg(cpu,regAF,(hd.a << 8) | hd.f);
	z80ex_set_reg(cpu,regIY,(hd.hx << 8) | hd.lx);
	z80ex_set_reg(cpu,regIX,(hd.hy << 8) | hd.ly);
	z80ex_set_reg(cpu,regSP,(hd.hsp << 8) | hd.lsp);
	z80ex_set_reg(cpu,regI,hd.i);
	z80ex_set_reg(cpu,regR,hd.r);
	z80ex_set_reg(cpu,regIM,hd.imod & 3);
	z80ex_set_reg(cpu,regIFF2,(hd.flag19 & 4) ? 1 : 0);
	zx->vid->brdcol = hd.border & 7;
	zx->vid->nextbrd = hd.border & 7;

	file.read(pageBuf,0x4000);
	memSetPage(zx->mem,MEM_RAM,5,pageBuf);
	file.read(pageBuf,0x4000);
	memSetPage(zx->mem,MEM_RAM,2,pageBuf);
	file.read(tmpgBuf,0x4000);
	if (fileSize < 49180) {
		zx->prt0 = 0x10;
		zx->prt1 = 0x00;
		memSetBank(zx->mem,MEM_BANK0,MEM_ROM,1);
		memSetBank(zx->mem,MEM_BANK3,MEM_RAM,0);
		memSetPage(zx->mem,MEM_RAM,0,tmpgBuf);
		zx->vid->curscr = 0;
		adr = z80ex_get_reg(cpu,regSP);
		tmp = memRd(zx->mem,adr++);
		tmp2 = memRd(zx->mem,adr++);
		z80ex_set_reg(cpu,regSP,adr);
		z80ex_set_reg(cpu,regPC,tmp | (tmp2 << 8));
	} else {
		z80ex_set_reg(cpu,regPC,getLEWord(&file));
		tmp = file.get();
		zxOut(zx,0x7ffd, tmp);
		tmp2 = file.get();
		if (tmp2 & 1) zx->bdi->flag |= BDI_ACTIVE; else zx->bdi->flag &= ~BDI_ACTIVE;
		for (tmp2 = 0; tmp2 < 8; tmp2++) {
			if ((tmp2 == 2) || (tmp2 == 5)) tmp2++;
			if ((tmp & 7) != tmp2) {
				file.read(pageBuf,0x4000);
				memSetPage(zx->mem,MEM_RAM,tmp2,pageBuf);
			}
		}
		memSetPage(zx->mem,MEM_RAM,tmp & 7,tmpgBuf);
	}
	delete(pageBuf);
	delete(tmpgBuf);
	return ERR_OK;
}

void putLEWord(std::ofstream* file, Z80EX_WORD wrd) {
	file->put((char)(wrd & 0x00ff));
	file->put((char)((wrd & 0xff00) >> 8));
}

int saveSNA(ZXComp* zx, const char* name,bool sna48) {
	std::ofstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;

	unsigned char bnk,i;
	char* pageBuf = new char[0x4000];
	Z80EX_CONTEXT* cpu = zx->cpu;
	Z80EX_WORD pc = z80ex_get_reg(cpu,regPC);
	Z80EX_WORD sp = z80ex_get_reg(cpu,regSP);
	if (sna48) {
		memWr(zx->mem,--sp,(pc & 0xff00) >> 8);
		memWr(zx->mem,--sp,pc & 0x00ff);
		z80ex_set_reg(cpu,regSP,sp);
	}
	file.put((char)z80ex_get_reg(cpu,regI));
	putLEWord(&file,z80ex_get_reg(cpu,regHL_));
	putLEWord(&file,z80ex_get_reg(cpu,regDE_));
	putLEWord(&file,z80ex_get_reg(cpu,regBC_));
	putLEWord(&file,z80ex_get_reg(cpu,regAF_));
	putLEWord(&file,z80ex_get_reg(cpu,regHL));
	putLEWord(&file,z80ex_get_reg(cpu,regDE));
	putLEWord(&file,z80ex_get_reg(cpu,regBC));
	putLEWord(&file,z80ex_get_reg(cpu,regIY));
	putLEWord(&file,z80ex_get_reg(cpu,regIX));
	file.put(z80ex_get_reg(cpu,regIFF2) ? 4 : 0);
	file.put((char)(z80ex_get_reg(cpu,regR) & 0x7f));
	putLEWord(&file,z80ex_get_reg(cpu,regAF));
	putLEWord(&file,z80ex_get_reg(cpu,regSP));
	file.put((char)z80ex_get_reg(cpu,regIM));
	file.put((char)zx->vid->brdcol);

	memGetPage(zx->mem,MEM_RAM,5,pageBuf);
	file.write(pageBuf,0x4000);
	memGetPage(zx->mem,MEM_RAM,2,pageBuf);
	file.write(pageBuf,0x4000);

	if (sna48) {
		memGetPage(zx->mem,MEM_RAM,0,pageBuf);		// 0xc000 - 0xffff (48K: bank 0)
		file.write(pageBuf,0x4000);
	} else {
		bnk = zx->mem->cram & 7;
		memGetPage(zx->mem,MEM_RAM,bnk,pageBuf);	// current bank
		file.write(pageBuf,0x4000);
		putLEWord(&file,z80ex_get_reg(cpu,regPC));
		file.put((char)zx->prt0);
		file.put((char)((zx->bdi->flag & BDI_ACTIVE) ? 0xff : 0x00));
		for (i = 0; i < 8; i++) {
			if ((i == 2) || (i == 5)) i++;
			if (i != bnk) {
				memGetPage(zx->mem,MEM_RAM,i,pageBuf);
				file.write(pageBuf,0x4000);
			}
		}
	}
	delete(pageBuf);
	return ERR_OK;
}
