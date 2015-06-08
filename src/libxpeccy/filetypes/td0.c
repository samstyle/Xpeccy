#include <stdio.h>
#include <liblhasa-1.0/lha_decoder.h>

#include "filetypes.h"

#pragma pack(push, 1)

typedef struct {
	char sign[2];	// td | TD
	unsigned char vol;
	unsigned char chk;
	unsigned char ver;	// 0x15
	unsigned char dens;	// 0x00
	unsigned char typ;
	unsigned char flag;	// b7:comment
	unsigned char dos;
	unsigned char sides;
	unsigned short crc;
} td0Head;

typedef struct {
	unsigned short crc;
	unsigned short len;
	unsigned char yr,mon,day,hr,min,sec;
} td0RemHead;

typedef struct {
	unsigned char nsec;
	unsigned char trk;
	unsigned char head;
	unsigned char crc;
} td0TrkHead;

typedef struct {
	unsigned char trk;
	unsigned char head;
	unsigned char sec;
	unsigned char secz;
	unsigned char ctrl;
	unsigned char crc;
} td0SecHead;

#pragma pack(pop)

size_t fGetData(void* buf, size_t len, void* data) {
	if (feof((FILE*)data)) return 0;
	fread(buf, len, 1, (FILE*)data);
	return len;
}

int loadTD0(Floppy* flp, const char* name) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;

	char sname[L_tmpnam + 12];
	unsigned tmpFile = 0;

	fseek(file, 0, SEEK_END);
	size_t sz = ftell(file) - sizeof(td0Head);
	rewind(file);

	int err = ERR_OK;
	td0Head hd;
	fread(&hd, sizeof(td0Head), 1, file);
	if (strncmp(hd.sign, "td", 2) && strncmp(hd.sign, "TD", 2)) {
		err = ERR_TD0_SIGN;
	} else if ((hd.dens != 0) || (hd.sides > 2)) {
		err = ERR_TD0_TYPE;
	} else {
		if (strncmp(hd.sign,"TD",2)) {		// 1 on td
			tmpFile = 1;
			LHADecoderType* decType = lha_decoder_for_name("-lh1-");
			LHADecoder* dec = lha_decoder_new(decType, fGetData, file, -1);
			tmpnam(sname);
			strcat(sname,".xpeccy.tmp");
			FILE* tfile = fopen(sname, "wb");
			unsigned char buf[1024];
			do {
				sz = lha_decoder_read(dec, buf, 1024);
				fwrite((char*)buf, sz, 1, tfile);
			} while (sz);
			fclose(file);
			fclose(tfile);
			lha_decoder_free(dec);
			file = fopen(sname, "rb");
		}
		if (hd.flag & 0x80) {
			td0RemHead rhd;
			fread(&rhd, sizeof(td0RemHead), 1, file);
#ifdef WORDS_BIG_ENDIAN
			rhd.len = swap16(rhd.len);
#endif
			fseek(file, rhd.len, SEEK_CUR);
		}
		int i, datlen, blktype, cnt, ch1, ch2;
		int idx;
		unsigned char *ptr, *sptr;
		int work = 1;
		td0TrkHead thd;
		td0SecHead shd;
		Sector sec[256];
		while (work && !feof(file)) {
			idx = 0;
			fread(&thd, sizeof(td0TrkHead), 1, file);
			if (thd.nsec == 0xff) break;
			for (i = 0; i < thd.nsec; i++) {
				if (!work) break;
				fread(&shd, sizeof(td0SecHead), 1, file);
				if (shd.sec == 0x65) {
					work = 0;
				} else if (((shd.ctrl & 0x30) == 0x00) && ((shd.secz & 0xf8) == 0)) {

					sec[idx].cyl = shd.trk;
					sec[idx].side = shd.head;
					sec[idx].sec = shd.sec;
					sec[idx].len = shd.secz;
					sec[idx].type = 0xfb;
					sec[idx].crc = -1;

					datlen = fgetwLE(file);
					if (shd.sec > thd.nsec) {
						fseek(file, datlen, SEEK_CUR);
					} else {
						blktype = fgetc(file);
						switch(blktype) {
							case 0x00:
								fread(sec[idx].dat, datlen-1, 1, file);
								idx++;
								break;
							case 0x01:
								cnt = fgetwLE(file);
								ch1 = fgetc(file);
								ch2 = fgetc(file);
								ptr = sec[idx].dat;
								while (cnt > 0) {
									*ptr++ = ch1;
									*ptr++ = ch2;
									cnt--;
								}
								idx++;
								break;
							case 0x02:
								ch2 = 0;
								ptr = sec[idx].dat;
								while (ch2 < datlen-1) {
									ch1 = fgetc(file);
									cnt = fgetc(file);
									ch2 += 2;
									if (ch1 == 0) {
										fread(ptr, cnt, 1, file);
										ptr += cnt;
										ch2 += cnt;
									} else {
										ch1 = (1 << ch1);
										sptr = ptr;
										fread(ptr, ch1, 1, file);
										ch2 += ch1;
										while (cnt > 0) {
											memcpy(ptr, sptr, ch1);
											ptr += ch1;
											cnt--;
										}
									}
								}
								idx++;
								break;
							default:
								work = 0;
								break;
						}
					}
				}
			}
			if (work) flpFormTrack(flp, (thd.trk << 1) + thd.head, sec, idx);
		}
		flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
		strcpy(flp->path,name);
		flp->insert = 1;
		flp->changed = 0;
	}

	fclose(file);
	if (tmpFile) {
		remove(sname);
	}
	return err;
}
