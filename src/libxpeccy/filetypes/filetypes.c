#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "filetypes.h"

#ifdef WORDS_BIG_ENDIAN

// swap 16/32 bit variables (LE <-> BE)
unsigned short swap16(unsigned short wr) {
	return ((wr & 0xff) << 8) | ((wr >> 8) & 0xff);
}

unsigned int swap32(unsigned int wr) {
	unsigned int res = ((wr & 0xff) << 24);
	res |= ((wr & 0xff00) << 8);
	res |= ((wr >> 8) & 0xff00);
	res |= ((wr >> 24) & 0xff);
	return res;
}

#else

unsigned short swap16(unsigned short wr) {return wr;}
unsigned int swap32(unsigned int wr) {return wr;}

#endif

// get filesize & rewind it
size_t fgetSize(FILE* file) {
	fseek(file, 0, SEEK_END);
	size_t res = ftell(file);
	rewind(file);
	return res;
}

unsigned short fgetw(FILE* file) {
	int res = fgetc(file);
	res |= (fgetc(file) << 8);
	return res & 0xffff;
}

int fgett(FILE* file) {
	int res = fgetw(file);
	res |= (fgetc(file) << 16);
	return res;
}

int fgeti(FILE* file) {
	int res = fgetw(file);
	res |= (fgetw(file) << 16);
	return res;
}

void fputw(unsigned short val, FILE* file) {
	fputc(val & 0xff, file);
	fputc((val >> 8) & 0xff, file);
}

void fputi(int val, FILE* file) {
	fputw(val & 0xffff, file);
	fputw((val >> 16) & 0xffff, file);
}

// load boot PATH to floppy FLP, if there is TRDOS disk
void loadBoot(Computer* comp, const char* path, int drv) {
	Floppy* flp = comp->dif->fdc->flop[drv & 3];
	if (diskGetType(flp) != DISK_TYPE_TRD) return;
	TRFile cat[128];
	int catSize = diskGetTRCatalog(flp, cat);
	int gotBoot = 0;
	for (int i = 0; i < catSize; i++) {
		if (strncmp((char*)cat[i].name, "boot    B", 9) == 0)
			gotBoot = 1;
	}
	if (gotBoot) return;
	loadHobeta(comp, path, drv);
	flp->changed = 0;
}

void diskClear(Floppy* flp) {
	for (int i = 0; i < 168; i++) {
		memset(flp->data[i].byte, 0, TRACKLEN);
		flpFillFields(flp, i, 0);
	}
}

// trdos

static unsigned char trd_8e0[] = {
	0x00,0x00,0x01,0x16,0x00,0xf0,0x09,0x10,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00
};

// format whole disk as 168x16x256 and init as TRDOS
void diskFormat(Floppy* flp) {
	int i;
	unsigned char buf[0x1000];
	memset(buf, 0, 0x1000);
	for (i = 1;i < 168; i++) diskFormTRDTrack(flp,i,buf);
	memcpy(buf + 0x8e0, trd_8e0, 0x20);
	diskFormTRDTrack(flp,0,buf);
}

// build a single track 16x256 (TRDOS), sector data @bpos (4K)
void diskFormTRDTrack(Floppy* flp, int tr, unsigned char* bpos) {
	Sector lst[16];
	Sector sct;
	int sc;
	unsigned char* ppos = bpos;
	sct.type = 0xfb;
	sct.crc = -1;
	sct.trk = ((tr & 0xfe) >> 1);
	sct.head = (tr & 0x01) ? 1 : 0;
	sct.sz = 1;
	for (sc = 0; sc < 16; sc++) {
		sct.sec = sc + 1;
		memcpy(sct.data, ppos, 256);
		lst[sc] = sct;
		ppos += 256;
	}
	diskFormTrack(flp,tr,lst,16);
}

