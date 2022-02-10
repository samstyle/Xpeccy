#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"

#define TRKLEN_DD	6250
#define TRKLEN_HD	17700
// disk type
#define DISK_TYPE_TRD	1

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
	unsigned char mark;	// f8/fb
	unsigned char size;	// code 0..3
	unsigned short pos;	// position in track
} xSecPos;

typedef struct {
	unsigned trk80:1;	// fdd is 80T
	unsigned doubleSide:1;	// fdd is DS
	unsigned motor:1;	// fdd motor is on
	unsigned virt:1;	// fdd is virtual
	unsigned insert:1;	// disk inserted
	unsigned door:1;	// door closed (auto close some time after insert)
	unsigned protect:1;	// disk is write protected
	unsigned changed:1;	// disk is changed
	unsigned index:1;	// disk index impulse

	unsigned rd:1;
	unsigned wr:1;

	int id;
	unsigned char trk;
	unsigned char field;
	int pos;
	int trklen;		// 12500 HD, 6250 DD
	char* path;
	struct {
		unsigned char byte[TRKLEN_HD];
		unsigned char field[TRKLEN_HD];
		int map[256];		// position of sector n = 1+
	} data[256];
} Floppy;

Floppy* flpCreate(int);
void flpDestroy(Floppy*);

void flp_set_path(Floppy*, const char*);
void flp_set_hd(Floppy*, int);

int flpEject(Floppy*);
unsigned char flpRd(Floppy*, int);
void flpWr(Floppy*, int, unsigned char);
int flpNext(Floppy*,int);		// return 1 if index strobe
void flpPrev(Floppy*,int);
void flpStep(Floppy*,int);

int flp_format_trk(Floppy* flp, int trk, int spt, int slen, char* data);
// int flp_format_trk_buf(int trk, int spt, int slen, int trklen, char* data, unsigned char* buf);

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
