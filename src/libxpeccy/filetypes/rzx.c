#ifdef HAVEZLIB

#include <stdio.h>
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
	strm->avail_in = ilen;
	strm->next_in = (unsigned char*)in;
	if (inflateInit(strm) != Z_OK) return 0;
	return 1;
}

int zGetData(z_stream* strm, char* out, int olen) {
	strm->avail_out = olen;
	strm->next_out = (unsigned char*)out;
	int res = inflate(strm, Z_FINISH);
	switch (res) {
		case Z_NEED_DICT:
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			printf("inflate error %i\n",res);
			inflateEnd(strm);
			break;
	}
	return res;
	// return (olen - strm->avail_out);
}

unsigned short zGetWord(z_stream* strm) {
	char bl,bh;
	zGetData(strm, &bl, 1);
	zGetData(strm, &bh, 1);
	return (bl & 0xff) | ((bh & 0xff) << 8);
}

int zlib_uncompress(char* in, int ilen, char* out, int olen) {
	z_stream strm;
	int err;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = (unsigned char*)in;
	strm.avail_in = ilen;
	err = inflateInit(&strm);
	if (err != Z_OK) return -1;
	strm.next_out = (unsigned char*)out;
	strm.avail_out = olen;
	err = inflate(&strm, Z_NO_FLUSH);
	inflateReset(&strm);
	if (err == Z_OK) return strm.avail_in;
	if (err == Z_STREAM_END) return strm.avail_in;
	return -2;
}

// TODO : FIX IT
void rzxAddFrame(Computer* zx, int pos, int fetches, int size, char* data) {

	// printf("%i : rzxAddFrame (%i %i)\n",pos, fetches, size);
	RZXFrame frm;
	frm.fetches = fetches;
	if (size == 0xffff) {
		frm.frmSize = zx->rzx.key.frmSize;
		// frm.frmData = (unsigned char*)malloc(frm.frmSize);
		memcpy(frm.frmData, zx->rzx.key.frmData, frm.frmSize);
	} else {
		frm.frmSize = size;
		// frm.frmData = (unsigned char*)malloc(size);
		memcpy(frm.frmData, data, size);
		zx->rzx.key = frm;
	}
	zx->rzx.data[pos] = frm;

}

void rzxFree(Computer*);
void rzxLoadFrame(Computer* zx) {
	int work = 1;
	int err;
	int i;
	int pos;
	unsigned char type;
	unsigned int len;
	char* sname;
	char* obuf = NULL;
	char* ibuf = NULL;
	FILE* file = zx->rzx.file;
	FILE* ofile;
	rzxSnap shd;
	rzxFrm fhd;
	int fetches;
	int size;
	char iobuf[0x10000];
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
					sname = (char*)malloc(len - 20);
					memset(sname, 0x00, len - 20);
					fread(sname, len - 21, 1, file);
				} else {				// internal snapshot
					obuf = (char*)malloc(shd.usl);	// unpacked size
					if (shd.flag & 2) {			// packed
						ibuf = (char*)malloc(len - 17);
						fread(ibuf, len - 17, 1, file);
						err = zlib_uncompress(ibuf, len - 17, obuf, shd.usl);
						free(ibuf);
					} else {				// not packed
						fread(obuf, shd.usl, 1, file);
					}
					if (err < 0) {
						printf("Decompress error\n");
						work = 0;
						rzxStop(zx);
						break;
					}
					sname = (char*)malloc(L_tmpnam);
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
					free(obuf);
					obuf = NULL;
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
					remove(sname);		// delete temp file
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
				memset(zx->rzx.data, 0x00, fhd.fCount * sizeof(RZXFrame));
				if (fhd.flags & 2) {				// packed
					ibuf = (char*)malloc(len - 18);
					obuf = NULL;
					fread(ibuf, len - 18, 1, file);
					size = len;
					do {
						obuf = (char*)realloc(obuf,size);
						// printf("Try obuf[%i]\n",size);
						err = zlib_uncompress(ibuf, len - 18, obuf, size);
						// printf("return %i\n",err);
						if (err < 0) break;		// inflate error
						if (err != 0) size <<= 1;
					} while (err != 0);
					if (err < 0) {
						printf("unpack error\n");
						rzxStop(zx);
					} else {
						pos = 0;
						for(i = 0; i < fhd.fCount; i++) {
							fetches = obuf[pos++] & 0xff;
							fetches |= ((obuf[pos++] & 0xff) << 8);
							size = obuf[pos++] & 0xff;
							size |= ((obuf[pos++] & 0xff) << 8);
							rzxAddFrame(zx, i, fetches, size, obuf + pos);
							if (size != 0xffff)
								pos += size;
						}

					}
					free(ibuf);
					free(obuf);
				} else {					// unpacked
					for (int i = 0; i < fhd.fCount; i++) {
						fetches = fgetwLE(zx->rzx.file);
						size = fgetwLE(zx->rzx.file);
						if (size != 0xffff) {
							fread(iobuf, size, 1, file);
						}
						rzxAddFrame(zx, i, fetches, size, iobuf);
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
}

int loadRZX(Computer* zx, const char* name) {
	zx->rzx.play = 0;
	zx->rzx.file = fopen(name, "rb");
	if (!zx->rzx.file) return ERR_CANT_OPEN;
	rzxHead hd;
	fread((char*)&hd, sizeof(rzxHead), 1, zx->rzx.file);
#ifdef WORDS_BIG_ENDIAN
	hd.flags = swap32(hd.flags);
#endif
	zx->rzx.play = 1;
	zx->rzx.frame = 0;
	zx->rzx.size = 0;
//	rzxLoadFrame(zx);
	return ERR_OK;
}

#endif