// format a single track from sectors list
// <sdata> is list of <scount> sectors
// TODO: calculate GAP3 len !!!
void diskFormTrack(Floppy* flp, int tr, Sector* sdata, int scount) {
	unsigned char *ppos = flp->data[tr].byte;
	int i,ln;
	int sc;
	if (tr > 255) return;
	int dsz = 0;
	for (i = 0; i < scount; i++) {
		dsz += (128 << sdata[i].sz);
	}
	dsz = (TRACKLEN - dsz) / scount - 72;
	if (dsz < 10) return;
	memset(ppos, 0x00, 12); ppos += 12;		// 12	space
	*(ppos++) = 0xc2;				// 	track mark
	*(ppos++) = 0xc2;
	*(ppos++) = 0xc2;
	*(ppos++) = 0xfc;
	for (sc = 0; sc < scount; sc++) {
		memset(ppos, 0x4e, 10); ppos += 10;		// 10	sync (GAP1)
		memset(ppos, 0x00, 12); ppos += 12;		// 12	space
		*(ppos++) = 0xa1;				//	address mark
		*(ppos++) = 0xa1;
		*(ppos++) = 0xa1;
		*(ppos++) = 0xfe;
		*(ppos++) = sdata[sc].trk;			// 	addr field
		*(ppos++) = sdata[sc].head;
		*(ppos++) = sdata[sc].sec;
		*(ppos++) = sdata[sc].sz;
		*(ppos++) = 0xf7; *(ppos++) = 0xf7;
		memset(ppos, 0x4e, 22); ppos += 22;		// 22	sync (GAP2)
		memset(ppos, 0x00, 12); ppos += 12;		// 12	space
		*(ppos++) = 0xa1;				//	data mark
		*(ppos++) = 0xa1;
		*(ppos++) = 0xa1;
		*(ppos++) = sdata[sc].type;
		ln = (128 << (sdata[sc].sz & 3));		//	data
		memcpy(ppos, sdata[sc].data, ln); ppos += ln;
		*(ppos++) = 0xf7; *(ppos++) = 0xf7;
		memset(ppos, 0x4e, dsz); ppos += dsz;		// 60	sync (GAP3)
	}
	while ((ppos - flp->data[tr].byte) < TRACKLEN) *(ppos++) = 0x4e;		// last sync (GAP4)
	flpFillFields(flp,tr,1);
}

static unsigned char fbuf[0x100];

int diskGetType(Floppy* flp) {
	int res = -1;
	// trdos
	if (!diskGetSectorData(flp,0,15,fbuf,0x100)) {			// at least 16 sectors
		if (diskGetSectorData(flp,0,9,fbuf,0x100)) {
			if (fbuf[0xe7] == 0x10) res = DISK_TYPE_TRD;
		}
	}
	return res;
}

int diskCreateDescriptor(Floppy* flp,TRFile* dsc) {
	unsigned char files;
	unsigned short freesec;
	if (!diskGetSectorData(flp,0,9,fbuf,256)) return ERR_SHIT;
	dsc->sec = fbuf[0xe1];
	dsc->trk = fbuf[0xe2];
	files = fbuf[0xe4];
	if (files > 127) return ERR_MANYFILES;
	files++;
	fbuf[0xe4] = files;
	freesec = fbuf[0xe5] + (fbuf[0xe6] << 8);
	if (freesec < dsc->slen) return ERR_NOSPACE;
	freesec -= dsc->slen;
	fbuf[0xe5] = freesec & 0xff;
	fbuf[0xe6] = ((freesec & 0xff00) >> 8);
	fbuf[0xe1] += (dsc->slen & 0x0f);
	fbuf[0xe2] += ((dsc->slen & 0xf0) >> 4);
	if (fbuf[0xe1] > 0x0f) {
		fbuf[0xe1] -= 0x10;
		fbuf[0xe2]++;
	}
	diskPutSectorData(flp,0,9,fbuf,256);
	files--;
	freesec = ((files & 0xf0) >> 4)+1;
	if (!diskGetSectorData(flp,0,freesec,fbuf,256)) return ERR_SHIT;
	memcpy(fbuf + ((files & 0x0f) << 4), (char*)dsc, 16);
	diskPutSectorData(flp,0,freesec,fbuf,256);
	flp->changed = 1;
	return ERR_OK;
}

int diskCreateFile(Floppy* flp, TRFile dsc, unsigned char* data, int len) {
	if (len > 0xff00) return ERR_SIZE;
	dsc.slen = ((len >> 8) & 0xff) + ((len & 0xff) ? 1 : 0);
	int err = diskCreateDescriptor(flp, &dsc);
	if (err != ERR_OK) return err;
	for (int i = 0; i < dsc.slen; i++) {
		diskPutSectorData(flp, dsc.trk, dsc.sec + 1, &data[i << 8], 256);
		dsc.sec++;
		if (dsc.sec > 15) {
			dsc.trk++;
			dsc.sec -= 16;
		}
	}
	return ERR_OK;
}

