#include <stdio.h>

#include "sound.h"
#include "xcore/xcore.h"

#include <iostream>
#ifdef HAVESDL
	#include <SDL.h>
	#undef main
#endif

#ifdef _WIN32
	#include <mmsystem.h>
#endif

#define SF_BUF	1
int sndFlag = 0;

unsigned char sndBufA[0x10000];
unsigned char sndBufB[0x10000];
unsigned char* sndBuf = sndBufA;
unsigned short ringPos = 0;
unsigned short playPos = 0;
int pass = 0;

int smpCount = 0;

long nsPerFrame;

OutSys *sndOutput = NULL;
int sndChunks = 882;
int sndBufSize = 1764;
int nsPerSample = 23143;
int lev,levr,levl;
unsigned char lastL,lastR;

OutSys* findOutSys(const char*);

#ifdef __linux__
	int32_t ossHandle;			// oss
	int32_t sndFormat;
#ifdef HAVEALSA
	snd_pcm_t* alsaHandle = NULL;
#endif
#elif _WIN32
	WAVEFORMATEX wf;
	WAVEHDR whdr;
	HWAVEOUT wout;
	HANDLE event;
#endif

// output

// return 1 when buffer is full
int sndSync(ZXComp* comp, int fast) {
	tapSync(comp->tape,comp->tapCount);
	comp->tapCount = 0;
	gsSync(comp->gs);
	tsSync(comp->ts,nsPerSample);
	saaSync(comp->saa,nsPerSample);
	if (!fast && (sndOutput != NULL)) {
		lev = comp->beeplev ? conf.snd.vol.beep : 0;
		if (comp->tape->levRec) lev += conf.snd.vol.tape;
		if (comp->tape->on && comp->tape->levPlay) {
			lev += conf.snd.vol.tape;
		}
		lev *= 0.16;
		levl = lev;
		levr = lev;

		sndPair svol = tsGetVolume(comp->ts);
		levl += svol.left * conf.snd.vol.ay / 100.0;
		levr += svol.right * conf.snd.vol.ay / 100.0;

		svol = gsGetVolume(comp->gs);
		levl += svol.left * conf.snd.vol.gs / 100.0;
		levr += svol.right * conf.snd.vol.gs / 100.0;

		svol = sdrvGetVolume(comp->sdrv);
		levl += svol.left * conf.snd.vol.beep / 100.0;
		levr += svol.right * conf.snd.vol.beep / 100.0;

		svol = saaGetVolume(comp->saa);		// TODO : saa volume control
		levl += svol.left;
		levr += svol.right;

		if (levl > 0xff) levl = 0xff;
		if (levr > 0xff) levr = 0xff;

		lastL = levl & 0xff;
		lastR = levr & 0xff;
	}

//	if (smpCount >= sndChunks) return;		// don't overfill buffer!

	sndBuf[ringPos++] = lastL;
	sndBuf[ringPos++] = lastR;
	smpCount++;

	return (smpCount < sndChunks) ? 0 : 1;
}

void sndFillToEnd() {
	while (smpCount < sndChunks) {
		sndBuf[ringPos++] = lastL;
		sndBuf[ringPos++] = lastR;
		smpCount++;
	}
}

void sndCalibrate() {
	sndChunks = conf.snd.rate / 50;			// samples played at 1/50 sec			882
	sndBufSize = conf.snd.chans * sndChunks;	// buffer size for 1/50 sec play		1764
	// nsPerSample = nsPerFrame / sndChunks;
	nsPerSample = 1e9 / conf.snd.rate;
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
	}
	if (!sndOutput->open()) {
		printf("Can't open sound system '%s'. Reset to NULL\n",name);
		setOutput("NULL");
	}
	sndCalibrate();
}

bool sndOpen() {
	if (sndOutput == NULL) return true;
	if (!sndOutput->open()) setOutput("NULL");
	return true;
}

void sndPlay() {
	sndFillToEnd();
	if (sndOutput != NULL) {
		sndOutput->play();
	}
	smpCount = 0;
}

