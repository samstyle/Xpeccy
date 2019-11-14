#ifndef _FILETYPES_H
#define _FILETYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../spectrum.h"


#if __linux || __APPLE__
	#define ENVHOME "HOME"
	#define SLASH "/"
	#define SLSH '/'
#elif _WIN32
	#define ENVHOME "HOMEPATH"
	#define SLASH "\\"
	#define SLSH '\\'
#endif

enum {
	ERR_OK = 0,
	ERR_CANT_OPEN,		// can't open file

	ERR_RZX_SIGN,		// rzx signature error
	ERR_RZX_CRYPT,		// rzx is crypted
	ERR_RZX_UNPACK,		// rzx unpacking error

	ERR_Z80_HW,		// Z80 hw mode not supported

	ERR_TAP_DATA,		// can't save tap because of not-standart blocks

	ERR_TZX_SIGN,		// tzx signature error
	ERR_TZX_UNKNOWN,	// tzx unsupported block

	ERR_T64_SIGN,		// T64 signature
	ERR_C64T_SIGN,		// C64 raw tape signature

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
	ERR_TD0_VERSION,	// unsupported version ( <20)

	ERR_NES_HEAD,		// header error
	ERR_NES_MAPPER		// unsupported mapper
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
int fgett(FILE*);
unsigned short fgetw(FILE*);
void fputi(int, FILE*);
void fputw(unsigned short, FILE*);

size_t fgetSize(FILE*);
unsigned int freadLen(FILE*,int);
void fputwLE(FILE*, unsigned short);

void putint(unsigned char*, unsigned int);
void cutSpaces(char*);
void loadBoot(Computer*, const char*, int);

unsigned short swap16(unsigned short);
unsigned int swap32(unsigned int);

// rzx

int loadRZX(Computer*, const char*, int);
void rzxGetFrame(Computer*);

// memory (snapshot)

int loadDUMP(Computer*, const char*, int);

int loadZ80(Computer*,const char*, int);
int loadZ80_f(Computer*, FILE*);

int loadSNA(Computer*,const char*, int);
int saveSNA(Computer*, const char*, int);
int loadSNA_f(Computer*, FILE*, size_t);

int loadSPG(Computer*,const char*, int);

int loadT64(Computer*,const char*,int);

int loadBIN(Computer*, const char*, int);

// tape

int loadTAP(Computer*,const char*, int);
int saveTAP(Computer*,const char*, int);
void blkFromData(TapeBlock*, char*, int, int*);
TapeBlock tapDataToBlock(char*,int,int*);

int loadTZX(Computer*,const char*, int);

int loadWAV(Computer*, const char*, int);

int loadC64RawTap(Computer*, const char*, int);

// disk

int loadRaw(Computer*,const char*, int);
int saveRawFile(Floppy*,int,const char*);

int loadHobeta(Computer*,const char*,int);
int saveHobetaFile(Floppy*,int,const char*);
int saveHobeta(TRFile,char*,const char*);

int loadSCL(Computer*,const char*,int);
int saveSCL(Computer*,const char*,int);

int loadTRD(Computer*,const char*,int);
int saveTRD(Computer*,const char*,int);

int loadUDI(Computer*,const char*,int);
int saveUDI(Computer*,const char*,int);

int loadFDI(Computer*,const char*,int);

int loadDSK(Computer*,const char*,int);

int loadTD0(Computer*,const char*,int);

int loadBkIMG(Computer*,const char*,int);
// cartridge

int loadSlot(Computer*,const char*, int);
int loadNes(Computer*, const char*, int);

#ifdef __cplusplus
}
#endif

#endif