TRFile diskGetCatalogEntry(Floppy* flp, int num) {
	TRFile res;
	memset(&res, 0x00, sizeof(TRFile));
	TRFile cat[128];
	int cnt = diskGetTRCatalog(flp, cat);
	int i = 0;
	while (i < cnt) {
		switch(cat[i].name[0]) {
			case 0x00: i = cnt; break;	// end
			case 0x01: break;		// skip
			default: if (num == 0) {
					res = cat[i];
					i = cnt;
				}
				num--;
				break;
		}
		i++;
	}
/*
	if (num > 127) return res;
	sec = ((num & 0xf0) >> 4);	// sector
	pos = ((num & 0x0f) << 4);	// file number inside sector
	if (!diskGetSectorData(flp,0,sec + 1,fbuf,256)) return res;
	memcpy((char*)&res,fbuf + pos,16);
*/
	return res;
}

TRFile diskMakeDescriptor(const char* name, char ext, int start, int len) {
	TRFile dsc;
	memset(dsc.name,' ',8);
	memcpy(dsc.name,name,(strlen(name) > 8) ? 8 : strlen(name));
	dsc.ext = ext;
	dsc.hst = (start >> 8) & 0xff;
	dsc.lst = start & 0xff;
	dsc.hlen = (len >> 8) & 0xff;
	dsc.llen = len & 0xff;
	dsc.slen = dsc.hlen + (dsc.llen ? 1 : 0);
	return dsc;
}

int diskGetTRCatalog(Floppy *flp, TRFile *dst) {
	int cnt = 0;
	if (diskGetType(flp) == DISK_TYPE_TRD) {
		int sc;
		int fc;
		unsigned char* ptr;
		unsigned char* dpt = (unsigned char*)dst;
		for (sc = 1; sc < 9; sc++) {
			if (diskGetSectorData(flp,0,sc,fbuf,256)) {
				ptr = fbuf;
				for (fc = 0; fc < 16; fc++) {
					if (*ptr == 0) break;
					memcpy(dpt,ptr,16);
					dpt += 16;
					ptr += 16;
					cnt++;
				}
				if (fc < 16) break;
			} else {
				break;
			}
		}
	}
	return cnt;
}

unsigned char* diskGetSectorDataPtr(Floppy* flp, unsigned char tr, unsigned char sc) {
#if 1
	int pos = flp->data[tr].map[sc];	// input nr 1+, tab nr 0+
	if (pos == 0) return NULL;
	return flp->data[tr].byte + pos;
#else
	int tpos = 0;
	int fnd;
	if (!flp->insert) return NULL;
	while (1) {
		while (flp->data[tr].field[tpos] != 1) {
			if (++tpos >= TRACKLEN) return NULL;
		}
		fnd = (flp->data[tr].byte[tpos+2] == sc) ? 1 : 0;
		tpos += 6;
		while ((flp->data[tr].field[tpos] != 2) && (flp->data[tr].field[tpos] != 3)) {
			if (++tpos >= TRACKLEN) return NULL;
		}
		if (fnd) {
			// printf("%i\n",tpos);	// 3190
			return flp->data[tr].byte + tpos;
		}
		tpos += 0x102;
		if (tpos >= TRACKLEN) return NULL;
	}
#endif
}

int diskPutSectorData(Floppy* flp, unsigned char tr,unsigned char sc,unsigned char* buf,int len) {
	unsigned short crc;
	unsigned char* ptr = diskGetSectorDataPtr(flp,tr,sc);
	if (ptr == NULL) return 0;
	memcpy(ptr,buf,len);
	crc = getCrc(ptr - 4,len + 4);
	*(ptr + len) = ((crc & 0xff00) >> 8);
	*(ptr + len + 1) = (crc & 0x00ff);
	return 1;
}

int diskGetSectorData(Floppy* flp, unsigned char tr,unsigned char sc,unsigned char* buf,int len) {
	unsigned char* ptr = diskGetSectorDataPtr(flp,tr,sc);
	if (ptr == NULL) return 0;
	memcpy(buf, ptr, len);
	return 1;
}

int diskGetSectorsData(Floppy* flp, unsigned char tr, unsigned char sc, unsigned char* ptr, int sl) {
	while (sl > 0) {
		if (!diskGetSectorData(flp,tr,sc,ptr,256)) return 0;
		ptr += 256;
		sc++;
		if (sc > 16) {
			sc = 1;
			tr++;
		}
		sl--;
	}
	return 1;
}
