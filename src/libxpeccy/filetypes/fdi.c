#include "filetypes.h"

#pragma pack (push, 1)

typedef struct {
	char sign[3];
	char wp;
	unsigned short cyls;
	unsigned short heads;
	unsigned short dInfo;
	unsigned short dData;
	unsigned short xSize;
} fdiHead;

typedef struct {
	unsigned int dData;
	unsigned short res;		// reserved
	unsigned char secCount;
} fdiTHead;

typedef struct {
	unsigned char cyl;
	unsigned char head;
	unsigned char sec;
	unsigned char len;
	unsigned char flag;
	unsigned short dData;
} fdiSHead;

#pragma pack (pop)

int loadFDI(Computer* comp, const char* name, int drv) {
	Floppy* flp = comp->dif->fdc->flop[drv & 3];
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;

	int err = ERR_OK;
	fdiHead hd;
	fdiTHead thd;
	fdiSHead shd;
	Sector trkImg[256];
	size_t pos;
//	int i;
//	for (i = 0; i < 256; i++) trkImg[i].data = NULL;
	fread((char*)&hd, sizeof(fdiHead), 1, file);
#ifdef WORDS_BIG_ENDIAN
	hd.cyls = swap16(hd.cyls);
	hd.heads = swap16(hd.heads);
	hd.dInfo = swap16(hd.dInfo);
	hd.dData = swap16(hd.dData);
	hd.xSize = swap16(hd.xSize);
#endif
	if (strncmp((char*)hd.sign, "FDI", 3) != 0) {
		err = ERR_FDI_SIGN;
	} else if ((hd.heads < 1) || (hd.heads > 2)) {
		err = ERR_FDI_HEAD;
	} else {
		fseek(file, hd.xSize, SEEK_CUR);
		int trk, head, sec;
		for (trk = 0; trk < hd.cyls; trk++) {
			for (head = 0; head < hd.heads; head++) {
				fread((char*)&thd, sizeof(fdiTHead), 1, file);
#ifdef WORDS_BIG_ENDIAN
				thd.dData = swap32(thd.dData);
#endif
				for (sec = 0; sec < thd.secCount; sec++) {
					fread((char*)&shd, sizeof(fdiSHead), 1, file);
#ifdef WORDS_BIG_ENDIAN
					shd.dData = swap16(shd.dData);
#endif
					trkImg[sec].trk = shd.cyl;
					trkImg[sec].head = shd.head;
					trkImg[sec].sec = shd.sec;
					trkImg[sec].sz = shd.len;
					trkImg[sec].type = (shd.flag & 0x80) ? 0xf8 : 0xfb;
					shd.len = shd.len & 3;
					pos = ftell(file);
					fseek(file, hd.dData + thd.dData + shd.dData, SEEK_SET);
					fread((char*)trkImg[sec].data, (128 << (shd.len & 3)), 1, file);
					fseek(file, pos, SEEK_SET);
				}
				diskFormTrack(flp, (trk << 1) + head, trkImg, thd.secCount);
			}
		}
		flp->protect = hd.wp ? 1 : 0;
		flp_insert(flp, name);
	}
	//for (i = 0; i < 256; i++) if (trkImg[i].data) free(trkImg[i].data);
	fclose(file);
	return err;
}
