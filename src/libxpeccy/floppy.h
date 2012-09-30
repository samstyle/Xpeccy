#ifndef _FLOPPY_H
#define	_FLOPPY_H

#ifdef __cplusplus
extern "C" {
#endif

//#include <stdint.h>

#define TRACKLEN 6250
// disk type
#define DISK_TYPE_TRD	1
// get
#define	FLP_DISKTYPE	10
// error
#define	ERR_OK		0
#define	ERR_MANYFILES	1
#define	ERR_NOSPACE	2
#define	ERR_SHIT	3
// step direction
#define	FLP_BACK	0
#define	FLP_FORWARD	1
// flags
#define	FLP_INSERT	1
#define	FLP_PROTECT	(1<<1)
#define	FLP_TRK80	(1<<2)
#define	FLP_DS		(1<<3)
#define	FLP_INDEX	(1<<4)
#define	FLP_MOTOR	(1<<5)
#define	FLP_HEAD	(1<<6)
#define	FLP_CHANGED	(1<<7)
#define	FLP_SIDE	(1<<8)

typedef struct {
	unsigned char cyl;
	unsigned char side;
	unsigned char sec;
	unsigned char len;
	unsigned char* data;
	unsigned char type;
	int crc;
	int flag;
} Sector;

typedef struct {
	unsigned char name[8];
	unsigned char ext;
	unsigned char lst,hst;
	unsigned char llen,hlen;
	unsigned char slen;
	unsigned char sec;
	unsigned char trk;
} TRFile;

typedef struct {
	int flag;
	unsigned char id;
	unsigned char iback;
	unsigned char trk,rtrk;
	unsigned char field;
	int pos;
	unsigned int ti;
	char* path;
	struct {
		unsigned char byte[TRACKLEN];
		unsigned char field[TRACKLEN];
	} data[256];
} Floppy;

Floppy* flpCreate(int);
void flpDestroy(Floppy*);

unsigned char flpRd(Floppy*);
void flpWr(Floppy*,unsigned char);
int flpEject(Floppy*);
unsigned char flpGetField(Floppy*);
int flpNext(Floppy*,int);		// return 1 if index strobe
void flpPrev(Floppy*,int);
void flpStep(Floppy*,int);

int flpGet(Floppy*,int);
void flpSet(Floppy*,int,int);

void flpClearTrack(Floppy*,int);
void flpClearDisk(Floppy*);

void flpFormat(Floppy*);
void flpFormTrack(Floppy*,int,Sector*,int);
void flpFormTRDTrack(Floppy*,int,unsigned char*);
void flpFillFields(Floppy*,int,int);

int flpGetSectorData(Floppy*,unsigned char,unsigned char,unsigned char*,int);
int flpGetSectorsData(Floppy*,unsigned char,unsigned char,unsigned char*,int);
int flpPutSectorData(Floppy*,unsigned char,unsigned char,unsigned char*,int);
void flpPutTrack(Floppy*,int,unsigned char*,int);
void flpGetTrack(Floppy*,int,unsigned char*);
void flpGetTrackFields(Floppy*,int,unsigned char*);

int flpCreateFile(Floppy*,TRFile*);
int flpGetTRCatalog(Floppy*,TRFile*);
TRFile flpGetCatalogEntry(Floppy*, int);

#ifdef __cplusplus
}
#endif

#endif
