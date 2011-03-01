#ifndef _SOUND_H
#define _SOUND_H

#define HAVESDLSOUND 0

#include <vector>
#include <string>
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

struct SndData {
	int r;
	int l;
	friend SndData operator +(SndData p1, SndData p2) {p1.r += p2.r; p1.l += p2.l; return p1;}
	friend SndData operator *(SndData p1, float mul) {p1.r *= mul; p1.l *= mul; return p1;}
};

struct OutSys {
	std::string name;
	bool (*open)();
	void (*play)();
	void (*close)();
};

struct AYChan {
	AYChan();
	bool lev;		// for A,B,C,noise : signal level
	unsigned char vol;	// volume
	float lim;		// period
	unsigned int bgn;	// 1st tick (for tickless snd)
	unsigned int pos,cur;	// for envelope; noise.cur = nvalue
};

struct AYProc {
	AYProc(int);
	int type;
	int stereo;
	unsigned char reg[16];
	AYChan a,b,c,e,n;
	int freq;
	unsigned char curreg;
	float aycoe;
	void reset();
	void setreg(unsigned char);
	void settype(int);
	SndData getvol();
	void calculate();
};

struct Sound {
	Sound();
	unsigned char sndbuf[4000];
	unsigned char* sbptr;
	std::vector<OutSys> outsyslist;
	OutSys *outsys;
	void addoutsys(std::string, bool(*)(),void(*)(),void(*)());
	void setoutptr(std::string);
	AYProc* sc1;	// 1st chip
	AYProc* sc2;	// 2nd chip
	AYProc* scc;	// selected chip
	bool enabled;
	bool mute;
	int tstype;
	int rate;
	int chans;
	int bufsize,chunks;
	bool beeplev;
	unsigned int t;
	unsigned char lev,levr,levl;
	unsigned char beepvol,tapevol,ayvol,gsvol;
	unsigned int tatbyte;
//	int tcnt;
#if HAVESDLSOUND
	Mix_Chunk *ch;
#endif
#ifndef WIN32
	char *device;		// alsa
	snd_output_t *output;
	snd_pcm_t *ahandle;
	snd_pcm_sframes_t frames;
	int audio;					// oss
	int sndformat;
#else
	HWAVEOUT hwaveout;
	WAVEFORMATEX wfx;
	WAVEHDR whdr;

	IDirectSound *ds;
	IDirectSoundBuffer *dsbuf;
	DSBUFFERDESC dsbdsc;
#endif
	void sync();
//	void tick();
	void defpars();
};

extern Sound *snd;

#endif
