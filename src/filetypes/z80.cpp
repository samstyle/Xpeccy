#include "filetypes.h"

typedef struct {
	unsigned char a,f,c,b,l,h;
	unsigned char pcl,pch;
	unsigned char spl,sph;
	unsigned char i,r7,flag12;
	unsigned char e,d;
	unsigned char _c,_b,_e,_d,_l,_h,_a,_f;
	unsigned char iyl,iyh;
	unsigned char ixl,ixh;
	unsigned char iff1,iff2,flag29;
} z80v1Header;

unsigned short getLEWord(std::ifstream* file) {
	unsigned short res = file->get();
	res += (file->get() << 8);
	return res;
}

unsigned short getBEWord(std::ifstream* file) {
	unsigned short res = file->get();
	res = (res << 8) + file->get();
	return res;
}

void z80uncompress(std::ifstream* file,char* buf,int maxlen) {
	char *ptr = buf;
	unsigned char tmp,tmp2;
	unsigned char lst = 0xed;
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
					} while (tmp2 && (ptr - buf < maxlen));
				}
			} else {
				*(ptr++) = 0xed;
				if (ptr - buf < maxlen) *(ptr++) = tmp;
				lst = tmp;
			}
		} else {
			*(ptr++) = tmp;
			lst = tmp;
		}
	} while (btm && !file->eof() && (ptr - buf < maxlen));
}

unsigned char z80readblock(std::ifstream* file,char* buf) {
	unsigned char tmp,tmp2;
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
	unsigned char tmp,tmp2,lst;
	unsigned short adr;
	CPU* cpu = zx->cpu;
	char* pageBuf = new char[0xc000];
	z80v1Header head;
	zx->prt0 = 0x10;
	zx->prt1 = 0x00;
	memSetBank(zx->mem,MEM_BANK0,MEM_ROM,1);
	memSetBank(zx->mem,MEM_BANK3,MEM_RAM,0);
	zx->vid->curscr = 0;

	file.read((char*)&head,sizeof(z80v1Header));
	if (head.flag12 == 0xff) head.flag12 = 0x01;	// Because of compatibility, if byte 12 is 255, it has to be regarded as being 1.
#ifdef SELFZ80
	cpu->a = head.a; cpu->f = head.f;
	cpu->b = head.b; cpu->c = head.c;
	cpu->d = head.d; cpu->e = head.e;
	cpu->h = head.h; cpu->l = head.l;

	cpu->a_ = head._a; cpu->f_ = head._f;
	cpu->b_ = head._b; cpu->c_ = head._c;
	cpu->d_ = head._d; cpu->e_ = head._e;
	cpu->h_ = head._h; cpu->l_ = head._l;

	cpu->hpc = head.pch; cpu->lpc = head.pcl;
	cpu->hsp = head.sph; cpu->lsp = head.spl;
	cpu->i = head.i;
	cpu->r7 = (head.flag12 & 0x01) ? 0x80 : 0x00;
	cpu->r = (head.r7 & 0x7f) | cpu->r7;
	cpu->imode = head.flag29 & 3;
	cpu->iff1 = head.iff1;
	cpu->iff2 = head.iff2;
#else
	z80ex_set_reg(cpu,regAF,(head.a << 8) + head.f);
	z80ex_set_reg(cpu,regBC,(head.b << 8) + head.c);
	z80ex_set_reg(cpu,regDE,(head.d << 8) + head.e);
	z80ex_set_reg(cpu,regHL,(head.h << 8) + head.l);

	z80ex_set_reg(cpu,regAF_,(head._a << 8) + head._f);
	z80ex_set_reg(cpu,regBC_,(head._b << 8) + head._c);
	z80ex_set_reg(cpu,regDE_,(head._d << 8) + head._e);
	z80ex_set_reg(cpu,regHL_,(head._h << 8) + head._l);

	z80ex_set_reg(cpu,regPC,(head.pch << 8) + head.pcl);
	z80ex_set_reg(cpu,regSP,(head.sph << 8) + head.spl);
	z80ex_set_reg(cpu,regI,head.i);
	z80ex_set_reg(cpu,regR7,(head.flag12 & 0x01) ? 0x80 : 0x00);
	z80ex_set_reg(cpu,regR,(head.r7 & 0x7f) | ((head.flag12 & 0x01) ? 0x80 : 0x00));
	z80ex_set_reg(cpu,regIM,head.flag29 & 3);
	z80ex_set_reg(cpu,regIFF1,head.iff1);
	z80ex_set_reg(cpu,regIFF2,head.iff2);
#endif
	zx->vid->brdcol = (head.flag12 >> 1) & 7;
	zx->vid->nextbrd = zx->vid->brdcol;

// unsupported things list
	if (head.flag12 & 0x10) printf("...flag 12.bit 4.Basic SamRom switched in\n");
	if (head.flag29 & 0x04) printf("...flag 29.bit 2.Issue 2 emulation\n");
	if (head.flag29 & 0x08) printf("...flag 29.bit 3.Double interrupt frequency\n");
// continued
	if (GETPC(cpu) == 0) {
		tmp = file.get();
		tmp2 = file.get();
		adr = tmp + (tmp2 << 8);
		SETPC(cpu,getLEWord(&file));
		lst = file.get();			// 34: HW mode
		tmp = file.get(); //zxOut(zx,0x7ffd,tmp);	// 35: 7FFD last out
		zx->hw->out(zx,0x7ffd,tmp,0);
		tmp = file.get();			// 36: skip (IF1)
		tmp = file.get();			// 37: skip (flags) TODO
		tmp = file.get(); //zxOut(zx,0xfffd,tmp);	// 38: last out to fffd
		zx->hw->out(zx,0xfffd,tmp,0);
		for (tmp2 = 0; tmp2 < 16; tmp2++) {
			tmp = file.get();
			zx->ts->chipA->reg[tmp2] = tmp;
		}
		if (adr > 23) {
printf(".z80 version 3\n");
			if (lst < 16) printf("Hardware: %s\n",v3hardware[lst]);
			switch (lst) {
				case 0:
				case 1:
				case 2: lst = 1; break;		// 48K
				case 4:
				case 5:
				case 6:
				case 9: lst = 2; break;		// 128K
				case 10: lst = 3; break;	// 256K
				default: lst = 0; break;	// undef
			}
			file.seekg(adr-23,std::ios_base::cur);	// skip all other bytes
		} else {
printf(".z80 version 2\n");
			if (lst < 16) printf("Hardware: %s\n",v2hardware[lst]);
			switch (lst) {
				case 0:
				case 1: lst = 1; break;
				case 3:
				case 4:
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
				RESETCPU(cpu);	// z80ex_reset(cpu);
				break;
		}
	} else {			// version 1
printf(".z80 version 1\n");
		if (head.flag12 & 0x20) {
			printf("data is compressed\n");
			z80uncompress(&file,pageBuf,0xc000);
			memSetPage(zx->mem,MEM_RAM,5,pageBuf);
			memSetPage(zx->mem,MEM_RAM,2,pageBuf + 0x4000);
			memSetPage(zx->mem,MEM_RAM,0,pageBuf + 0x8000);
		} else {
			printf("data is not compressed\n");
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
