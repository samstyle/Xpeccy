#include "filetypes.h"

uint16_t getLEWord(std::ifstream* file) {
	uint16_t res = file->get();
	res += (file->get() << 8);
	return res;
}

uint16_t getBEWord(std::ifstream* file) {
	uint16_t res = file->get();
	res = (res << 8) + file->get();
	return res;
}

void z80uncompress(std::ifstream* file,char* buf,int maxlen) {
	char *ptr = buf;
	uint8_t tmp,tmp2;
	uint8_t lst = 0xed;
	bool btm = true;
	do {
		tmp = file->get();
		if (tmp == 0xed) {
			tmp = file->get();
			if (tmp == 0xed) {
				tmp2 = file->get();
				if ((lst == 0x00) && (tmp2 == 0x00)) {
					btm = false;	// stop @ 00 ed ed 00
				} else {
					tmp = file->get();
					do {
						*(ptr++) = tmp;
						tmp2--;
					} while (tmp2);
				}
			} else {
				*(ptr++) = 0xed;
				*(ptr++) = tmp;
				lst = tmp;
			}
		} else {
			*(ptr++) = tmp;
			lst = tmp;
		}
	} while (btm && !file->eof() && (ptr - buf < maxlen));
}

uint8_t z80readblock(std::ifstream* file,char* buf) {
	uint8_t tmp,tmp2;
	int adr;
	tmp = file->get(); tmp2 = file->get(); adr = tmp + (tmp2 << 8);	// compr.page size
	tmp = file->get();						// page num
	if (adr == 0xffff) {
		file->read(buf,0x4000);
	} else {
		z80uncompress(file,buf,0x4000);
	}
	return tmp;
}

const char* v2hardware[16] = {
	"48k","48k + If.1","SamRam","128k","128k + If.1","unknown","unknown",
	"Spectrum +3","unknown","Pentagon 128K","Scorpion 256K","Didaktik",
	"Spectrum +2","Spectrum +2A","TC1048","TC2068"
};

const char* v3hardware[16] = {
	"48k","48k + If.1","SamRam","48k + M.G.T","128k","128k + If.1","128k + M.G.T.",
	"Spectrum +3","unknown","Pentagon 128K","Scorpion 256K","Didaktik",
	"Spectrum +2","Spectrum +2A","TC1048","TC2068"
};

