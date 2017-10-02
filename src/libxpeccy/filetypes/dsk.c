#include "filetypes.h"

// +0x00 signature (23)
// +0x17 DiskInfo\r\n
// +0x22 disk info block

#pragma pack (push, 1)

typedef struct {
	char creator[14];
	unsigned char tracks;
	unsigned char sides;
	unsigned char byte32;		// dsk:track size
	unsigned char byte33;
	char trackSize[0x100 - 0x34];	// edsk:tracks size
} DiskInfBlock;

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

static const char edsksgn[] =	"EXTENDED CPC DSK File\r\n";
static const char dsksgn[] =	"MV - CPCEMU Disk-File\r\n";

int loadDSK(Floppy* flp, const char *name) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
	int ext = -1;
	int tr,sc,i;
	int pos;
	int tlen;
	int slen;
	DiskInfBlock dib;
	TrackInfBlock tib;
	SectorInfBlock* sib;
	Sector secs[256];
	char sigBuf[256];
	memset(sigBuf,0,256);
	fread(sigBuf, 23, 1, file);					// file format signature

	if (!strncmp(sigBuf, edsksgn, strlen(edsksgn))) {		// extend dsk format
		ext = 1;
	} else if (!strncmp(sigBuf, dsksgn, strlen(dsksgn))) {		// common dsk format
		ext = 0;
	}
	if (ext < 0) {
		err = ERR_DSK_SIGN;
	} else {
		fread(sigBuf, 11, 1, file);
		if (!strncmp(sigBuf, "Disk-Info\r\n",11)) {
			fread((char*)&dib, sizeof (DiskInfBlock), 1, file);
			tr = 0;
			pos = 0x100;								// 1st track allways @ 0x100
			tlen = dib.byte32 | (dib.byte33 << 8);
			for (i = 0; (i < dib.tracks * dib.sides) && (err == ERR_OK); i++) {
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
					if (dib.sides == 1) tr++;
					pos += ext ? (dib.trackSize[i] << 8) : tlen;
				} else {
					err = ERR_DSK_SIGN;
				}
			}
			flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
			strcpy(flp->path,name);
			flp->doubleSide = (dib.sides > 1) ? 1 : 0;
			flp->trk80 = 1;
			flp->insert = 1;
			flp->changed = 0;
		} else {
			err = ERR_DSK_SIGN;
		}
	}
	fclose(file);
	return err;
}
