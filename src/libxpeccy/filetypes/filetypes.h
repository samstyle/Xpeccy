#ifndef _FILETYPES_H
#define _FILETYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
#include <fstream>
#endif

#ifdef WORDS_BIG_ENDIAN
	#include <endian.h>
#endif

#include "../spectrum.h"

#ifdef _WIN32
#define SLASH "\\"
#endif

#ifdef __linux__
#define SLASH "/"
#endif

#define	TYP_SNA		0
#define	TYP_Z80 	1
#define	TYP_RZX 	2
#define TYPE_TRD	3
#define TYPE_SCL	4
#define TYPE_FDI	5
#define TYPE_UDI	6
#define TYPE_HOBETA	7
#define	TYPE_TAP	8
#define	TYPE_TZX	9

#define	ERR_OK		0
#define	ERR_CANT_OPEN	1	// can't open file

#define	ERR_RZX_SIGN	0x10	// rzx signature error
#define	ERR_RZX_CRYPT	0x11	// rzx is crypted
#define	ERR_RZX_UNPACK	0x12	// rzx unpacking error

#define	ERR_TAP_DATA	0x20	// can't save tap because of not-standart blocks

#define	ERR_TZX_SIGN	0x30	// tzx signature error
#define	ERR_TZX_UNKNOWN	0x31	// tzx unsupported block

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

// spg

int loadSPG(ZXComp*,const char*);		// still in c++ part because of depackers

#ifdef __cplusplus
extern "C" {
#endif

// common

unsigned int freadLen(FILE*,int);
unsigned short fgetwLE(FILE*);
void fputwLE(FILE*, unsigned short);

void putint(unsigned char*, unsigned int);
void cutSpaces(char*);
void loadBoot(Floppy*, const char*);

// rzx

int loadRZX(ZXComp*,const char*);
void rzxLoadFrame(ZXComp*);

// memory (snapshot)

int loadDUMP(ZXComp*, const char*, int);

int loadZ80(ZXComp*,const char*);

int loadSNA(ZXComp*,const char*);
int saveSNA(ZXComp*, const char*, int);

// tape

int loadTAP(Tape*,const char*);
int saveTAP(Tape*,const char*);
TapeBlock tapDataToBlock(char*,int,int*);

int loadTZX(Tape*,const char*);

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

#ifdef __cplusplus
}
#endif

#endif
