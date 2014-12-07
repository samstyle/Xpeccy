#include "filetypes.h"

#pragma pack (1)

struct fdiHead {
	char sign[3];
	char wp;
	unsigned short cyls;
	unsigned short heads;
	unsigned short dInfo;
	unsigned short dData;
	unsigned short xSize;
};

struct fdiTHead {
	unsigned int dData;
	unsigned short res;		// reserved
	unsigned char secCount;
};

struct fdiSHead {
	unsigned char cyl;
	unsigned char head;
	unsigned char sec;
	unsigned char len;
	unsigned char flag;
	unsigned short dData;
};

#pragma pack ()

int loadFDI(Floppy* flp,const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;

	fdiHead hd;
	fdiTHead thd;
	fdiSHead shd;
	Sector trkImg[256];
	size_t pos;
	file.read((char*)&hd, sizeof(fdiHead));
#ifdef WORDS_BIG_ENDIAN
	hd.cyls = le16toh(hd.cyls);
	hd.heads = le16toh(hd.heads);
	hd.dInfo = le16toh(hd.dInfo);
	hd.dData = le16toh(hd.dData);
	hd.xSize = le16toh(hd.xSize);
#endif
	if (strncmp((char*)hd.sign, "FDI", 3) != 0) return ERR_FDI_SIGN;
	if ((hd.heads == 0) || (hd.heads > 2)) return ERR_FDI_HEAD;		// too many heads (1 or 2 allowed)
	file.seekg(14 + hd.xSize, std::ios_base::beg);
	int trk, head, sec;
	for (trk = 0; trk < hd.cyls; trk++) {
		for (head = 0; head < hd.heads; head++) {
			file.read((char*)&thd,sizeof(fdiTHead));
#ifdef WORDS_BIG_ENDIAN
			thd.dData = le32toh(thd.dData);
#endif
			for (sec = 0; sec < thd.secCount; sec++) {
				file.read((char*)&shd, sizeof(fdiSHead));
#ifdef WORDS_BIG_ENDIAN
				shd.dData = le16toh(shd.dData);
#endif
				trkImg[sec].cyl = shd.cyl;
				trkImg[sec].side = shd.head;
				trkImg[sec].sec = shd.sec;
				trkImg[sec].len = shd.len;
				trkImg[sec].type = (shd.flag & 0x80) ? 0xf8 : 0xfb;
				trkImg[sec].data = new unsigned char[4096];
				shd.len = shd.len & 3;
				pos = file.tellg();
				file.seekg(hd.dData + thd.dData + shd.dData);
				file.read((char*)trkImg[sec].data, (128 << shd.len));
				file.seekg(pos,std::ios_base::beg);
			}
			flpFormTrack(flp, (trk << 1) + head, trkImg, thd.secCount);
		}
	}
	flp->protect = hd.wp ? 1 : 0;
	flp->insert = 1;
	loadBoot(flp);
	flp->changed = 0;
	flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
	strcpy(flp->path,name);
	return ERR_OK;
}
