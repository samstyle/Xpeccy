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

enum {
	ERR_OK = 0,
	ERR_CANT_OPEN,		// can't open file

	ERR_RZX_SIGN,		// rzx signature error
	ERR_RZX_CRYPT,		// rzx is crypted
	ERR_RZX_UNPACK,		// rzx unpacking error

	ERR_TAP_DATA,		// can't save tap because of not-standart blocks

	ERR_TZX_SIGN,		// tzx signature error
	ERR_TZX_UNKNOWN,	// tzx unsupported block

	ERR_WAV_HEAD,		// wrong wave header
	ERR_WAV_FORMAT,		// unsupported wav format

	ERR_TRD_LEN,		// incorrect trd lenght
	ERR_TRD_SIGN,		// not trd image
	ERR_TRD_SNF,		// can't save trd: wrong disk structure
	ERR_NOTRD,		// this is not trd disk

	ERR_HOB_CANT,		// can't create hobeta @ disk

	ERR_UDI_SIGN,		// udi signature errror

	ERR_FDI_SIGN,		// fdi signature error
	ERR_FDI_HEAD,		// wrong fdi heads count

	ERR_SCL_SIGN,		// scl signature error
	ERR_SCL_MANY,		// too many files in scl

	ERR_DSK_SIGN,		// dsk signature error

	ERR_RAW_LONG,		// raw file too long

	ERR_TD0_SIGN,		// td0 signature error
	ERR_TD0_TYPE,		// unsupported td0 type
	ERR_TD0_VERSION		// unsupported version ( <20)
};

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

int fgeti(FILE*);
unsigned short fgetw(FILE*);
void fputi(int, FILE*);
void fputw(unsigned short, FILE*);

size_t fgetSize(FILE*);
unsigned int freadLen(FILE*,int);
void fputwLE(FILE*, unsigned short);

void putint(unsigned char*, unsigned int);
void cutSpaces(char*);
void loadBoot(Floppy*, const char*);

unsigned short swap16(unsigned short);
unsigned int swap32(unsigned int);

// rzx

int loadRZX(Computer*,const char*);
void rzxGetFrame(Computer*);

// memory (snapshot)

int loadDUMP(Computer*, const char*, int);

int loadZ80(Computer*,const char*);
int loadZ80_f(Computer*, FILE*);

int loadSNA(Computer*,const char*);
int saveSNA(Computer*, const char*, int);
int loadSNA_f(Computer*, FILE*, size_t);

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

int loadSlot(xCartridge*,const char*);

#ifdef __cplusplus
}
#endif

#endif
