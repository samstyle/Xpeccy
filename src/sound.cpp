#include "sound.h"
#include "xcore/xcore.h"
#include <stdio.h>

#include <iostream>
#ifdef HAVESDL
	#include <SDL.h>
	#undef main
#endif

#ifdef _WIN32
	#include <mmsystem.h>
#endif

bool sndEnabled;
bool sndMute;

#define SF_BUF	1
int sndFlag = 0;

unsigned char sndBufA[0x4000];
unsigned char sndBufB[0x4000];
unsigned char* sndBuf = sndBufA;
int ringPos = 0;
int playPos = 0;
int pass = 0;

int smpCount = 0;

int beepVolume = 100;
int tapeVolume = 100;
int ayVolume = 100;
int gsVolume = 100;

OutSys *sndOutput = NULL;
unsigned int sndRate = 44100;
int sndChans = 2;
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

void sndSync(int fast) {
	tapSync(zx->tape,zx->tapCount);
	zx->tapCount = 0;
	gsSync(zx->gs);
	tsSync(zx->ts,nsPerSample);
	if (fast != 0) return;
	if (sndOutput == NULL) return;
	lev = zx->beeplev ? beepVolume : 0;
	if (zx->tape->flag & TAPE_ON) {
		lev += (zx->tape->outsig ? tapeVolume : 0) + (zx->tape->signal ? tapeVolume : 0);
	}

	lev *= 0.16;
	levl = lev;
	levr = lev;

	tsPair tsvol = tsGetVolume(zx->ts);
	levl += tsvol.left * ayVolume / 100.0;
	levr += tsvol.right * ayVolume / 100.0;

	gsPair gsvol = gsGetVolume(zx->gs);
	levl += gsvol.left * gsVolume / 100.0;
	levr += gsvol.right * gsVolume / 100.0;

	sdrvPair sdvol = sdrvGetVolume(zx->sdrv);
	levl += sdvol.left * beepVolume / 100.0;
	levr += sdvol.right * beepVolume / 100.0;

	if (levl > 0xff) levl = 0xff;
	if (levr > 0xff) levr = 0xff;

	if (smpCount >= sndChunks) return;		// don't overfill buffer!

	lastL = levl & 0xff;
	lastR = levr & 0xff;
	sndBuf[ringPos] = lastL;
	ringPos++;
	sndBuf[ringPos] = lastR;
	ringPos++;
	ringPos &= 0x3fff;
	smpCount++;
//	return tk;
}

void sndFillToEnd() {
	while (smpCount < sndChunks) {
		sndBuf[ringPos] = lastL;
		ringPos++;
		sndBuf[ringPos] = lastR;
		ringPos++;
		ringPos &= 0x3fff;
		smpCount++;
	}
}

void sndCalibrate() {
	sndChunks = (int)(sndRate / 50.0);			// samples played at 1/50 sec			882
	sndBufSize = sndChans * sndChunks;			// buffer size for 1/50 sec play		1764
	nsPerSample = zx->nsPerFrame / sndChunks;
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
#ifdef HAVESDL
	if (sndOutput == NULL) return;
	if (strcmp(sndOutput->name,"SDL") != 0) return;
	SDL_PauseAudio(b ? 1 : 0);
#endif
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
	for (int bufPos=0; bufPos < len; bufPos++) {
		sndBufB[bufPos] = sndBufA[playPos];
		playPos++;
		playPos &= 0x3fff;
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

// FIXME: something going wrong. sdlPlayAudio plays buffer slower than emulation fill it
void sdlPlayAudio(void*,Uint8* stream, int len) {
	if (pass < 2) {
		pass++;
	} else {
		int diff;
		if (playPos < ringPos) {
			diff = ringPos - playPos;
		} else {
			diff = 0x4000 - (ringPos - playPos);
		}
		if (diff >= len) fillBuffer(len);
	}
	memcpy(stream,sndBufB,len);
}

bool sdlopen() {
//	printf("Open SDL audio device...");
	SDL_AudioSpec* asp = new SDL_AudioSpec;
	asp->freq = sndRate;
	asp->format = AUDIO_U8;
	asp->channels = sndChans;
	asp->samples = sndChunks + 1;
	asp->callback = &sdlPlayAudio;
	asp->userdata = NULL;
	if (SDL_OpenAudio(asp,NULL) < 0) {
		printf("SDL audio device opening...failed\n");
		return false;
	}
	SDL_PauseAudio(0);
//	printf("OK\n");
	return true;
}

void sdlplay() {
}

void sdlclose() {
//	printf("Close SDL audio device\n");
	SDL_PauseAudio(1);
	SDL_CloseAudio();
}

#endif

#ifdef __linux__

bool oss_open() {
//	printf("Open OSS audio device\n");
	ossHandle = open("/dev/dsp",O_WRONLY,0777);
	if (ossHandle < 0) return false;
	ioctl(ossHandle,SNDCTL_DSP_SETFMT,&sndFormat);
	ioctl(ossHandle,SNDCTL_DSP_CHANNELS,&sndChans);
	ioctl(ossHandle,SNDCTL_DSP_SPEED,&sndRate);
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
	err = snd_pcm_open(&alsaHandle,"default",SND_PCM_STREAM_PLAYBACK,0);
	if ((err < 0) || (alsaHandle == NULL)) {
		alsaHandle = NULL;
		res = false;
	} else {
		err = snd_pcm_set_params(alsaHandle,SND_PCM_FORMAT_U8,SND_PCM_ACCESS_RW_INTERLEAVED,sndChans,sndRate,1,10000);
		if (err < 0) {
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
		ptr += res * sndChans;
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
	wf.nChannels = sndChans;
	wf.nSamplesPerSec = sndRate;
	wf.wBitsPerSample = 8;
	wf.nBlockAlign = (sndChans * wf.wBitsPerSample) >> 3;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
	wf.cbSize = 0;

	whdr.lpData = (LPSTR)ringBuffer;
	whdr.dwBufferLength = sndBufSize;
	whdr.dwBytesRecorded = 0;
	whdr.dwUser = 0;
	whdr.dwLoops = 0;
	whdr.lpNext = NULL;
	whdr.reserved = 0;

	event = CreateEvent(0, FALSE, FALSE, 0);
//	MMRESULT res = waveOutOpen(&wout,WAVE_MAPPER,&wf,NULL,NULL,CALLBACK_NULL);
	MMRESULT res = waveOutOpen(&wout,WAVE_MAPPER,&wf,DWORD_PTR(event),0,CALLBACK_EVENT | WAVE_FORMAT_DIRECT);
	return (res == MMSYSERR_NOERROR);
}

void wave_play() {
	whdr.dwFlags = 0;
	whdr.dwBufferLength = ringPos - 1;
	waveOutPrepareHeader(wout,&whdr,sizeof(WAVEHDR));
	waveOutWrite(wout,&whdr,sizeof(WAVEHDR));
	while (!(whdr.dwFlags & WHDR_DONE)) {
		WaitForSingleObject(event, INFINITE);
	}
	waveOutUnprepareHeader(wout,&whdr,sizeof(WAVEHDR));
	ringPos = 0;
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
//	{SND_OSS,"OSS",&oss_open,&oss_play,&oss_close},
#ifdef HAVEALSA
//	{SND_ALSA,"ALSA",&alsa_open,&alsa_play,&alsa_close},
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
	sndRate = 44100;
	sndChans = 2;
	sndEnabled = true;
	sndMute = true;
	sndOutput = NULL;
	beepVolume = tapeVolume = ayVolume = gsVolume = 100;
	initNoise();							// ay/ym
}
