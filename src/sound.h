#ifndef _SOUND_H
#define _SOUND_H

#include <vector>
#include <string>
//#include <stdint.h>

#ifndef _WIN32
#ifdef HAVEALSA
	#include <alsa/asoundlib.h>
#endif
	#include <sys/ioctl.h>
	#include <sys/soundcard.h>
	#include <fcntl.h>
#else
	#include <windows.h>
	#include <windef.h>
	#include <mmsystem.h>
//	#include <dsound.h>
#endif

#define	SND_NULL	0
#define	SND_OSS		1
#define	SND_ALSA	2
#define	SND_SDL		3

#define	SND_RATE	0
#define	SND_COUNT	1
#define	SND_BEEP	2
#define	SND_TAPE	3
#define	SND_AYVL	4
#define	SND_GSVL	5
#define	SND_ENABLE	6
#define	SND_MUTE	7

extern bool sndEnabled;
extern bool sndMute;
//extern volatile int smpCount;
extern int beepVolume;
extern int tapeVolume;
extern int ayVolume;
extern int gsVolume;
extern int sndRate;

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
void sndSync(int);

void sndFillToEnd();

#endif
