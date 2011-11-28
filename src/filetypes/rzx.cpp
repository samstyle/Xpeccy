#ifdef HAVEZLIB
#include "filetypes.h"
#include <string>
#include <zlib.h>

uint32_t getint(std::ifstream* file) {
	uint32_t res = file->get();
	res += (file->get() << 8);
	res += (file->get() << 16);
	res += (file->get() << 24);
	return res;
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
	
	bool btm;
	uint8_t tmp;
	int flg,len,len2;
	std::string tmpStr;
	char* buf = new char[16];
	char* zbuf = NULL;
	std::ofstream ofile;

	file.seekg(0,std::ios_base::end);	// get filesize
	size_t fileSize = file.tellg();
	file.seekg(0);

	file.read(buf,4);
	if (std::string(buf,4) != "RZX!") return ERR_RZX_SIGN;
	file.seekg(2,std::ios_base::cur);	// version
	flg = getint(&file);			// flags
	if (flg & 1) return ERR_RZX_CRYPT;	
	zx->mem->rzx.clear();
	btm = true;
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
					delete(buf);
					buf = new char[len - 21];
					file.read(buf,len - 21);		// snapshot file name
					tmpStr = std::string(buf, len - 21);
					delete(buf);
					buf = NULL;
				} else {
					delete(buf);
					buf = new char[len2];	// uncompressed data
					if (flg & 2) {
						if (zbuf) delete(zbuf);
						zbuf = new char[len];	// compressed data
						file.read(zbuf,len-17);
						len2 = zlib_uncompress(zbuf,len-17,buf,len2);
					} else {
						file.read(buf,len2);
					}
					if (len2 == 0) return ERR_RZX_UNPACK;
					ofile.open("/tmp/lain.tmp");
					ofile.write(buf,len2);
					ofile.close();
					tmpStr = "/tmp/lain.tmp";
				}
				switch (tmp) {
					case TYP_SNA:
						loadSNA(zx,tmpStr.c_str());
						break;
					case TYP_Z80:
						loadZ80(zx,tmpStr.c_str());
						break;
				}
				btm = false;		// stop after 1st snapshot; TODO: multiple snapshots loading
				break;
			default:
				file.seekg(len-5,std::ios_base::cur);	// skip block
				break;
		}
	}
	return ERR_OK;
}

#endif
