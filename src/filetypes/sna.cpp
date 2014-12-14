#include "filetypes.h"

#pragma pack (1)

typedef struct {
	unsigned char i;
	unsigned char _l,_h,_e,_d,_c,_b,_f,_a;
	unsigned char l,h,e,d,c,b,ly,hy,lx,hx;
	unsigned char flag19;
	unsigned char r;
	unsigned char f,a,lsp,hsp;
	unsigned char imod,border;
} snaHead;

#pragma pack ()

int loadSNA(ZXComp* zx, const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;

	unsigned char tmp,tmp2;
	unsigned short adr;
	char pageBuf[0x4000];
	char tmpgBuf[0x4000];
	CPU* cpu = zx->cpu;

	file.seekg(0,std::ios_base::end);	// get filesize
	size_t fileSize = file.tellg();
	file.seekg(0);

	zxReset(zx, (fileSize < 49180) ? RES_48 : RES_128);

	snaHead hd;
	file.read((char*)&hd,sizeof(snaHead));
#ifdef SELFZ80
	cpu->hl_ = (hd._h << 8) | hd._l;
	cpu->de_ = (hd._d << 8) | hd._e;
	cpu->bc_ = (hd._b << 8) | hd._c;
	cpu->af_ = (hd._a << 8) | hd._f;
	cpu->hl = (hd.h << 8) | hd.l;
	cpu->de = (hd.d << 8) | hd.e;
	cpu->bc = (hd.b << 8) | hd.c;
	cpu->af = (hd.a << 8) | hd.f;
	cpu->ix = (hd.hx << 8) | hd.lx;
	cpu->iy = (hd.hy << 8) | hd.ly;
	cpu->sp = (hd.hsp << 8) | hd.lsp;
	cpu->i = hd.i;
	cpu->r = hd.r;
	cpu->r7 = hd.r & 0x80;
	cpu->imode = hd.imod & 3;
	cpu->iff1 = (hd.flag19 & 4) ? 1 : 0;
#else
	z80ex_set_reg(cpu,regR7,hd.r & 0x7f);
	z80ex_set_reg(cpu,regHL_,(hd._h << 8) | hd._l);
	z80ex_set_reg(cpu,regDE_,(hd._d << 8) | hd._e);
	z80ex_set_reg(cpu,regBC_,(hd._b << 8) | hd._c);
	z80ex_set_reg(cpu,regAF_,(hd._a << 8) | hd._f);
	z80ex_set_reg(cpu,regHL,(hd.h << 8) | hd.l);
	z80ex_set_reg(cpu,regDE,(hd.d << 8) | hd.e);
	z80ex_set_reg(cpu,regBC,(hd.b << 8) | hd.c);
	z80ex_set_reg(cpu,regAF,(hd.a << 8) | hd.f);
	z80ex_set_reg(cpu,regIX,(hd.hx << 8) | hd.lx);
	z80ex_set_reg(cpu,regIY,(hd.hy << 8) | hd.ly);
	z80ex_set_reg(cpu,regSP,(hd.hsp << 8) | hd.lsp);
	z80ex_set_reg(cpu,regI,hd.i);
	z80ex_set_reg(cpu,regR,hd.r);
	z80ex_set_reg(cpu,regIM,hd.imod & 3);
//	z80ex_set_reg(cpu,regIFF2,(hd.flag19 & 4) ? 1 : 0);
	z80ex_set_reg(cpu,regIFF1,(hd.flag19 & 4) ? 1 : 0);
#endif
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
		zx->dosen = 0;
		zx->hw->mapMem(zx);
		memSetBank(zx->mem,MEM_BANK3,MEM_RAM,0);
		memSetPage(zx->mem,MEM_RAM,0,tmpgBuf);
		zx->vid->curscr = 5;
		adr = GETSP(cpu);		// z80ex_get_reg(cpu,regSP);
		tmp = memRd(zx->mem,adr++);
		tmp2 = memRd(zx->mem,adr++);
		SETSP(cpu,adr);			//z80ex_set_reg(cpu,regSP,adr);
		SETPC(cpu,tmp | (tmp2 << 8));	// z80ex_set_reg(cpu,regPC,tmp | (tmp2 << 8));
	} else {
		SETPC(cpu,getLEWord(&file)); // z80ex_set_reg(cpu,regPC,getLEWord(&file));
		tmp = file.get();
		zx->hw->out(zx,0x7ffd,tmp,0);
		tmp2 = file.get();
		zx->dosen = tmp2 & 1;
		for (tmp2 = 0; tmp2 < 8; tmp2++) {
			if ((tmp2 == 2) || (tmp2 == 5)) tmp2++;
			if ((tmp & 7) != tmp2) {
				file.read(pageBuf,0x4000);
				memSetPage(zx->mem,MEM_RAM,tmp2,pageBuf);
			}
		}
		memSetPage(zx->mem,MEM_RAM,tmp & 7,tmpgBuf);
	}
	tsReset(zx->ts);
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
	CPU* cpu = zx->cpu;
	Z80EX_WORD pc = GETPC(cpu);	// z80ex_get_reg(cpu,regPC);
	Z80EX_WORD sp = GETSP(cpu);	// z80ex_get_reg(cpu,regSP);
	if (sna48) {
		memWr(zx->mem,--sp,(pc & 0xff00) >> 8);
		memWr(zx->mem,--sp,pc & 0x00ff);
		SETSP(cpu,sp);	// z80ex_set_reg(cpu,regSP,sp);
	}
	file.put((char)GETI(cpu));
	putLEWord(&file,GETHL_(cpu));
	putLEWord(&file,GETDE_(cpu));
	putLEWord(&file,GETBC_(cpu));
	putLEWord(&file,GETAF_(cpu));
	putLEWord(&file,GETHL(cpu));
	putLEWord(&file,GETDE(cpu));
	putLEWord(&file,GETBC(cpu));
	putLEWord(&file,GETIY(cpu));
	putLEWord(&file,GETIX(cpu));
	file.put(GETIFF2(cpu) ? 4 : 0);
	file.put((char)(GETR(cpu) & 0x7f));
	putLEWord(&file,GETAF(cpu));
	putLEWord(&file,GETSP(cpu));
	file.put((char)GETIM(cpu));
	file.put((char)zx->vid->brdcol);

	memGetPage(zx->mem,MEM_RAM,5,pageBuf);
	file.write(pageBuf,0x4000);
	memGetPage(zx->mem,MEM_RAM,2,pageBuf);
	file.write(pageBuf,0x4000);

	if (sna48) {
		memGetPage(zx->mem,MEM_RAM,0,pageBuf);		// 0xc000 - 0xffff (48K: bank 0)
		file.write(pageBuf,0x4000);
	} else {
		bnk = zx->mem->pt[3]->num & 7;
		memGetPage(zx->mem,MEM_RAM,bnk,pageBuf);	// current bank
		file.write(pageBuf,0x4000);
		putLEWord(&file,GETPC(cpu));
		file.put((char)zx->prt0);
		file.put((char)((zx->dosen & 1) ? 0xff : 0x00));
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
