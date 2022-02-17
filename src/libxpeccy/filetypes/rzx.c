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

// new

static char* msgRzxStop = " RZX playback end ";

void rzxGetFrame(Computer* comp) {
	int type;
	int len;
	int work;
	int size;
	size_t pos;
	if (!comp->rzx.file) {
		rzxStop(comp);
	} else {
		if (comp->rzx.fCount > 0) {
			comp->rzx.frm.fetches = fgetw(comp->rzx.file);
			size = fgetw(comp->rzx.file);
			if (size != 0xffff) {
				comp->rzx.frm.size = size;
				if (size > 0)
					fread(comp->rzx.frm.data, size, 1, comp->rzx.file);
			}
			comp->rzx.frm.pos = 0;
		} else {
			work = 1;
			while (work) {
				type = fgetc(comp->rzx.file) & 0xff;
				len = fgeti(comp->rzx.file);
				pos = ftell(comp->rzx.file);
				switch (type) {
					case 0x80:					// IN block
						comp->rzx.fCount = fgeti(comp->rzx.file);		// +0 frame count
						size = fgeti(comp->rzx.file);				// +4 start Tstate
						// vid_set_ray(comp->vid, size << 1);
						comp->rzx.frm.fetches = fgetw(comp->rzx.file);		// +8.. frames		+0 fetches
						size = fgetw(comp->rzx.file);				//			+2 size
						if (size != 0xffff) {
							comp->rzx.frm.size = size;
							fread(comp->rzx.frm.data, comp->rzx.frm.size, 1, comp->rzx.file);// +4 data
						}
						comp->rzx.frm.pos = 0;
						work = 0;
						break;
					case 0x30:					// TODO: snapshot
						type = fgetc(comp->rzx.file);
						switch(type) {
							case 0x00:
								loadSNA_f(comp, comp->rzx.file, len - 1);
								fseek(comp->rzx.file, pos + len, SEEK_SET);
								break;
							case 0x01:
								if (loadZ80_f(comp, comp->rzx.file) == ERR_OK) {	// bad loading?
									fseek(comp->rzx.file, pos + len, SEEK_SET);
								} else {
									rzxStop(comp);
									work = 0;
								}
								break;
							default:
								printf("unknown snapshot type\n");
								rzxStop(comp);
								work = 0;
								// fseek(comp->rzx.file, len - 1, SEEK_CUR);
								break;
						}
						break;
					case 0xff:					// EOF
						rzxStop(comp);
						comp->msg = msgRzxStop;
						work = 0;
						break;
					default:
						fseek(comp->rzx.file, len, SEEK_CUR);	// skip (len) bytes
						break;

				}
			}
		}
	}
}

int inflateToFile(char* buf, int len, FILE* file) {
	int err = ERR_OK;
	z_stream strm;
	char* obuf = malloc(0x4000);
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = (unsigned char*)buf;
	strm.avail_in = len;
	if (inflateInit(&strm) != Z_OK) {
		err = ERR_RZX_UNPACK;
	} else {
		do {
			strm.next_out = (unsigned char*)obuf;
			strm.avail_out = 0x4000;
			err = inflate(&strm, Z_NO_FLUSH);
			if ((err == Z_OK) || (err == Z_STREAM_END)) {
				fwrite(obuf, 0x4000 - strm.avail_out, 1, file);
			}
		} while (err == Z_OK);
		inflateEnd(&strm);
		err = (err == Z_STREAM_END) ? ERR_OK : ERR_RZX_UNPACK;
	}
	free(obuf);
	return err;
}

int rzxGetSnapType(char* ext) {
	int res = 0xff;
	if (!strncmp(ext, "sna", 3) || !strncmp(ext, "SNA", 3)) {
		res = 0;
	} else if (!strncmp(ext, "z80", 3) || !strncmp(ext, "Z80", 3)) {
		res = 1;
	}
	return res;
}

