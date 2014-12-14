#ifndef _SOUND_H
#define _SOUND_H

#include <vector>
#include <string>

#if __linux
#ifdef HAVEALSA
	#include <alsa/asoundlib.h>
#endif
	#include <sys/ioctl.h>
	#include <sys/soundcard.h>
	#include <fcntl.h>
#endif
#if __WIN32
	#include <windows.h>
	#include <windef.h>
	#include <mmsystem.h>
#endif

#include "sound.h"
#include "xcore/xcore.h"
#include "libxpeccy/spectrum.h"

#define	SND_NULL	0
#define	SND_OSS		1
#define	SND_ALSA	2
#define	SND_SDL		3
#define SND_WAVE	4

#define	SND_RATE	0
#define	SND_COUNT	1
#define	SND_BEEP	2
#define	SND_TAPE	3
#define	SND_AYVL	4
#define	SND_GSVL	5
#define	SND_ENABLE	6
#define	SND_MUTE	7

extern long nsPerFrame;

struct OutSys {
	int id;
	const char* name;
	bool (*open)();
	void (*play)();
	void (*close)();
};

extern OutSys sndTab[];
extern OutSys* sndOutput;

extern int nsPerSample;

void sndInit();
void sndCalibrate();
void addOutput(std::string, bool(*)(),void(*)(),void(*)());
void setOutput(const char*);
bool sndOpen();
void sndPlay();
void sndPause(bool);
void sndClose();
void sndSync(ZXComp*, int);

void sndFillToEnd();

#endif
