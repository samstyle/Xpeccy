#include "filetypes.h"

int loadSNA(ZXComp* zx, const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;

	uint8_t tmp,tmp2;
	uint16_t adr;
	char* pageBuf = new char[0x4000];
	char* tmpgBuf = new char[0x4000];
	Z80EX_CONTEXT* cpu = zx->cpu;

	file.seekg(0,std::ios_base::end);	// get filesize
	size_t fileSize = file.tellg();
	file.seekg(0);

	z80ex_set_reg(cpu,regI,file.get());
	z80ex_set_reg(cpu,regHL_,getLEWord(&file));
	z80ex_set_reg(cpu,regDE_,getLEWord(&file));
	z80ex_set_reg(cpu,regBC_,getLEWord(&file));
	z80ex_set_reg(cpu,regAF_,getLEWord(&file));
	z80ex_set_reg(cpu,regHL,getLEWord(&file));
	z80ex_set_reg(cpu,regDE,getLEWord(&file));
	z80ex_set_reg(cpu,regBC,getLEWord(&file));
	z80ex_set_reg(cpu,regIY,getLEWord(&file));
	z80ex_set_reg(cpu,regIX,getLEWord(&file));
	tmp = file.get();
	z80ex_set_reg(cpu,regIFF1,tmp & 4);
	z80ex_set_reg(cpu,regIFF2,tmp & 4);
	tmp = file.get();
	z80ex_set_reg(cpu,regR,tmp);
	z80ex_set_reg(cpu,regR7,tmp);
	z80ex_set_reg(cpu,regAF,getLEWord(&file));
	z80ex_set_reg(cpu,regSP,getLEWord(&file));
	tmp = file.get();
	z80ex_set_reg(cpu,regIM,tmp);
	tmp = file.get();
	zx->vid->brdcol = tmp & 7;
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
		zx->vid->curscr = false;
		adr = z80ex_get_reg(cpu,regSP);
		tmp = memRd(zx->mem,adr++);
		tmp2 = memRd(zx->mem,adr++);
		z80ex_set_reg(cpu,regSP,adr);
		z80ex_set_reg(cpu,regPC,tmp | (tmp2 << 8));
		memSetPage(zx->mem,MEM_RAM,0,tmpgBuf);
	} else {
		z80ex_set_reg(cpu,regPC,getLEWord(&file));
		tmp = file.get();
		zx->out(0x7ffd, tmp);
		zx->bdi->active = (file.get() & 1);
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
	
	uint8_t bnk,i;
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
	putLEWord(&file,z80ex_get_reg(cpu,regHL));	// file.put((char)cpu->l).put((char)cpu->h);
	putLEWord(&file,z80ex_get_reg(cpu,regDE));	// file.put((char)cpu->e).put((char)cpu->d);
	putLEWord(&file,z80ex_get_reg(cpu,regBC));	// file.put((char)cpu->c).put((char)cpu->b);
	putLEWord(&file,z80ex_get_reg(cpu,regIY));	// file.put((char)cpu->ly).put((char)cpu->hy);	// iy
	putLEWord(&file,z80ex_get_reg(cpu,regIX));	// file.put((char)cpu->lx).put((char)cpu->ly);	// ix
	file.put(z80ex_get_reg(cpu,regIFF2) ? 4 : 0);
	file.put((char)((z80ex_get_reg(cpu,regR) & 0x7f) | (z80ex_get_reg(cpu,regR7) & 0x80)));	//file.put((char)(cpu->iff2?4:0)).put((char)cpu->r);// iff2,r
	putLEWord(&file,z80ex_get_reg(cpu,regAF));	// file.put((char)cpu->f).put((char)cpu->a);		// f,a
	putLEWord(&file,z80ex_get_reg(cpu,regSP));	// file.put((char)cpu->lsp).put((char)cpu->hsp);	// SP
	file.put((char)z80ex_get_reg(cpu,regIM));		// imode
	file.put((char)zx->vid->brdcol);		// border color

	memGetPage(zx->mem,MEM_RAM,5,pageBuf);
	file.write(pageBuf,0x4000);
	memGetPage(zx->mem,MEM_RAM,2,pageBuf);
	file.write(pageBuf,0x4000);
	
	if (sna48) {
		memGetPage(zx->mem,MEM_RAM,0,pageBuf);		// 0xc000 - 0xffff (48K: bank 0)
		file.write(pageBuf,0x4000);
	} else {
		bnk = memGet(zx->mem,MEM_RAM) & 7;
		memGetPage(zx->mem,MEM_RAM,bnk,pageBuf);	// current bank
		file.write(pageBuf,0x4000);
		putLEWord(&file,z80ex_get_reg(cpu,regPC));
		file.put((char)zx->prt0);
		file.put((char)(zx->bdi->active?0xff:0x00));
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
