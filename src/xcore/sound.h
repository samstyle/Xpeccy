#ifndef X_SOUND_H
#define X_SOUND_H

#include <vector>
#include <string>

#if __linux
	#include <sys/ioctl.h>
	#include <sys/soundcard.h>
	#include <fcntl.h>
#endif

#include "sound.h"
#include "xcore.h"

enum xSoundOutput {
	xOutputNone = 0,
	xOutputOss,
	xOutputAlsa,
	xOutputSDL,
	xOutputWave
};

typedef struct {
	int id;
	const char* name;
	int (*open)();
	void (*play)();
	void (*close)();
} OutSys;

extern OutSys sndTab[];
extern OutSys* sndOutput;

extern int nsPerSample;

void sndInit();
void sndCalibrate(Computer*);
void addOutput(std::string, bool(*)(),void(*)(),void(*)());
void setOutput(const char*);

int sndOpen();
void sndPlay();
void sndClose();
int sndSync(Computer*);

#endif
