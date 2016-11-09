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
enum {
	ERR_MANYFILES = 1,
	ERR_NOSPACE,
	ERR_SHIT,
	ERR_SIZE
};

// step direction
enum {
	FLP_BACK = 0,
	FLP_FORWARD
};

typedef struct {
	unsigned trk80:1;	// fdd is 80T
	unsigned doubleSide:1;	// fdd is DS
	unsigned motor:1;	// fdd motor is on
	unsigned virt:1;	// fdd is virtual
	unsigned insert:1;	// disk inserted
	unsigned protect:1;	// disk is write protected
	unsigned changed:1;	// disk is changed
	unsigned index:1;	// disk index impulse

	unsigned rd:1;
	unsigned wr:1;

	unsigned char id;
	unsigned char trk,rtrk;
	unsigned char field;
	int pos;
	char* path;
	struct {
		unsigned char byte[TRACKLEN];
		unsigned char field[TRACKLEN];
	} data[256];
} Floppy;

Floppy* flpCreate(int);
void flpDestroy(Floppy*);

int flpEject(Floppy*);
unsigned char flpRd(Floppy*);
void flpWr(Floppy*,unsigned char);
int flpNext(Floppy*,int);		// return 1 if index strobe
void flpPrev(Floppy*,int);
void flpStep(Floppy*,int);

unsigned char flpGetField(Floppy*);
void flpFillFields(Floppy*,int,int);

void flpPutTrack(Floppy*,int,unsigned char*,int);
void flpGetTrack(Floppy*,int,unsigned char*);
void flpGetTrackFields(Floppy*,int,unsigned char*);

void flpClearTrack(Floppy*,int);
void flpClearDisk(Floppy*);

unsigned short getCrc(unsigned char*, int);

#ifdef __cplusplus
}
#endif

#endif