int loadZ80(ZXComp* zx, const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;

	bool btm;
	uint8_t tmp,tmp2,lst;
	uint16_t adr;
	Z80EX_CONTEXT* cpu = zx->cpu;
	char* pageBuf = new char[0xc000];
	
	zx->prt0 = 0x10;
	zx->prt1 = 0x00;
	memSetBank(zx->mem,MEM_BANK0,MEM_ROM,1);
	memSetBank(zx->mem,MEM_BANK3,MEM_RAM,0);
	zx->vid->curscr = false;
	z80ex_set_reg(cpu,regAF,getBEWord(&file));
	z80ex_set_reg(cpu,regBC,getLEWord(&file));
	z80ex_set_reg(cpu,regHL,getLEWord(&file));
	z80ex_set_reg(cpu,regPC,getLEWord(&file));
	z80ex_set_reg(cpu,regSP,getLEWord(&file));
	z80ex_set_reg(cpu,regI,file.get());
	tmp2 = file.get() & 0x7f;
	tmp = file.get();
	if (tmp == 0xff) tmp = 0x01;
	if (tmp & 1) tmp2 |= 0x80;
	z80ex_set_reg(cpu,regR,tmp);
	z80ex_set_reg(cpu,regR7,tmp);
	zx->vid->brdcol = (tmp >> 1) & 7;
	z80ex_set_reg(cpu,regDE,getLEWord(&file));
	z80ex_set_reg(cpu,regBC_,getLEWord(&file));
	z80ex_set_reg(cpu,regDE_,getLEWord(&file));
	z80ex_set_reg(cpu,regHL_,getLEWord(&file));
	z80ex_set_reg(cpu,regAF_,getBEWord(&file));
	z80ex_set_reg(cpu,regIY,getLEWord(&file));
	z80ex_set_reg(cpu,regIX,getLEWord(&file));
	z80ex_set_reg(cpu,regIFF1,file.get());
	z80ex_set_reg(cpu,regIFF2,file.get());
	tmp2 = file.get();
	z80ex_set_reg(cpu,regIM,tmp2 & 3);
	if (z80ex_get_reg(cpu,regPC) == 0) {
		tmp = file.get();
		tmp2 = file.get();
		adr = tmp + (tmp2 << 8);
		z80ex_set_reg(cpu,regPC,getLEWord(&file));
		lst = file.get();			// 34: HW mode
		tmp = file.get(); zx->out(0x7ffd,tmp);	// 35: 7FFD last out
		tmp = file.get();			// 36: skip (IF1)
		tmp = file.get();			// 37: skip (flags) TODO
		tmp = file.get(); zx->out(0xfffd,tmp);	// 38: last out to fffd
		for (tmp2 = 0; tmp2 < 16; tmp2++) {
			tmp = file.get();
			tsSet(zx->ts,CHIP_A_REG,tmp2,tmp);
		}
		if (adr > 23) {
printf(".z80 version 3\n");
			if (lst < 16) printf("Hardware: %s\n",v3hardware[lst]);
			switch (lst) {
				case 0: lst = 1; break;		// 48K
				case 4: lst = 2; break;
				case 9: lst = 2; break;		// 128K
				case 10: lst = 3; break;	// 256K
				default: lst = 0; break;	// undef
			}
			file.seekg(adr-23,std::ios_base::cur);	// skip all other bytes
		} else {
printf(".z80 version 2\n");
			if (lst < 16) printf("Hardware: %s\n",v2hardware[lst]);
			switch (lst) {
				case 0: lst = 1; break;
				case 3: lst = 2; break;
				case 9: lst = 2; break;		// 128K
				case 10: lst = 3; break;	// 256K
				default: lst = 0; break;	// undef
			}
		}
		switch (lst) {
			case 1:
				btm = true;
				do {
					tmp = z80readblock(&file,pageBuf);
					switch (tmp) {
						case 4: memSetPage(zx->mem,MEM_RAM,2,pageBuf); break;
						case 5: memSetPage(zx->mem,MEM_RAM,0,pageBuf); break;
						case 8: memSetPage(zx->mem,MEM_RAM,5,pageBuf); break;
						default: btm = false; break;
					}
				} while (btm && !file.eof());
				break;
			case 2:
				btm = true;
				do {
					tmp = z80readblock(&file,pageBuf);
					if ((tmp > 2) && (tmp < 11)) {
						memSetPage(zx->mem,MEM_RAM,tmp-3,pageBuf);
					} else {
						btm = false;
					}
				} while (btm && !file.eof());
				break;
			case 3:
				btm = true;
				do {
					tmp = z80readblock(&file,pageBuf);
					if ((tmp > 2) && (tmp < 19)) {
						memSetPage(zx->mem,MEM_RAM,tmp-3,pageBuf);
					} else {
						btm = false;
					}
				} while (btm && !file.eof());
				break;
			default:
				printf("Hardware mode not supported. reset\n");
				z80ex_reset(cpu);
				break;
		}
	} else {			// version 1
printf(".z80 version 1\n");
		if (tmp & 0x20) {
			z80uncompress(&file,pageBuf,0xc000);
			memSetPage(zx->mem,MEM_RAM,5,pageBuf);
			memSetPage(zx->mem,MEM_RAM,2,pageBuf + 0x4000);
			memSetPage(zx->mem,MEM_RAM,0,pageBuf + 0x8000);
		} else {
			file.read(pageBuf,0x4000);
			memSetPage(zx->mem,MEM_RAM,5,pageBuf);
			file.read(pageBuf,0x4000);
			memSetPage(zx->mem,MEM_RAM,2,pageBuf);
			file.read(pageBuf,0x4000);
			memSetPage(zx->mem,MEM_RAM,0,pageBuf);
		}
	}
	delete(pageBuf);
	return ERR_OK;
}