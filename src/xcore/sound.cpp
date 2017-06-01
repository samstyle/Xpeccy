#include <stdio.h>

#include "sound.h"
#include "xcore.h"

#include <iostream>
#include <QMutex>

#include <SDL.h>
#undef main

// extern QMutex emutex;		// unlock to start emulation cycle

typedef struct {
	unsigned char data[0x1000];
	unsigned pos:12;
} sndBuffa;

sndBuffa bufA;		// ring buffer @ real freq
sndBuffa bufB;		// 1 frame buffer for output

unsigned short playPos = 0;
int pass = 0;

int smpCount = 0;

OutSys *sndOutput = NULL;
int sndChunks = 882;
int sndBufSize = 1764;
int nsPerSample = 23143;

sndPair sndLev;
// sndPair sndLast;

OutSys* findOutSys(const char*);

#ifdef __linux
	int32_t ossHandle;			// oss
	int32_t sndFormat;
#if defined(HAVEALSA)
	snd_pcm_t* alsaHandle = NULL;
#endif

#elif _WIN32
	WAVEFORMATEX wf;
	WAVEHDR whdr;
	HWAVEOUT wout;
	HANDLE event;
#endif

// output

#include "../libxpeccy/hardware/hardware.h"

sndPair mixer(sndPair cur, int levL, int levR, int vol) {
	levL = levL * vol / 100.0;
	levR = levR * vol / 100.0;
	double fl = cur.left / 256.0;
	double fr = cur.right / 256.0;
	double sl = levL / 256.0;
	double sr = levR / 256.0;
	double xl = (fl + sl) / (1.0 + fl * sl);
	double xr = (fr + sr) / (1.0 + fr * sr);
	cur.left = xl * 256;
	cur.right = xr * 256;
	return cur;
}

void sndMix(Computer* comp) {
	int lev = 0;
	sndPair svol;
	sndLev.left = 0;
	sndLev.right = 0;
	// tape
	if (comp->tape->on) {
		if (comp->tape->rec) {
			lev = comp->tape->levRec ? 0x7f : 0x00;
		} else {
			lev = comp->tape->levPlay ? 0x7f : 0x00;
		}
	}
//	lev = (comp->tape->on && (comp->tape->levRec || comp->tape->levPlay)) ? 0x7f : 0x00;
	sndLev = mixer(sndLev, lev, lev, conf.snd.vol.tape);
	// beeper
	bcSync(comp->beep, -1);
	lev = comp->beep->val;
	sndLev = mixer(sndLev, lev, lev, conf.snd.vol.beep);
	// turbosound
	svol = tsGetVolume(comp->ts);
	sndLev = mixer(sndLev, svol.left, svol.right, conf.snd.vol.ay);
	// gameboy
	svol = gbsVolume(comp->gbsnd);
	sndLev = mixer(sndLev, svol.left, svol.right, 100);
	// general sound
	//svol = gsGetVolume(comp->gs);
	//sndLev = mixer(sndLev, svol.left, svol.right, conf.snd.vol.gs);
	// soundrive
//	svol = sdrvGetVolume(comp->sdrv);
//	sndLev = mixer(sndLev, svol.left, svol.right, conf.snd.vol.beep);
	// saa
	svol = saaGetVolume(comp->saa);		// TODO : saa volume control
	sndLev = mixer(sndLev, svol.left, svol.right, 100);
	// DEVICES
	int idx = 0;
	xDevice* dev;
	int vol = 0;
	while ((idx < MAX_DEV_COUNT) && comp->devList[idx]) {
		dev = comp->devList[idx];
		if (dev->vol) {
			switch(dev->type) {
				case DEV_SDRIVE: vol = 100; break;
				case DEV_GSOUND: vol = conf.snd.vol.gs; break;
				default: vol = 0; break;
			}
			svol = dev->vol(dev->ptr.ptr);
			sndLev = mixer(sndLev, svol.left, svol.right, vol);
		}
		idx++;
	}
	// cut
	if (sndLev.left > 0xff) sndLev.left = 0xff;
	if (sndLev.right > 0xff) sndLev.right = 0xff;
}

// return 1 when buffer is full
int sndSync(Computer* comp, int nosync, int fast) {
	if (!nosync) {
		tapSync(comp->tape,comp->tapCount);
		comp->tapCount = 0;
		//gsSync(comp->gs);
		tsSync(comp->ts,nsPerSample);
		saaSync(comp->saa,nsPerSample);

		compDevFlush(comp);
	}
	if (!nosync && !fast) {
		sndMix(comp);
	}

	bufA.data[bufA.pos++] = sndLev.left >> 2;
	bufA.data[bufA.pos++] = sndLev.right >> 2;
	bufA.pos &= 0xfff;

	smpCount++;
	if (smpCount < sndChunks) return 0;

//	memcpy(bufB.data, bufA.data, sndBufSize);
//	bufA.pos = 0;
	conf.snd.fill = 0;
	smpCount = 0;
	return 1;
}

