#ifndef _XPGSOUND
#define _XPGSOUND

#include <stdint.h>
#include <utility>

#define	GS_STEREO	0
#define	GS_FLAG		1

#define GS_ENABLE	1
#define GS_RESET	(1<<1)

#define	GS_MONO		0
#define	GS_12_34	1

#define	GS_OK	0
#define	GS_ERR	1

struct GSound;

GSound* gsCreate();
void gsDestroy(GSound*);
void gsReset(GSound*);
void gsSync(GSound*, int);
std::pair<uint8_t,uint8_t> gsGetVolume(GSound*);

int gsIn(GSound*, int, uint8_t*);
int gsOut(GSound*, int, uint8_t);

int gsGet(GSound*,int);
void gsSet(GSound*,int,int);
void gsSetRom(GSound*,int,char*);

#endif
