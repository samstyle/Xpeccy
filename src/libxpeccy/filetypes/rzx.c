#ifdef HAVEZLIB

#include <stdlib.h>
#include <zlib.h>

#include "filetypes.h"

#pragma pack (push, 1)

typedef struct {
	char sign[4];
	unsigned char major;
	unsigned char minor;
	int flags;
} rzxHead;

typedef struct {
	int flag;
	char ext[4];
	int usl;
} rzxSnap;

typedef struct {
	int fCount;
	char byte09;
	int tStart;
	int flags;
} rzxFrm;

#pragma pack (pop)

int zInit(z_stream* strm, char* in, int ilen) {
	strm->zalloc = Z_NULL;
	strm->zfree = Z_NULL;
	strm->opaque = Z_NULL;
	if (inflateInit(strm) != Z_OK) return 0;
	strm->avail_in = ilen;
	strm->next_in = (unsigned char*)in;
	return 1;
}

int zGetData(z_stream* strm, char* out, int olen) {
	strm->avail_out = olen;
	strm->next_out = (unsigned char*)out;
	int res = inflate(strm,Z_FINISH);
	switch (res) {
		case Z_NEED_DICT:
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			inflateEnd(strm);
			return 0;
	}
	return (olen - strm->avail_out);
}

unsigned short zGetWord(z_stream* strm) {
	char buf[2];
	zGetData(strm, buf, 2);
	return (buf[0] & 0xff) | ((buf[1] & 0xff) << 8);
}

int zlib_uncompress(char* in, int ilen, char* out, int olen) {
	z_stream strm;
	if (!zInit(&strm, in, ilen)) return 0;
	int res = zGetData(&strm, out, olen);
	inflateEnd(&strm);
	return res;
}

void rzxAddFrame(ZXComp* zx, RZXFrame frm, int pos) {
	if (frm.frmSize == 0xffff) {
		frm.frmSize = zx->rzx.data[pos - 1].frmSize;
		frm.frmData = (unsigned char*)malloc(frm.frmSize);
		memcpy(frm.frmData, zx->rzx.data[pos - 1].frmData, frm.frmSize);
	}
	zx->rzx.data[pos] = frm;
}

void rzxFree(ZXComp*);
void rzxLoadFrame(ZXComp* zx) {
	int work = 1;
	unsigned char type;
	unsigned int len;
	char* sname = NULL;
	char* obuf = NULL;
	char* ibuf = NULL;
	FILE* file = zx->rzx.file;
	FILE* ofile;
	rzxSnap shd;
	rzxFrm fhd;
	z_stream strm;
	RZXFrame frm;
	frm.frmData = NULL;
	while (work) {
		type = fgetc(file);
		if (feof(zx->rzx.file)) {
			printf("eof\n");
			rzxStop(zx);
			break;
		}
		len = freadLen(file, 4);
		switch (type) {
			case 0x30:
				fread((char*)&shd, sizeof(rzxSnap), 1, file);
				if (shd.flag & 1) {			// external snapshot file
					fseek(file, 4, SEEK_CUR);
					sname = (char*)realloc(sname, len - 20);
					memset(sname, 0x00, len - 20);
					fread(sname, len - 21, 1, file);
				} else {				// internal snapshot
					obuf = (char*)realloc(obuf, shd.usl);	// unpacked size
					if (shd.flag & 2) {			// packed
						ibuf = (char*)realloc(ibuf, len - 17);
						fread(ibuf, len - 17, 1, file);
						len = zlib_uncompress(ibuf, len - 17, obuf, shd.usl);
					} else {				// not packed
						fread(obuf, shd.usl, 1, file);
					}
					if (len == 0) {
						printf("Decompress error\n");
						work = 0;
						rzxStop(zx);
						break;
					}
					sname = (char*)realloc(sname, L_tmpnam);
					tmpnam(sname);
					strcat(sname, ".xpeccy.tmp");
					printf("%s\n",sname);
					ofile = fopen(sname, "wb");
					if (!ofile) {
						work = 0;
						rzxStop(zx);
						break;
					}
					fwrite(obuf, shd.usl, 1, ofile);
					fclose(ofile);
				}
				if ((strncmp(shd.ext, "sna", 3) && strncmp(shd.ext, "SNA", 3)) == 0) {
					if (loadSNA(zx, sname) != ERR_OK) {
						work = 0;
						rzxStop(zx);
					}
				} else if ((strncmp(shd.ext, "z80", 3) && strncmp(shd.ext, "Z80", 3)) == 0) {
					if (loadZ80(zx, sname) != ERR_OK) {
						work = 0;
						rzxStop(zx);
					}
				}
				if (~shd.flag & 1) {
					unlink(sname);		// delete temp file
				}
				free(sname);
				break;
			case 0x80:
				work = 0;
				fread((char*)&fhd, sizeof(rzxFrm), 1, file);
#ifdef WORDS_BIG_ENDIAN
				fhd.fCount = swap32(fhd.fCount);
				fhd.tStart = swap32(fhd.tStart);
				fhd.flags = swap32(fhd.flags);
#endif
				if (fhd.flags & 1) {
					printf("Crypted rzx data\n");
					rzxStop(zx);
					break;
				}

				rzxFree(zx);
				zx->rzx.size = fhd.fCount;
				zx->rzx.data = (RZXFrame*)malloc(fhd.fCount * sizeof(RZXFrame));
				if (fhd.flags & 2) {				// packed
					ibuf = (char*)realloc(ibuf, len - 18);
					fread(ibuf, len - 18, 1, file);
					zInit(&strm, ibuf, len - 18);
					for (int i = 0; i < fhd.fCount; i++) {
						frm.fetches = zGetWord(&strm);
						frm.frmSize = zGetWord(&strm);
						if (frm.frmSize != 0xffff) {
							frm.frmData = (unsigned char*)malloc(frm.frmSize);
							zGetData(&strm, (char*)frm.frmData, frm.frmSize);
						}
						rzxAddFrame(zx, frm, i);
					}
					inflateEnd(&strm);
				} else {					// unpacked
					for (int i = 0; i < fhd.fCount; i++) {
						frm.fetches = fgetwLE(zx->rzx.file);
						frm.frmSize = fgetwLE(zx->rzx.file);
						if (frm.frmSize != 0xffff) {
							frm.frmData = (unsigned char*)malloc(frm.frmSize);
							fread(frm.frmData, frm.frmSize, 1, zx->rzx.file);
						}
						rzxAddFrame(zx, frm, i);
					}
				}
				zx->rzx.fetches = zx->rzx.data[0].fetches - fhd.tStart;
				zx->rzx.frame = 0;
				zx->rzx.pos = 0;
				break;
			default:
				fseek(zx->rzx.file, len - 5, SEEK_CUR);
				break;
		}
	}
	if (ibuf) free(ibuf);
	if (obuf) free(obuf);
}

int loadRZX(ZXComp* zx, const char* name) {
	zx->rzxPlay = 0;
	zx->rzx.file = fopen(name, "rb");
	if (!zx->rzx.file) return ERR_CANT_OPEN;
	rzxHead hd;
	fread((char*)&hd, sizeof(rzxHead), 1, zx->rzx.file);
#ifdef WORDS_BIG_ENDIAN
	hd.flags = swap32(hd.flags);
#endif
	zx->rzxPlay = 1;
	rzxLoadFrame(zx);
	return ERR_OK;
}

#endif