int loadRZX(Computer* comp, const char* name, int drv) {
	int err = ERR_OK;
	comp->rzx.play = 0;
	comp->rzx.fTotal = 0;
	FILE* file = fopen(name, "rb");
	int type;
	int len;
	rzxSnap shd;
	rzxFrm fhd;
	FILE* sfile;
	char* buf = NULL;
	char* obuf = malloc(0x4000);
	if (!file) {
		err = ERR_CANT_OPEN;
	} else {
		rzxHead hd;
		fread(&hd, sizeof(rzxHead), 1, file);
		hd.flags = swap32(hd.flags);
		if (strncmp(hd.sign,"RZX!",4)) {
			err = ERR_RZX_SIGN;
		} else {
			printf("RZX ver %i.%i\n",hd.major,hd.minor);
			comp->rzx.file = tmpfile(); // fopen("/home/sam/rzx.tmp","w+b");
			if (!comp->rzx.file) {
				err = ERR_CANT_OPEN;
			} else {
				err = ERR_OK;
				while (!feof(file) && (err == ERR_OK)) {
					type = fgetc(file) & 0xff;
					len = fgeti(file);
					if (feof(file)) break;
					switch (type) {
						case 0x30:
							fread(&shd, sizeof(rzxSnap), 1, file);
							shd.flag = swap32(shd.flag);
							shd.usl = swap32(shd.usl);
							if (shd.flag & 1) {		// external
								fgeti(file);	// checksum
								buf = realloc(buf, len - 20);
								memset(buf, 0x00, len - 20);
								fread(buf, len - 21, 1, file);
								sfile = fopen(buf, "rb");
								if (sfile) {
									len = fgetSize(sfile);
									fputc(0x30, comp->rzx.file);
									fputi(len + 1, comp->rzx.file);
									fputc(rzxGetSnapType(shd.ext), comp->rzx.file);
									while (len > 0) {
										fread(obuf, 0x4000, 1, sfile);
										fwrite(obuf, (len > 0x4000) ? 0x4000 : len, 1, comp->rzx.file);
										len -= 0x4000;
									}
									fclose(sfile);
								} else {
									err = ERR_CANT_OPEN;
								}
							} else if (shd.flag & 2) {	// compressed
								fputc(0x30, comp->rzx.file);
								fputi(shd.usl + 1, comp->rzx.file);
								fputc(rzxGetSnapType(shd.ext), comp->rzx.file);
								buf = realloc(buf, len - 17);
								fread(buf, len - 17, 1, file);
								err = inflateToFile(buf, len - 17, comp->rzx.file);
							} else {			// not compressed
								buf = realloc(buf, shd.usl);
								fread(buf, shd.usl, 1, file);
								fputc(0x30, comp->rzx.file);
								fputi(shd.usl + 1, comp->rzx.file);
								fputc(rzxGetSnapType(shd.ext), comp->rzx.file);
								fwrite(buf, shd.usl, 1, comp->rzx.file);
							}
							break;
						case 0x80:
							//fread(&fhd, sizeof(rzxFrm), 1, file);
							//fhd.fCount = swap32(fhd.fCount);
							//fhd.tStart = swap32(fhd.tStart);
							//fhd.flags = swap32(fhd.flags);
							fhd.fCount = fgeti(file);	// +0 frames in block
							fhd.byte09 = fgetc(file);	// +4 skip 1 byte
							fhd.tStart = fgeti(file);	// +5 T state @ start
							fhd.flags = fgeti(file);	// +9 flags
							comp->rzx.fTotal += fhd.fCount;
							fputc(0x80, comp->rzx.file);
							fputi(0, comp->rzx.file);		// ??? len
							fputi(fhd.fCount, comp->rzx.file);
							fputi(fhd.tStart, comp->rzx.file);
							// fputw(fhd.tStart, comp->rzx.file);
							if (fhd.flags & 1) {			// crypted
								err = ERR_RZX_CRYPT;
							} else if (fhd.flags & 2) {		// packed
								buf = realloc(buf, len - 18);
								fread(buf, len - 18, 1, file);
								err = inflateToFile(buf, len - 18, comp->rzx.file);
							} else {				// raw
								buf = realloc(buf, len - 18);
								fread(buf, len - 18, 1, file);
								fwrite(buf, len - 18, 1, comp->rzx.file);
							}
							break;
						default:
							fseek(file, len - 5, SEEK_CUR);
							break;
					}
				}
				if (err == ERR_OK) {
					fputc(0xff, comp->rzx.file);
					rewind(comp->rzx.file);
					comp->rzx.start = 1;
					comp->rzx.play = 0;
				} else {
					rzxStop(comp);
				}
			}
		}
		fclose(file);
	}
	free(buf);
	free(obuf);
	return err;
}

#endif
