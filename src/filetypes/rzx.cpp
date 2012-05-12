#ifdef HAVEZLIB
#include "filetypes.h"
#include <stdlib.h>
//#include <string>
#include <zlib.h>

int eatsize = 0;

uint32_t getint(std::ifstream* file) {
	uint32_t res = file->get();
	res += (file->get() << 8);
	res += (file->get() << 16);
	res += (file->get() << 24);
	return res;
}

void frmAddValue(RZXFrame* frm, uint8_t val) {
	if ((frm->frmSize & 0xff) == 0) {
		frm->frmData = (uint8_t*)realloc(frm->frmData, (frm->frmSize + 0x100) * sizeof(uint8_t));
	}
	frm->frmData[frm->frmSize] = val;
	frm->frmSize++;
}

void rzxAddFrame(ZXComp* zx, RZXFrame* frm) {
	RZXFrame nfrm = *frm;
	nfrm.frmData = (uint8_t*)malloc(frm->frmSize * sizeof(uint8_t));		// THIS IS MEMORY EATER
	memcpy(nfrm.frmData, frm->frmData, frm->frmSize * sizeof(uint8_t));
	zx->rzxData = (RZXFrame*)realloc(zx->rzxData,(zx->rzxSize + 1) * sizeof(RZXFrame));
	zx->rzxData[zx->rzxSize] = nfrm;
	zx->rzxSize++;

	eatsize += nfrm.frmSize;
}

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
	strm.next_in = (uint8_t*)in;
	strm.avail_out = olen;
	strm.next_out = (uint8_t*)out;
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

int loadRZX(ZXComp* zx, const char* name) {
	std::ifstream file(name,std::ios::binary);
	if (!file.good()) return ERR_CANT_OPEN;
#ifdef WIN32
	std::string tmpName = std::string(getenv("TEMP"))+"\\lain.tmp";
#else
	std::string tmpName = "/tmp/lain.tmp";
#endif
	bool btm;
	uint8_t tmp;
	int err;
	int flg,len,len2,len3;//,tstates;
	std::string tmpStr;
	char* buf = new char[16];
	char* zbuf = NULL;
	std::ofstream ofile;
	RZXFrame rzxf;
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

	while (btm && (file.tellg() < fileSize)) {
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
					rzxf.fetches = (uint8_t)buf[flg] + ((uint8_t)buf[flg+1] << 8);
					flg += 2;
					len3 = (uint8_t)buf[flg] + ((uint8_t)buf[flg+1] << 8);
					flg += 2;
					if (len3 != 0xffff) {
						rzxf.frmSize = 0;
						if (rzxf.frmData) free(rzxf.frmData);
						rzxf.frmData = NULL;
						while (len3 > 0) {
							frmAddValue(&rzxf,buf[flg]);
							//rzxf.in.push_back((uint8_t)buf[flg]);
							flg++;
							len3--;
						}
					}
					rzxAddFrame(zx,&rzxf);
					free(rzxf.frmData);
					rzxf.frmData = NULL;
					rzxf.frmSize = 0;
					//zx->rzx.push_back(rzxf);
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
	printf("Memory eated for RZX: %i\n",eatsize);
	return ERR_OK;
}

#endif