void sndPause(bool b) {
	smpCount = 0;
	sndFillToEnd();
}

void sndClose() {
	if (sndOutput != NULL) sndOutput->close();
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
		sndBufB[pos++] = sndBufA[playPos++];
	}
	sndBuf = sndBufA;
}

void switchSndBuf() {
	sndBuf = (sndFlag & SF_BUF) ? sndBufA : sndBufB;
	sndFlag ^= SF_BUF;
	ringPos = 0;
}

bool null_open() {return true;}
void null_play() {}
void null_close() {}

#ifdef HAVESDL

void sdlPlayAudio(void*,Uint8* stream, int len) {
	if (pass < 2) {
		pass++;
	} else {
		int diff = ringPos - playPos;
		if (diff < 0) {
			diff = 0x10000 - diff;
		}
		if (diff >= len) fillBuffer(len);
	}
	SDL_MixAudio(stream, sndBufB, len, SDL_MIX_MAXVOLUME);
	// memcpy(stream,sndBufB,len);
}

bool sdlopen() {
//	printf("Open SDL audio device...");
	SDL_AudioSpec asp;
	asp.freq = conf.snd.rate;
	asp.format = AUDIO_U8;
	asp.channels = 2;
	asp.samples = sndChunks + 1;
	asp.callback = &sdlPlayAudio;
	asp.userdata = NULL;
	if (SDL_OpenAudio(&asp, NULL) != 0) {
		printf("SDL audio device opening...failed\n");
		return false;
	}
	SDL_PauseAudio(0);
	return true;
}

void sdlplay() {
}

void sdlclose() {
	SDL_CloseAudio();
}

#endif

#ifdef __linux__

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
//	fillBuffer(sndBufSize);
	unsigned char* ptr = sndBuf;
	int fsz = sndBufSize;	// smpCount * sndChans;
	int res;
	while (fsz > 0) {
		res = write(ossHandle,ptr,fsz);
		ptr += res;
		fsz -= res;
	}
	switchSndBuf();
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
	if ((err < 0) || (alsaHandle == NULL)) {
		alsaHandle = NULL;
		res = false;
	} else {
		err = snd_pcm_set_params(alsaHandle,SND_PCM_FORMAT_U8,SND_PCM_ACCESS_RW_INTERLEAVED,conf.snd.chans,conf.snd.rate,1,100000);
		if (err != 0) {
			alsaHandle = NULL;
			res = false;
		}
	}
	return res;
}

void alsa_play() {
	if (alsaHandle == NULL) return;
	snd_pcm_sframes_t res;
//	fillBuffer(sndBufSize);
	unsigned char* ptr = sndBuf;
	int fsz = smpCount;
	while (fsz > 0) {
		res = snd_pcm_writei(alsaHandle, ptr, fsz);
		if (res < 0) res = snd_pcm_recover(alsaHandle, res, 1);
		if (res < 0) break;
		fsz -= res;
		ptr += res * conf.snd.chans;
	}
	switchSndBuf();
}

void alsa_close() {
	if (alsaHandle == NULL) return;
	snd_pcm_close(alsaHandle);
}

#endif

#elif _WIN32

// TODO: Windows sound output would be here... someday

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
	whdr.lpData = (LPSTR)sndBuf;
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

#endif

// init

OutSys sndTab[] = {
	{SND_NULL,"NULL",&null_open,&null_play,&null_close},
#ifdef __linux__
	{SND_OSS,"OSS",&oss_open,&oss_play,&oss_close},
#ifdef HAVEALSA
	{SND_ALSA,"ALSA",&alsa_open,&alsa_play,&alsa_close},
#endif
#elif _WIN32
//	{SND_WAVE,"WaveOut",&wave_open,&wave_play,&wave_close},
#endif
#ifdef HAVESDL
	{SND_SDL,"SDL",&sdlopen,&sdlplay,&sdlclose},
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
#ifdef __linux__
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
}
