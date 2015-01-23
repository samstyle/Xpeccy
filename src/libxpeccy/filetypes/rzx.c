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

/*
void frmAddValue(RZXFrame* frm, unsigned char val) {
	if ((frm->frmSize & 0xff) == 0) {
		frm->frmData = (unsigned char*)realloc(frm->frmData, (frm->frmSize + 0x100) * sizeof(unsigned char));
	}
	frm->frmData[frm->frmSize] = val;
	frm->frmSize++;
}

void rzxAddFrame(ZXComp* zx, RZXFrame* frm) {
	RZXFrame nfrm;
	nfrm.fetches = frm->fetches;
	nfrm.frmSize = frm->frmSize;
	nfrm.frmData = (unsigned char*)malloc(frm->frmSize * sizeof(unsigned char));		// THIS IS MEMORY EATER
	memcpy(nfrm.frmData, frm->frmData, frm->frmSize * sizeof(unsigned char));
	zx->rzxData = (RZXFrame*)realloc(zx->rzxData,(zx->rzxSize + 1) * sizeof(RZXFrame));
	zx->rzxData[zx->rzxSize] = nfrm;
	zx->rzxSize++;

	eatsize += nfrm.frmSize;
}
*/

int zlib_uncompress(char* in, int ilen, char* out, int olen) {
	int ret;
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = inflateInit(&strm);
	if (ret!= Z_OK) {
		printf("Inflate init error\n");
		return 0;
	}
	strm.avail_in = ilen;
	strm.next_in = (unsigned char*)in;
	strm.avail_out = olen;
	strm.next_out = (unsigned char*)out;
	ret = inflate(&strm,Z_FINISH);
	switch (ret) {
		case Z_NEED_DICT:
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			inflateEnd(&strm);
			return 0;
	}
	inflateEnd(&strm);
	return (olen - strm.avail_out);
}

