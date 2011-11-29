#include <stdint.h>
#include <fstream>

#include "../spectrum.h"

#define	ERR_OK		0
#define	ERR_CANT_OPEN	1	// can't open file

#define	ERR_RZX_SIGN	0x10	// rzx signature error
#define	ERR_RZX_CRYPT	0x11	// rzx is crypted
#define	ERR_RZX_UNPACK	0x12	// rzx unpacking error

#define	ERR_TZX_SIGN	0x40	// tzx signature error
#define	ERR_TZX_UNKNOWN	0x41	// tzx unsupported block
#define	ERR_TAP_DATA	0x50	// can't save tap because of not-standart blocks

uint16_t getLEWord(std::ifstream*);
uint16_t getBEWord(std::ifstream*);

// snapshot

int loadSNA(ZXComp*,const char*);
int saveSNA(ZXComp*,const char*,bool);

int loadZ80(ZXComp*,const char*);

int loadRZX(ZXComp*,const char*);

// tape

TapeBlock tapDataToBlock(char*,int,int*);

int loadTAP(Tape*,const char*);
int saveTAP(Tape*,const char*);

int loadTZX(Tape*,const char*);
