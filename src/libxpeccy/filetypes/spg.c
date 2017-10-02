#include "filetypes.h"
#include "unpackers/unpackers.h"

#include <stdio.h>

#pragma pack (push, 1)

typedef struct {
	char astr[32];
	char sign[12];
	unsigned char fmt;	// 2c : format
	unsigned char day;	// 2d : day
	unsigned char month;	// 2e : month
	unsigned char year;	// 2f : year
	unsigned char pcl,pch;	// 30 : pc
	unsigned char spl,sph;	// 32 : sp
	unsigned char page3;	// 34 : page
	unsigned char flag35;	// 35 : flag35
	unsigned char pal,pah;	// 36 : pager address
	unsigned char ral,rah;	// 38 : resident address
	unsigned char bcl,bch;	// 3A : block count
	unsigned char sec;	// 3C : sec
	unsigned char min;	// 3D : min
	unsigned char hour;	// 3E : hour
	char res3F[17];		// 3F : reserved
	char creator[32];
	char res70[144];	// 70 : reserved
} spgHead;

#pragma pack (pop)

int loadSPG(Computer* comp, const char* name) {
	FILE* file = fopen(name,"rb");
	if (!file) return ERR_CANT_OPEN;

	spgHead hd;
	unsigned char blkInfo[768];
	unsigned char inbuf[0x4000];
	unsigned char outbuf[0xffff];
	fread((char*)&hd, sizeof(spgHead), 1, file);
	fread((char*)blkInfo, 768, 1, file);

	// printf("spg ver %i.%i\n",(hd.fmt & 0xf0) >> 4, hd.fmt & 0x0f);

	int blkCount = hd.bcl | (hd.bch << 8);
	int idx = 0;
	int sze;
	unsigned char pg;
	int addr;
	for (int i = 0; i < blkCount; i++) {
		addr = (blkInfo[idx] & 0x1f) << 9;		// addr in page (00:0000 .. 1F:3E00)
		sze = ((blkInfo[idx+1] & 0x1f) + 1) << 9;	// block size (00:512 .. 1f:16K
		pg = blkInfo[idx+2];

		fread((char*)inbuf, sze, 1, file);
		switch (blkInfo[idx + 1] & 0xc0) {
			case 0x00:
				memcpy(comp->mem->ramData + (pg << 14) + addr, inbuf, sze);
				break;
			case 0x40:
				sze = demegalz(inbuf, outbuf);		// it works?
				memcpy(comp->mem->ramData + (pg << 14) + addr, outbuf, sze);
				break;
			case 0x80:
				sze = dehrust(inbuf, outbuf);		// no 'last 6 bytes'
				memcpy(comp->mem->ramData + (pg << 14) + addr, outbuf, sze);
				break;
			default:
				printf("(%.2X,%.4X,%.4X) unknown compression\n",pg,addr,sze);
				break;
		}

		if (blkInfo[idx] & 0x80) break;			// last block flag
		idx += 3;
	}
	compReset(comp, RES_DEFAULT);
	comp->cpu->pc = (hd.pch << 8) | hd.pcl;
	comp->cpu->sp = (hd.sph << 8) | hd.spl;
	switch (hd.flag35 & 0x03) {
		case 0: compSetTurbo(comp, 1); break;
		case 1: compSetTurbo(comp, 2); break;
		default: compSetTurbo(comp, 3); break;
	}
	comp->cpu->iff1 = (hd.flag35 & 0x04) ? 1 : 0;	// int enabled/disabled
	comp->cpu->inten = Z80_NMI | (comp->cpu->iff1 ? Z80_INT : 0);
	comp->cpu->imode = 1;				// im 1
	comp->cpu->i = 0x3f;				// i = 3F
	comp->dos = 0;					// basic 48 in bank0
	comp->rom = 1;
	comp->cpm = 0;
	comp->p7FFD = 0x10;
	comp->pEFF7 = 0x00;
	comp->prt2 = 0x00;
	comp->hw->mapMem(comp);
	memSetBank(comp->mem,MEM_BANK1,MEM_RAM,5,NULL,NULL,NULL);
	memSetBank(comp->mem,MEM_BANK2,MEM_RAM,2,NULL,NULL,NULL);
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,hd.page3,NULL,NULL,NULL);
	comp->vid->tsconf.tconfig = 0;
	comp->vid->nogfx = 0;
	tsReset(comp->ts);
	fclose(file);
	return ERR_OK;
}