void rzxLoadFrame(ZXComp* zx) {
	int work = 1;
	unsigned char type;
	unsigned int len;
	int pos;
	char* sname = NULL;
	char* obuf = NULL;
	char* ibuf = NULL;
	FILE* file = zx->rzx.file;
	FILE* ofile;
	rzxSnap shd;
	rzxFrm fhd;
//	RZXFrame frm;
//	frm.frmSize = 0;
//	frm.frmData = NULL;
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
					sname = tmpnam(NULL);			// write to temp file
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
				if (shd.flag & 1) {
					free(sname);
				} else {
					unlink(sname);		// delete temp file
				}
				break;
			case 0x80:
				work = 0;
				fread((char*)&fhd, sizeof(rzxFrm), 1, file);
#ifdef WORDS_BIG_ENDIAN
				fhd.fCount = le32toh(fhd.fCount);
				fhd.tStart = le32toh(fhd.tStart);
				fhd.flags = le32toh(fhd.flags);
#endif
				if (fhd.flags & 1) {
					printf("Crypted rzx data\n");
					rzxStop(zx);
					break;
				}
				if (fhd.flags & 2) {
					obuf = (char*)realloc(obuf, 0x1000000);		// ~16Mb
					ibuf = (char*)realloc(ibuf, len - 18);
					fread(ibuf, len - 18, 1, file);
					len = zlib_uncompress(ibuf, len - 18, obuf, 0x1000000);
					printf("unpacked : %.X\n",len);
				} else {
					obuf = (char*)realloc(obuf, len - 18);
					fread(obuf, len - 18, 1, file);
				}
				if (len == 0) {
					printf("Decompress error\n");
					rzxStop(zx);
					break;
				}
				printf("Frames: %i\n",fhd.fCount);
				zx->rzx.size = fhd.fCount;
				zx->rzx.data = (RZXFrame*)realloc(zx->rzx.data, fhd.fCount * sizeof(RZXFrame));
				pos = 0;
				for (int i = 0; i < fhd.fCount; i++) {
					zx->rzx.data[i].fetches = (obuf[pos] & 0xff) | ((obuf[pos+1] & 0xff) << 8);
					len = (obuf[pos+2] & 0xff) | ((obuf[pos+3] & 0xff) << 8);
					pos += 4;
					if (len != 0xffff) {
						zx->rzx.data[i].frmSize = len;
						zx->rzx.data[i].frmData = (unsigned char*)malloc(len);
						memcpy(zx->rzx.data[i].frmData, &obuf[pos], len);
						pos += len;
					} else {
						len = zx->rzx.data[i - 1].frmSize;
						zx->rzx.data[i].frmSize = len;
						zx->rzx.data[i].frmData = (unsigned char*)malloc(len);
						memcpy(zx->rzx.data[i].frmData, zx->rzx.data[i - 1].frmData, len);
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
	hd.flags = le32toh(hd.flags);
#endif
	zx->rzxPlay = 1;
	rzxLoadFrame(zx);
	return ERR_OK;
}

/*
#ifdef _WIN32
	std::string tmpName = std::string(getenv("TEMP"))+"\\lain.tmp";
#else
	std::string tmpName = "/tmp/lain.tmp";
#endif
	bool btm;
	unsigned char tmp;
	int err;
	int flg,len,len2,len3;//,tstates;
	std::string tmpStr;
	char* buf = new char[16];
	char* zbuf = NULL;
	std::ofstream ofile;
	RZXFrame rzxf;
	rzxf.frmSize = 0;
	rzxf.frmData = NULL;

	file.seekg(0,std::ios_base::end);	// get filesize
	size_t fileSize = file.tellg();
	file.seekg(0);

	file.read(buf,4);
	if (std::string(buf,4) != "RZX!") return ERR_RZX_SIGN;
	file.seekg(2,std::ios_base::cur);	// version
	flg = getint(&file);			// flags
	if (flg & 1) return ERR_RZX_CRYPT;
	rzxClear(zx);
	btm = true;

	eatsize = 0;

	while (btm && ((size_t)file.tellg() < fileSize)) {
		tmp = file.get();	// block type
		len = getint(&file);	// block len;
		switch (tmp) {
			case 0x30:
				flg = getint(&file);
				file.read(buf,4);
				tmpStr = std::string(buf,3);
				tmp = 0xff;
				if ((tmpStr == "z80") || (tmpStr == "Z80")) tmp = TYP_Z80;
				if ((tmpStr == "sna") || (tmpStr == "SNA")) tmp = TYP_SNA;
				len2 = getint(&file);	// data length;
				if (flg & 1) {
					printf("External snapshot");
					file.seekg(4,std::ios_base::cur);	// checksum
					buf = (char*)realloc(buf,(len - 21) * sizeof(char));
					file.read(buf,len - 21);		// snapshot file name
					tmpStr = std::string(buf, len - 21);
				} else {
					buf = (char*)realloc(buf,sizeof(char) * len2);	// uncompressed data
					if (flg & 2) {
						zbuf = (char*)realloc(zbuf,sizeof(char) * len);	// compressed data
						file.read(zbuf,len-17);
						len2 = zlib_uncompress(zbuf,len-17,buf,len2);
					} else {
						file.read(buf,len2);
					}
					if (len2 == 0) {
						delete(buf);
						if (zbuf != NULL) delete(zbuf);
						return ERR_RZX_UNPACK;
					}
					ofile.open(tmpName.c_str(),std::ios::binary);
					ofile.write(buf,len2);
					ofile.close();
					tmpStr = tmpName;
				}
				err = ERR_CANT_OPEN;
//				printf("path = %s\n",tmpStr.c_str());
				switch (tmp) {
					case TYP_SNA:
						err = loadSNA(zx,tmpStr.c_str());
						break;
					case TYP_Z80:
						err = loadZ80(zx,tmpStr.c_str());
						break;
				}
				if (err != ERR_OK) btm = false;
				if (buf) free(buf);
				buf = NULL;
				break;
			case 0x80:
				len2 = getint(&file);	// number of frames
				file.get();		// reserved
				getint(&file);		// TStates @ beginning
				flg = getint(&file);	// flags
				if (flg & 1) return ERR_RZX_CRYPT;
				len3 = len - 18;
				if (flg & 2) {			// get data in buf, uncompress if need
					buf = (char*)realloc(buf,sizeof(char) * 0x7ffffff);
					zbuf = (char*)realloc(zbuf,sizeof(char) * len3);	// compressed data
					file.read(zbuf,len3);
					len3 = zlib_uncompress(zbuf,len3,buf,0x7ffffff);
					//printf("%i\n",len3);
				} else {
					buf = (char*)realloc(buf,sizeof(char) * len3);
					file.read(buf, len3);
				}
				if (len3 == 0) {
					free(buf);
					if (zbuf) free(zbuf);
					return ERR_RZX_UNPACK;
				}
				flg = 0;

				while (len2 > 0) {
					rzxf.fetches = (unsigned char)buf[flg] + ((unsigned char)buf[flg+1] << 8);
					flg += 2;
					len3 = (unsigned char)buf[flg] + ((unsigned char)buf[flg+1] << 8);
					flg += 2;
					if (len3 != 0xffff) {
						if (rzxf.frmData) free(rzxf.frmData);
						rzxf.frmData = NULL;
						rzxf.frmSize = 0;
						while (len3 > 0) {
							frmAddValue(&rzxf,buf[flg]);
							//rzxf.in.push_back((unsigned char)buf[flg]);
							flg++;
							len3--;
						}
					}
					rzxAddFrame(zx,&rzxf);
					len2--;
				}

				zx->rzxFrame = 0;
				zx->rzxPos = 0;
				if (zx->rzxSize != 0) {
					zx->rzxPlay = 1;
					zx->rzxFetches = zx->rzxData[0].fetches;
				} else {
					zx->rzxPlay = 0;
				}
				if (rzxf.frmData) free(rzxf.frmData);			// fuuuuuck!
				free(buf);
				buf = NULL;
				if (zbuf) free(zbuf);
				zbuf = NULL;
				btm = false;
				break;
			default:
				file.seekg(len-5,std::ios_base::cur);	// skip block
				break;
		}
	}
	tsReset(zx->ts);
	printf("Memory eated for RZX: %i\n",eatsize);
	return ERR_OK;
}
*/

#endif