void sndCalibrate(Computer* comp) {
	// sndChunks = conf.snd.rate / 50;		// samples / frame
	sndBufSize = conf.snd.chans * sndChunks;	// buffer size
	nsPerSample = 1e9 / conf.snd.rate;		// ns / sample
#ifdef ISDEBUG
	printf("snd.rate = %i\n",conf.snd.rate);
	printf("sndChunks = %i\n",sndChunks);
	printf("sndBufSize = %i\n",sndBufSize);
	printf("nsPerSample = %i\n",nsPerSample);
#endif
}

std::string sndGetOutputName() {
	std::string res = "NULL";
	if (sndOutput != NULL) {
		res = sndOutput->name;
	}
	return res;
}

void setOutput(const char* name) {
	if (sndOutput != NULL) {
		sndOutput->close();
	}
	sndOutput = findOutSys(name);
	if (sndOutput == NULL) {
		printf("Can't find sound system '%s'. Reset to NULL\n",name);
		setOutput("NULL");
	} else if (!sndOutput->open()) {
		printf("Can't open sound system '%s'. Reset to NULL\n",name);
		setOutput("NULL");
	}
	// sndCalibrate();
}

int sndOpen() {
	sndChunks = conf.snd.rate / 50;
	if (sndOutput == NULL) return 0;
	if (!sndOutput->open()) {
		setOutput("NULL");
	}
	bufA.pos = 0;
	bufB.pos = 0;
	playPos = 0;
	pass = 0;
	return 1;
}

void sndPlay() {
	if (sndOutput) {
		sndOutput->play();
	}
	// playPos = (bufA.pos - sndBufSize) & 0xffff;
}

void sndClose() {
	if (sndOutput != NULL)
		sndOutput->close();
}

std::string sndGetName() {
	std::string res = "NULL";
	if (sndOutput != NULL) {
		res = sndOutput->name;
	}
	return res;
}

//------------------------
// Sound output
//------------------------

void fillBuffer(int len) {
	int pos = 0;
	while (pos < len) {
		bufB.data[pos++] = bufA.data[playPos++];
		playPos &= 0xfff;
	}
}

// NULL

int null_open() {
	return 1;
}

void null_play() {}
void null_close() {}

// SDL

void sdlPlayAudio(void*,Uint8* stream, int len) {
	if (pass > 3) {
		int diff = bufA.pos - playPos;
		if (diff < 0) {
			diff += 0x1000;
		}
		if (diff >= len) {
			fillBuffer(len);
		}
	} else {
		pass++;
	}
	// SDL_MixAudio(stream, bufB.data, len, SDL_MIX_MAXVOLUME);
	memcpy(stream, bufB.data, len);
}

int sdlopen() {
//	printf("Open SDL audio device...");
	int res;
	SDL_AudioSpec asp;
	SDL_AudioSpec dsp;
	asp.freq = conf.snd.rate;
	asp.format = AUDIO_U8;
	asp.channels = conf.snd.chans;
	asp.samples = sndChunks;
	asp.callback = &sdlPlayAudio;
	asp.userdata = NULL;
	if (SDL_OpenAudio(&asp, &dsp) != 0) {
		printf("SDL audio device opening...failed\n");
		res = 0;
	} else {
		printf("SDL audio device opening...success: %i %i\n",dsp.freq, dsp.samples);
		bufA.pos = 0;
		playPos = 0;
		pass = 0;
		SDL_PauseAudio(0);
		res = 1;
	}
	return res;
}

void sdlplay() {
}

void sdlclose() {
//	SDL_RemoveTimer(tid);
	SDL_CloseAudio();
}

/*
 *
#ifdef __linux

bool oss_open() {
//	printf("Open OSS audio device\n");
	ossHandle = open("/dev/dsp",O_WRONLY,0777);
	if (ossHandle < 0) return false;
	ioctl(ossHandle,SNDCTL_DSP_SETFMT,&sndFormat);
	ioctl(ossHandle,SNDCTL_DSP_CHANNELS,&conf.snd.chans);
	ioctl(ossHandle,SNDCTL_DSP_SPEED,&conf.snd.rate);
	return true;
}

void oss_play() {
	if (ossHandle < 0) return;
	fillBuffer(sndBufSize);
	unsigned char* ptr = bufB.data;	//sndBufB;
	int fsz = sndBufSize;	// smpCount * sndChans;
	int res;
	while (fsz > 0) {
		res = write(ossHandle,ptr,fsz);
		ptr += res;
		fsz -= res;
	}
	// switchSndBuf();
	// ringPos = 0;
}

void oss_close() {
	if (ossHandle < 0) return;
//	printf("Close OSS audio device\n");
	close(ossHandle);
}


#ifdef HAVEALSA

bool alsa_open() {
	int err;
	bool res = true;
	err = snd_pcm_open(&alsaHandle,"default",SND_PCM_STREAM_PLAYBACK,SND_PCM_NONBLOCK);
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		alsaHandle = NULL;
		res = false;
	} else {
		err = snd_pcm_set_params(alsaHandle,SND_PCM_FORMAT_U8,SND_PCM_ACCESS_RW_INTERLEAVED,conf.snd.chans,conf.snd.rate,1,100000);
		if (err < 0) {
			printf("Set params error: %s\n", snd_strerror(err));
			alsaHandle = NULL;
			res = false;
		}
	}
	return res;
}

void alsa_play() {
	if (alsaHandle == NULL) return;
	snd_pcm_sframes_t res;
	memcpy(bufB.data, bufA.data, sndChunks * conf.snd.chans);
	bufA.pos = 0;
	unsigned char* ptr = bufB.data;
	int fsz = sndChunks;
	while (fsz > 0) {
		res = snd_pcm_writei(alsaHandle, ptr, fsz);
		if (res < 0) res = snd_pcm_recover(alsaHandle, res, 1);
		if (res < 0) break;
		fsz -= res;
		ptr += res * conf.snd.chans;
	}
}

void alsa_close() {
	if (alsaHandle == NULL) return;
	snd_pcm_close(alsaHandle);
}

#endif

*/

