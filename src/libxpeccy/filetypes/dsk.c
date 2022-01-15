#include "filetypes.h"

// +0x00 signature (23)
// +0x17 DiskInfo\r\n
// +0x22 disk info block

#pragma pack (push, 1)

typedef struct {
	char byte0d0f[4];	// unused
	unsigned char track;
	unsigned char side;
	unsigned char dataRate;	// unknown, SD, HD, ED
	unsigned char recMode;	// unknown, FM, MFM
	unsigned char secSize;
	unsigned char secCount;
	unsigned char gap3Size;
	unsigned char filler;
	char sectorInfo[0x100 - 0x18];	// max 29 (x 8 bytes)
} TrackInfBlock;

typedef struct {
	unsigned char track;
	unsigned char side;
	unsigned char sector;
	unsigned char size;
	unsigned char sr1;
	unsigned char sr2;
	unsigned char bslo;
	unsigned char bshi;
} SectorInfBlock;	// 8 bytes

#pragma pack (pop)

void fgetLine(FILE* file, char* buf, int size, int term) {
	int ch;
	do {
		ch = fgetc(file);
		if (!feof(file)) {
			*(buf++) = ch;
		} else {
			*buf = 0x00;
		}
		size--;
	} while (!feof(file) && (size > 0) && (ch != term));
}

//static const char edsksgn[] =	"EXTENDED CPC DSK File\r\n";
//static const char dsksgn[] =	"MV - CPCEMU Disk-File\r\n";
static const char edsksgn[] =	"EXTENDED";
static const char dsksgn[] =	"MV - CPC";

int loadDSK(Computer* comp, const char *name, int drv) {
	Floppy* flp = comp->dif->fdc->flop[drv & 3];
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
	int ext = -1;
	int tr,sc,i;
	int pos;
	int trkcnt;
	int sidcnt;
	int tlen;
	int slen;
	int rtlen;
	TrackInfBlock tib;
	SectorInfBlock* sib;
	Sector secs[256];
	char sigBuf[256];
	fread(sigBuf, 256, 1, file);					// disk info block
	if (!strncmp(sigBuf, edsksgn, strlen(edsksgn))) {		// extend dsk format
		ext = 1;
	} else if (!strncmp(sigBuf, dsksgn, strlen(dsksgn))) {		// common dsk format
		ext = 0;
	}
	if (ext < 0) {
		err = ERR_DSK_SIGN;
	} else {
		diskClear(flp);
		trkcnt = sigBuf[0x30] & 0xff;
		sidcnt = sigBuf[0x31] & 0xff;
		tlen = (sigBuf[0x32] & 0xff) | ((sigBuf[0x33] & 0xff) << 8);
		tr = 0;
		pos = 0x100;								// 1st track allways @ 0x100
		for (i = 0; (i < trkcnt * sidcnt) && (err == ERR_OK); i++) {
			rtlen = ext ? sigBuf[0x34 + i] << 8 : tlen;
			if (rtlen > 0) {
				fseek(file, pos, SEEK_SET);					// move to track info block
				fread(sigBuf, 12, 1, file);
				if (strncmp(sigBuf,"Track-Info\r\n",12) == 0) {			// track-info
					fread((char*)&tib, sizeof(TrackInfBlock), 1, file);
					sib = (SectorInfBlock*)&tib.sectorInfo;
					for (sc = 0; sc < tib.secCount; sc++) {
						if (!ext) {
							if (sib[sc].size > 5)
								slen = 0x1800;
							else
								slen = (0x80 << sib[sc].size) & 0xffff;
						} else {
							slen = sib[sc].bslo | (sib[sc].bshi << 8);
						}
						secs[sc].trk = sib[sc].track;
						secs[sc].head = sib[sc].side;
						secs[sc].sec = sib[sc].sector;
						secs[sc].sz = sib[sc].size;
						secs[sc].type = 0xfb;
						// printf("Sector %i:%i:%i(%i)\n",sib[sc].track,sib[sc].side,sib[sc].sector,sib[sc].bytesSize);
						fread((char*)secs[sc].data, slen, 1, file);
					}
					diskFormTrack(flp,tr,secs,tib.secCount);
					tr++;
					if (sidcnt == 1) tr++;
					pos += rtlen;
				} else {
					err = ERR_DSK_SIGN;
				}
			} else {
				// flp_format_trk(flp, tr, 10, 512, NULL);
				tr++;
				if (sidcnt == 1) tr++;
			}
		}
		flp_set_path(flp, name);
		//flp->doubleSide = (sidcnt > 1) ? 1 : 0;
		//flp->trk80 = (trkcnt > 42) ? 1 : 0;
		flp->insert = 1;
		flp->changed = 0;
	}
	fclose(file);
	return err;
}

