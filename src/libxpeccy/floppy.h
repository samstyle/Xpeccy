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
#define ERR_SIZE	4
// step direction
#define	FLP_BACK	0
#define	FLP_FORWARD	1

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
	unsigned insert:1;
	unsigned protect:1;
	unsigned trk80:1;
	unsigned doubleSide:1;
	unsigned index:1;
	unsigned motor:1;
	unsigned head:1;
	unsigned changed:1;
	unsigned side:1;
	unsigned virt:1;

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

int flpCreateDescriptor(Floppy*,TRFile*);
int flpCreateFile(Floppy*, TRFile, unsigned char*, int);
int flpGetTRCatalog(Floppy*,TRFile*);
TRFile flpGetCatalogEntry(Floppy*, int);
TRFile flpMakeDescriptor(const char*, char, int, int);

#ifdef __cplusplus
}
#endif

#endif
