#include "filetypes.h"

#pragma pack (push, 1)

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

#pragma pack (pop)

/*
unsigned short fgetwLE(FILE* file) {		// get WORD little endian (low-hi)
	unsigned short res = fgetc(file);
	res |= (fgetc(file) << 8);
	return res;
}
*/

void fputwLE(FILE* file, unsigned short wrd) {
	fputc(wrd & 0xff, file);
	fputc((wrd & 0xff00) >> 8, file);
}

void z80uncompress(FILE* file, char* buf, int maxlen) {
	char *ptr = buf;
	unsigned char tmp,tmp2;
	unsigned char lst = 0xed;
	int btm = 1;
	do {
		tmp = fgetc(file);
		if (tmp == 0xed) {
			tmp = fgetc(file);
			if (tmp == 0xed) {
				tmp2 = fgetc(file);
				if ((lst == 0x00) && (tmp2 == 0x00)) {
					btm = 0;	// stop @ 00 ed ed 00
				} else {
					tmp = fgetc(file);
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
	} while (btm && !feof(file) && (ptr - buf < maxlen));
}

unsigned char z80readblock(FILE* file, char* buf) {
	unsigned char tmp;
	int adr;
	adr = fgetw(file);		// compressed size
	tmp = fgetc(file);		// page num
	if (adr == 0xffff) {
		fread(buf, 0x4000, 1, file);
	} else {
		z80uncompress(file, buf, 0x4000);
	}
	return tmp;
}

static const char* v2hardware[16] = {
	"48k","48k + If.1","SamRam","128k","128k + If.1","unknown","unknown",
	"Spectrum +3","unknown","Pentagon 128K","Scorpion 256K","Didaktik",
	"Spectrum +2","Spectrum +2A","TC1048","TC2068"
};

static const char* v3hardware[16] = {
	"48k","48k + If.1","SamRam","48k + M.G.T","128k","128k + If.1","128k + M.G.T.",
	"Spectrum +3","unknown","Pentagon 128K","Scorpion 256K","Didaktik",
	"Spectrum +2","Spectrum +2A","TC1048","TC2068"
};

int loadZ80(Computer* comp, const char* name, int drv) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int res = loadZ80_f(comp, file);
	fclose(file);
	if (res == ERR_OK)
		mem_set_path(comp->mem, name);
	return res;
}

int loadZ80_f(Computer* comp, FILE* file) {
	int btm;
	int err = ERR_OK;
	unsigned char tmp,tmp2,reg,lst;
	unsigned short adr, twrd;
//	CPU* cpu = comp->cpu;
	char pageBuf[0xc000];
	z80v1Header hd;
	comp->p7FFD = 0x10;
	comp->pEFF7 = 0x00;
	memSetBank(comp->mem,0x00,MEM_ROM,1,MEM_16K,NULL,NULL,NULL);
	memSetBank(comp->mem,0xc0,MEM_RAM,0,MEM_16K,NULL,NULL,NULL);
	comp->vid->curscr = 5;

	fread((char*)&hd, sizeof(z80v1Header), 1, file);
	if (hd.flag12 == 0xff) hd.flag12 = 0x01;	// Because of compatibility, if byte 12 is 255, it has to be regarded as being 1.

	comp->cpu->af = (hd.a << 8) | hd.f;
	comp->cpu->bc = (hd.b << 8) | hd.c;
	comp->cpu->de = (hd.d << 8) | hd.e;
	comp->cpu->hl = (hd.h << 8) | hd.l;
	comp->cpu->af_ = (hd._a << 8) | hd._f;
	comp->cpu->bc_ = (hd._b << 8) | hd._c;
	comp->cpu->de_ = (hd._d << 8) | hd._e;
	comp->cpu->hl_ = (hd._h << 8) | hd._l;
	comp->cpu->pc = (hd.pch << 8) | hd.pcl;
	comp->cpu->sp = (hd.sph << 8) | hd.spl;
	comp->cpu->ix = (hd.ixh << 8) | hd.ixl;
	comp->cpu->iy = (hd.iyh << 8) | hd.iyl;
	comp->cpu->i = hd.i;
	comp->cpu->r7 = (hd.flag12 & 1) ? 0x80 : 0;
	comp->cpu->r = (hd.r7 & 0x7f) | comp->cpu->r7;
	comp->cpu->imode = hd.flag29 & 3;
	comp->cpu->iff1 = hd.iff1;
	comp->cpu->iff2 = hd.iff2;
	comp->cpu->inten = Z80_NMI | (hd.iff1 ? Z80_INT : 0);
	comp->vid->brdcol = (hd.flag12 >> 1) & 7;
	comp->vid->nextbrd = comp->vid->brdcol;
// unsupported things list
	if (hd.flag12 & 0x10) printf("...flag 12.bit 4.Basic SamRom switched in\n");
	if (hd.flag29 & 0x04) printf("...flag 29.bit 2.Issue 2 emulation\n");
	if (hd.flag29 & 0x08) printf("...flag 29.bit 3.Double interrupt frequency\n");
// continued
	if (comp->cpu->pc == 0) {
		adr = fgetw(file);
		twrd = fgetw(file);
		comp->cpu->pc = twrd;
		lst = fgetc(file);			// 34: HW mode
		tmp = fgetc(file);			// 35: 7FFD last out
		comp->hw->out(comp, 0x7ffd, tmp, 0);
		tmp = fgetc(file);			// 36: skip (IF1)
		tmp = fgetc(file);			// 37: skip (flags) TODO
		reg = fgetc(file);			// 38: last out to fffd
		for (tmp2 = 0; tmp2 < 16; tmp2++) {	// AY regs
			tmp = fgetc(file);
			tsOut(comp->ts, 0xfffd, tmp2);
			tsOut(comp->ts, 0xbffd, tmp);
		}
		comp->hw->out(comp, 0xfffd, reg, 0);

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
			fseek(file, adr-23, SEEK_CUR);		// skip all other bytes
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
				btm = 1;
				do {
					tmp = z80readblock(file,pageBuf);
					switch (tmp) {
						case 4: memPutData(comp->mem,MEM_RAM,2,MEM_16K,pageBuf); break;
						case 5: memPutData(comp->mem,MEM_RAM,0,MEM_16K,pageBuf); break;
						case 8: memPutData(comp->mem,MEM_RAM,5,MEM_16K,pageBuf); break;
						default: btm = 0; break;
					}
				} while (btm && !feof(file));
				break;
			case 2:
				btm = 1;
				do {
					tmp = z80readblock(file,pageBuf);
					if ((tmp > 2) && (tmp < 11)) {
						memPutData(comp->mem,MEM_RAM,tmp-3,MEM_16K,pageBuf);
					} else {
						btm = 0;
					}
				} while (btm && !feof(file));
				break;
			case 3:
				btm = 1;
				do {
					tmp = z80readblock(file,pageBuf);
					if ((tmp > 2) && (tmp < 19)) {
						memPutData(comp->mem,MEM_RAM,tmp-3,MEM_16K,pageBuf);
					} else {
						btm = 0;
					}
				} while (btm && !feof(file));
				break;
			default:
				printf("Hardware mode not supported. reset\n");
				compReset(comp, RES_DEFAULT);
				err = ERR_Z80_HW;
				break;
		}
	} else {			// version 1
printf(".z80 version 1\n");
		if (hd.flag12 & 0x20) {
			printf("data is compressed\n");
			z80uncompress(file,pageBuf,0xc000);
			memPutData(comp->mem,MEM_RAM,5,MEM_16K,pageBuf);
			memPutData(comp->mem,MEM_RAM,2,MEM_16K,pageBuf + MEM_16K);
			memPutData(comp->mem,MEM_RAM,0,MEM_16K,pageBuf + MEM_32K);
		} else {
			printf("data is not compressed\n");
			fread(pageBuf, 0x4000, 1, file);
			memPutData(comp->mem,MEM_RAM,5,MEM_16K,pageBuf);
			fread(pageBuf, 0x4000, 1, file);
			memPutData(comp->mem,MEM_RAM,2,MEM_16K,pageBuf);
			fread(pageBuf, 0x4000, 1, file);
			memPutData(comp->mem,MEM_RAM,0,MEM_16K,pageBuf);
		}
	}
	tsReset(comp->ts);
	return err;
}
