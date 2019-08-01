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
//	DiskInfBlock dib;
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
		trkcnt = sigBuf[0x30] & 0xff;
		sidcnt = sigBuf[0x31] & 0xff;
		tlen = (sigBuf[0x32] & 0xff) | ((sigBuf[0x33] & 0xff) << 8);
		tr = 0;
		pos = 0x100;								// 1st track allways @ 0x100
		for (i = 0; (i < trkcnt * sidcnt) && (err == ERR_OK); i++) {
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
				pos += ext ? (sigBuf[0x34 + i] << 8) : tlen;
			} else {
				err = ERR_DSK_SIGN;
			}
		}
		flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
		strcpy(flp->path,name);
		// flp->doubleSide = (sidcnt > 1) ? 1 : 0;
		flp->trk80 = 1;
		flp->insert = 1;
		flp->changed = 0;
	}
	fclose(file);
	return err;
}
