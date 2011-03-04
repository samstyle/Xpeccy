#ifndef _SOUND_H
#define _SOUND_H

#define HAVESDLSOUND 0

#include <vector>
#include <string>
#include <stdint.h>
#if HAVESDLSOUND
	#include <SDL_mixer.h>
#endif

#ifndef WIN32
	#include <alsa/asoundlib.h>
	#include <sys/ioctl.h>
	#include <sys/soundcard.h>
	#include <fcntl.h>
#else
	#include <windef.h>
	#include <mmsystem.h>
	#include <dsound.h>
#endif

#define	SND_NONE	0
#define	SND_AY		1
#define	SND_YM		2
#define	SND_END		3

#define	AY_MONO		0
#define	AY_ABC		1
#define	AY_ACB		2
#define	AY_BAC		3
#define	AY_BCA		4
#define	AY_CAB		5
#define	AY_CBA		6

#define	TS_NONE		0
#define	TS_NEDOPC	1

class SndData {
	public:
		int r;
		int l;
		friend SndData operator +(SndData p1, SndData p2) {p1.r += p2.r; p1.l += p2.l; return p1;}
		friend SndData operator *(SndData p1, float mul) {p1.r *= mul; p1.l *= mul; return p1;}
};

class OutSys {
	public:
		std::string name;
		bool (*open)();
		void (*play)();
		void (*close)();
};

class AYChan {
	public:
		AYChan();
		bool lev;		// for A,B,C,noise : signal level
		uint8_t vol;		// volume
		float lim;		// period
		uint32_t bgn;		// 1st tick (for tickless snd)
		uint32_t pos,cur;	// for envelope; noise.cur = nvalue
};

class AYProc {
	public:
		AYProc(int32_t);
		int32_t type;
		int32_t stereo;
		uint8_t reg[16];
		AYChan a,b,c,e,n;
		int32_t freq;
		uint8_t curreg;
		float aycoe;
		void reset();
		void setreg(uint8_t);
		void settype(int32_t);
		SndData getvol();
		void calculate();
};

struct Sound {
	public:
		Sound();
		uint8_t sndbuf[4000];
		uint8_t* sbptr;
		std::vector<OutSys> outsyslist;
		OutSys *outsys;
		void addoutsys(std::string, bool(*)(),void(*)(),void(*)());
		void setoutptr(std::string);
		AYProc* sc1;	// 1st chip
		AYProc* sc2;	// 2nd chip
		AYProc* scc;	// selected chip
		bool enabled;
		bool mute;
		int32_t tstype;
		int32_t rate;
		int32_t chans;
		int32_t bufsize,chunks;
		bool beeplev;
		uint32_t t;
		uint8_t lev,levr,levl;
		uint8_t beepvol,tapevol,ayvol,gsvol;
		uint32_t tatbyte;
#if HAVESDLSOUND
		Mix_Chunk *ch;
#endif
#ifndef WIN32
		char *device;		// alsa
		snd_output_t *output;
		snd_pcm_t *ahandle;
		snd_pcm_sframes_t frames;
		int32_t audio;					// oss
		int32_t sndformat;
#else
		HWAVEOUT hwaveout;
		WAVEFORMATEX wfx;
		WAVEHDR whdr;
		IDirectSound *ds;
		IDirectSoundBuffer *dsbuf;
		DSBUFFERDESC dsbdsc;
#endif
		void sync();
		void defpars();
};

extern Sound *snd;

#endif
