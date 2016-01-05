#ifndef _FILETYPES_H
#define _FILETYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../spectrum.h"


#if __linux || __APPLE__
	#define ENVHOME "HOME"
	#define SLASH "/"
#elif _WIN32
	#define ENVHOME "HOMEPATH"
	#define SLASH "\\"
#endif

#define	ERR_OK		0
#define	ERR_CANT_OPEN	1	// can't open file

#define	ERR_RZX_SIGN	0x10	// rzx signature error
#define	ERR_RZX_CRYPT	0x11	// rzx is crypted
#define	ERR_RZX_UNPACK	0x12	// rzx unpacking error

#define	ERR_TAP_DATA	0x20	// can't save tap because of not-standart blocks

#define	ERR_TZX_SIGN	0x30	// tzx signature error
#define	ERR_TZX_UNKNOWN	0x31	// tzx unsupported block

#define ERR_WAV_HEAD	0x38	// wrong wave header
#define	ERR_WAV_FORMAT	0x39	// unsupported wav format

#define	ERR_TRD_LEN	0x40	// incorrect trd lenght
#define	ERR_TRD_SIGN	0x41	// not trd image
#define	ERR_TRD_SNF	0x42	// can't save trd: wrong disk structure
#define	ERR_NOTRD	0x43	// this is not trd disk

#define	ERR_HOB_CANT	0x50	// can't create hobeta @ disk

#define	ERR_UDI_SIGN	0x60	// udi signature errror

#define	ERR_FDI_SIGN	0x70	// fdi signature error
#define	ERR_FDI_HEAD	0x71	// wrong fdi heads count

#define	ERR_SCL_SIGN	0x80	// scl signature error
#define	ERR_SCL_MANY	0x81	// too many files in scl

#define	ERR_DSK_SIGN	0x88	// dsk signature error

#define	ERR_RAW_LONG	0x90	// raw file too long

#define	ERR_TD0_SIGN	0x98	// td0 signature error
#define ERR_TD0_TYPE	0x99	// unsupported td0 type
#define ERR_TD0_VERSION	0x9a	// unsupported version ( <20)

// spg

#ifdef __cplusplus
extern "C" {
#endif

// disk specific operations

typedef struct {
	unsigned char trk;
	unsigned char head;
	unsigned char sec;
	unsigned char sz;	// 0..3 = 128..1024
	unsigned char type;
	int crc;
	unsigned char data[4096];
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

void diskFormat(Floppy*);
void diskFormTrack(Floppy*,int,Sector*,int);
void diskFormTRDTrack(Floppy*,int,unsigned char*);

int diskGetType(Floppy*);

int diskGetSectorData(Floppy*,unsigned char,unsigned char,unsigned char*,int);
int diskGetSectorsData(Floppy*,unsigned char,unsigned char,unsigned char*,int);
int diskPutSectorData(Floppy*,unsigned char,unsigned char,unsigned char*,int);

int diskCreateDescriptor(Floppy*,TRFile*);
int diskCreateFile(Floppy*, TRFile, unsigned char*, int);
int diskGetTRCatalog(Floppy*,TRFile*);
TRFile diskGetCatalogEntry(Floppy*, int);
TRFile diskMakeDescriptor(const char*, char, int, int);

// common

size_t fgetSize(FILE*);
unsigned int freadLen(FILE*,int);
unsigned short fgetwLE(FILE*);
void fputwLE(FILE*, unsigned short);

void putint(unsigned char*, unsigned int);
void cutSpaces(char*);
void loadBoot(Floppy*, const char*);

unsigned short swap16(unsigned short);
unsigned int swap32(unsigned int);

// rzx

int loadRZX(Computer*,const char*);
void rzxLoadFrame(Computer*);

// memory (snapshot)

int loadDUMP(Computer*, const char*, int);

int loadZ80(Computer*,const char*);

int loadSNA(Computer*,const char*);
int saveSNA(Computer*, const char*, int);

int loadSPG(Computer*,const char*);

// tape

int loadTAP(Tape*,const char*);
int saveTAP(Tape*,const char*);
void blkFromData(TapeBlock*, char*, int, int*);
TapeBlock tapDataToBlock(char*,int,int*);

int loadTZX(Tape*,const char*);

int loadWAV(Tape*, const char*);

// disk

int loadRaw(Floppy*,const char*);
int saveRawFile(Floppy*,int,const char*);

int loadHobeta(Floppy*,const char*);
int saveHobetaFile(Floppy*,int,const char*);
int saveHobeta(TRFile,char*,const char*);

int loadSCL(Floppy*,const char*);
int saveSCL(Floppy*,const char*);

int loadTRD(Floppy*,const char*);
int saveTRD(Floppy*,const char*);

int loadUDI(Floppy*,const char*);
int saveUDI(Floppy*,const char*);

int loadFDI(Floppy*,const char*);

int loadDSK(Floppy*,const char*);

int loadTD0(Floppy*,const char*);

// cartridge

int loadCARD(Computer*,const char*,int);

#ifdef __cplusplus
}
#endif

#endif