long dsk_save_track(Floppy* flp, int trk, int side, FILE* file) {
	TrackInfBlock tinf;
	Sector sdata[29];
	long fpos = ftell(file);
	int sec = 0;
	int pos = 0;
	int i, dsz, dps;
	int mdsz = 0;
	int rtrk = (trk << 1) | side;
	side &= 1;
	memset(&tinf, 0, sizeof(TrackInfBlock));
	// scan track & collect sectors info & data
	for (pos = 0; (pos < flp->trklen) && (sec < 29); pos++) {
		if (flp->data[rtrk].field[pos] == 1) {			// sector info
			sdata[sec].trk = flp->data[rtrk].byte[pos++];	// C
			sdata[sec].head = flp->data[rtrk].byte[pos++];	// H
			sdata[sec].sec = flp->data[rtrk].byte[pos++];	// R
			sdata[sec].sz = flp->data[rtrk].byte[pos++] & 7;// N
			pos += 2;					// skip crc
			if (sdata[sec].sz > mdsz)			// max N
				mdsz = sdata[sec].sz;
			dsz = 128 << sdata[sec].sz;			// sector data size
			if (dsz > 0x1800)
				dsz = 0x1800;
			dps = flp->data[rtrk].map[sdata[sec].sec];	// position of sector data
			if (dps > 0)
				sdata[sec].type = flp->data[rtrk].byte[dps - 1];	// sector data type (FB | F8)
			memcpy(sdata[sec].data, flp->data[rtrk].byte + dps, dsz);	// copy sector data
			sec++;
		}
	}
	if (sec < 1)		// no sectors found, no data written, size = 0
		return 0;
	// fill track info
	tinf.track = trk & 0xff;
	tinf.side = side & 1;
	tinf.secSize = mdsz & 7;		// 0 @ extend dsk ?
	tinf.secCount = sec & 0xff;
	tinf.gap3Size = 0x1a;
	tinf.filler = 0xe5;
	pos = 0;
	for (i = 0; i < sec; i++) {
		tinf.sectorInfo[pos] = sdata[i].trk;
		tinf.sectorInfo[pos + 1] = sdata[i].head;
		tinf.sectorInfo[pos + 2] = sdata[i].sec;
		tinf.sectorInfo[pos + 3] = sdata[i].sz;
		dsz = 128 << sdata[i].sz;
		if (i == (sec - 1))				// end of cylinder
			tinf.sectorInfo[pos + 4] = 0x80;
		tinf.sectorInfo[pos + 6] = dsz & 0xff;
		tinf.sectorInfo[pos + 7] = (dsz >> 8) & 0xff;
		pos += 8;
	}
	// save track info block
	fwrite("Track-Info\r\n", 12, 1, file);
	fwrite(&tinf, sizeof(TrackInfBlock), 1, file);
	// save sectors data
	for (i = 0; i < sec; i++) {
		dsz = (128 << sdata[i].sz);
		if (dsz > 0x1800)
			dsz = 0x1800;
		fwrite(sdata[i].data, dsz, 1, file);
	}
	return ftell(file) - fpos;
}

static const char sign[] = "EXTENDED CPC DSK File\r\nDisk-Info\r\nXpeccy        ";

int saveDSK(Computer* comp, const char* name, int drv) {
	int err = ERR_OK;
	FILE* file = fopen(name, "wb");
	if (file) {
		Floppy* flp = comp->dif->fdc->flop[drv & 3];
		unsigned char buf[256];
		memset(buf, 0, 0x100);
		memcpy(buf, sign, 0x22 + 0x0e);
		// 30 = track count, will be updated later
		// 31: 1 | 2 sides
		// 32,33 = 0 @ extend format
		// 34+N = size of track N (from Track-Info to Track-Info)
		// fwrite(buf, 256, 1, file);		// signature + disk info : buf will be saved later
		fseek(file, 256, SEEK_SET);
		int cpos = 0x34;
		long isize;
		int ds = 0;
		int tcount = 0;
		for (int i = 0; i < 86; i++) {
			isize = dsk_save_track(flp, i, 0, file);
			buf[cpos] = (isize >> 8) & 0xff;
			cpos++;
			if (isize > 0)
				tcount = i + 1;
			if (flp->doubleSide) {
				isize = dsk_save_track(flp, i, 1, file);
				if (isize > 0)
					ds = 1;
				buf[cpos] = (isize >> 8) & 0xff;
				cpos++;
				if (isize > 0)
					tcount = i + 1;
			}
		}
		buf[0x30] = (tcount < 43) ? 42 : 84; // = tcount & 0xff;
		buf[0x31] = ds ? 2 : 1;
		if (!ds) {
			cpos = 0x36;		// 0x34 doesn't changed
			tcount = 0x35;
			while(cpos < 256) {
				buf[tcount] = buf[cpos];
				buf[cpos] = 0;
				tcount++;
				cpos += 2;
			}
		}
		// save disk info block
		fseek(file, 0, SEEK_SET);
		fwrite(buf, 256, 1, file);
		fclose(file);
		// update name
		flp_set_path(flp, name);
		flp->changed = 0;
	} else {
		err = ERR_CANT_OPEN;
	}
	return err;
}
