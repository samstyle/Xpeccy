#ifndef _XPGSOUND
#define _XPGSOUND

#include <stdint.h>

#define GS_ENABLE	1
#define GS_RESET	2

#define	GS_STEREO	0
#define	GS_MONO		0
#define	GS_12_34	1

#define	GS_OK	0
#define	GS_ERR	1

struct GSound;
typedef struct {
	int r;
	int l;
} GSData;

GSound* gsCreate();
void gsDestroy(GSound*);
int gsGetFlag(GSound*);
void gsSetFlag(GSound*,int);
int gsGetParam(GSound*,int);
void gsSetParam(GSound*,int,int);
void gsSetRom(GSound*,int,char*);
void gsReset(GSound*);
GSData gsGetVolume(GSound*);
void gsSync(GSound*, uint32_t);
int gsIn(GSound*, int, uint8_t*);
int gsOut(GSound*, int, uint8_t);

#endif
