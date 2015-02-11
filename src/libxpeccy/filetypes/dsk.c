#include "filetypes.h"

#pragma pack (push, 1)

typedef struct {
	char signature[34];
	char creator[14];
	unsigned char tracks;
	unsigned char sides;
	char byte31;
	char byte32;
	char trackSize[0x100 - 0x34];
} DiskInfBlock;

typedef struct {
//	char signature[13];
	char byte0d0f[3];	// unused
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
	unsigned short bytesSize;
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

int loadDSK(Floppy* flp, const char *name) {
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
	DiskInfBlock dib;
	fread((char*)&dib, sizeof(DiskInfBlock), 1, file);
	if (strncmp(dib.signature,"EXTENDED CPC DSK File\r\nDisk-Info\r\n",34) != 0) {
		err = ERR_DSK_SIGN;
	} else {
		char sigBuf[256];
		memset(sigBuf,0,256);
		TrackInfBlock tib;
		SectorInfBlock* sib;
		int tr = 0;
		int i,sc;
		Sector secs[256];

		for (i = 0; i < 256; i++)
			secs[i].data = NULL;

		for (i = 0; i < dib.tracks * dib.sides; i++) {
			if (!feof(file)) {
				fgetLine(file, sigBuf, 255, 0);					// read block signature (till byte 0x00)
				if (strncmp(sigBuf,"Track-Info\r\n",12) == 0) {			// track-info
					fread((char*)&tib, sizeof(TrackInfBlock), 1, file);
					sib = (SectorInfBlock*)&tib.sectorInfo;
					for (sc = 0; sc < tib.secCount; sc++) {
						secs[sc].cyl = sib[sc].track;
						secs[sc].side = sib[sc].side;
						secs[sc].sec = sib[sc].sector;
						secs[sc].len = sib[sc].size;
						secs[sc].data = (unsigned char*)realloc(secs[sc].data,sib[sc].bytesSize * sizeof(char));
						secs[sc].type = 0xfb;
						fread((char*)secs[sc].data, sib[sc].bytesSize, 1, file);
					}
					flpFormTrack(flp,tr,secs,tib.secCount);
					tr++;
					if (dib.sides == 1) tr++;
				} else {
					break;
				}
			}
		}
		for (i = 0; i < 256; i++) {
			if (secs[i].data)
				free(secs[i].data);
		}
		flp->path = (char*)realloc(flp->path,sizeof(char) * (strlen(name) + 1));
		strcpy(flp->path,name);
		flp->insert = 1;
		flp->changed = 0;
	}
	fclose(file);
	return err;
}