#ifdef _WIN32

// TODO: Windows sound output would be here... someday

/*
bool wave_open() {
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = conf.snd.chans;
	wf.nSamplesPerSec = conf.snd.rate;
	wf.wBitsPerSample = 8;
	wf.nBlockAlign = (conf.snd.chans * wf.wBitsPerSample) >> 3;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
	wf.cbSize = 0;

	whdr.dwBufferLength = sndBufSize;
	whdr.dwBytesRecorded = 0;
	whdr.dwUser = 0;
	whdr.dwLoops = 0;
	whdr.lpNext = NULL;
	whdr.reserved = 0;

//	event = CreateEvent(0, FALSE, FALSE, 0);
	MMRESULT res = waveOutOpen(&wout,WAVE_MAPPER,&wf,NULL,NULL,CALLBACK_NULL);
//	MMRESULT res = waveOutOpen(&wout,WAVE_MAPPER,&wf,DWORD_PTR(event),0,CALLBACK_EVENT | WAVE_FORMAT_DIRECT);
	return (res == MMSYSERR_NOERROR);
}

void wave_play() {
    whdr.lpData = (LPSTR)bufBig.data;
	whdr.dwFlags = 0;
	whdr.dwBufferLength = sndBufSize;
	waveOutPrepareHeader(wout,&whdr,sizeof(WAVEHDR));
	waveOutWrite(wout,&whdr,sizeof(WAVEHDR));
	while (!(whdr.dwFlags & WHDR_DONE)) {
		WaitForSingleObject(event, INFINITE);
	}
	waveOutUnprepareHeader(wout,&whdr,sizeof(WAVEHDR));
	switchSndBuf();
}

void wave_close() {
	waveOutReset(wout);
	waveOutUnprepareHeader(wout,&whdr,sizeof(WAVEHDR));
	waveOutClose(wout);
	CloseHandle(event);
}
*/

#endif

// init

OutSys sndTab[] = {
	{xOutputNone,"NULL",&null_open,&null_play,&null_close},
#ifdef __linux
//	{xOutputOss,"OSS",&oss_open,&oss_play,&oss_close},
#ifdef HAVEALSA
//	{xOutputAlsa,"ALSA",&alsa_open,&alsa_play,&alsa_close},
#endif
#elif _WIN32
//	{xOutputWave,"WaveOut",&wave_open,&wave_play,&wave_close},
#endif
#if defined(HAVESDL1) || defined(HAVESDL2)
	{xOutputSDL,"SDL",&sdlopen,&sdlplay,&sdlclose},
#endif
	{0,NULL,NULL,NULL,NULL}
};

OutSys* findOutSys(const char* name) {
	OutSys* res = NULL;
	int idx = 0;
	while (sndTab[idx].name != NULL) {
		if (strcmp(sndTab[idx].name,name) == 0) {
			res = &sndTab[idx];
			break;
		}
		idx++;
	}
	return res;
}

void sndInit() {
#ifdef __linux
	sndFormat = AFMT_U8;
#endif
	conf.snd.rate = 44100;
	conf.snd.chans = 2;
	conf.snd.enabled = 1;
	conf.snd.mute = 1;
	sndOutput = NULL;
	conf.snd.vol.beep = 100;
	conf.snd.vol.tape = 100;
	conf.snd.vol.ay = 100;
	conf.snd.vol.gs = 100;
	initNoise();							// ay/ym
    bufA.pos = 0;
    bufB.pos = 0;
//    bufBig.pos = 0;
}

// debug

void sndDbg() {
	int adr = playPos;
	for (int i = 0; i < sndBufSize; i++) {
		if ((i & 0x1f) == 0) {
			printf("\n%.4X : ",adr & 0xffff);
		}
		printf("%.2X ",bufA.data[adr & 0xffff]);
		adr++;
	}
	printf("\n");
}
