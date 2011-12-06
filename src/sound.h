#ifndef _SOUND_H
#define _SOUND_H

#include <vector>
#include <string>
#include <stdint.h>

#ifndef WIN32
#ifdef HAVEALSASOUND
	#include <alsa/asoundlib.h>
#endif
	#include <sys/ioctl.h>
	#include <sys/soundcard.h>
	#include <fcntl.h>
#else
	#include <windef.h>
	#include <mmsystem.h>
//	#include <dsound.h>
#endif

#define	SND_RATE	0
#define	SND_COUNT	1
#define	SND_BEEP	2
#define	SND_TAPE	3
#define	SND_AYVL	4
#define	SND_GSVL	5
#define	SND_ENABLE	6
#define	SND_MUTE	7

void sndInit();
void sndCalibrate();
void addOutput(std::string, bool(*)(),void(*)(),void(*)());
void setOutput(std::string);
std::string sndGetOutputName();
bool sndOpen();
void sndPlay();
void sndPause(bool);
void sndClose();
double sndSync(double);
int sndGet(int);
void sndSet(int,int);
void sndSet(int,bool);
std::vector<std::string> sndGetList();
std::string sndGetName();

#endif
