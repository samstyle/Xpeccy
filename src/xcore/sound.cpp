#include <stdio.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include "sound.h"
#include "xcore.h"

#include <iostream>
#include <QMutex>
#include <QWaitCondition>

#include <SDL.h>

// new
static unsigned char sbuf[0x4000];
static int posf = 0x1000;			// fill pos
static int posp = 0x0004;			// play pos

static int smpCount = 0;
OutSys *sndOutput = NULL;
static int sndChunks = 882;

#define DISCRATE 32
int nsPerSample = 22675;
// static int disCount = 0;
static sndPair tmpLev = {0, 0};
static sndPair sndLev;

OutSys* findOutSys(const char*);

static long double H[DISCRATE] = {0};

static int sb_pos = 0;
static int sp_pos = 0;
static sndPair smpBuf[128] = {{0,0}};

#if defined(HAVESDL2)
static SDL_AudioDeviceID sdldevid;
#endif

// output

#define USEKIH 0

// return 1 when buffer is full
// NOTE: need sync|flush devices if debug
int sndSync(Computer* comp) {
	if (!conf.emu.pause || comp->debug) {
		if (comp->hw->grp == HWG_ZX)
			gsFlush(comp->gs);
//		saaFlush(comp->saa);
		if (!conf.emu.fast && !conf.emu.pause) {
			sndLev = comp->hw->vol(comp, &conf.snd.vol);
			sndLev.left = sndLev.left * conf.snd.vol.master / 100;
			sndLev.right = sndLev.right * conf.snd.vol.master / 100;
			if (sndLev.left > 0x7fff) sndLev.left = 0x7fff;
			if (sndLev.right > 0x7fff) sndLev.right = 0x7fff;

			smpBuf[sb_pos & 127] = sndLev;
			sb_pos++;
			if ((sb_pos % DISCRATE) == 0) {
				tmpLev.left = 0;
				tmpLev.right = 0;
#if USEKIH
				sp_pos = sb_pos - DISCRATE;
				for (int i = 0; i < DISCRATE; i++) {
					tmpLev.left += smpBuf[sp_pos & 127].left * H[i];
					tmpLev.right += smpBuf[sp_pos & 127].right * H[i];
					sp_pos++;
				}
#else
				sp_pos = sb_pos - DISCRATE;
				for (int i = 0; i < DISCRATE; i++) {
					tmpLev.left += smpBuf[sp_pos & 127].left;
					tmpLev.right += smpBuf[sp_pos & 127].right;
					sp_pos++;
				}
				tmpLev.left /= DISCRATE;
				tmpLev.right /= DISCRATE;
#endif
				sndLev = tmpLev;
				tmpLev.left = 0;
				tmpLev.right = 0;
//				disCount = 0;

				if (!conf.snd.enabled) {
					sndLev.left = 0;
					sndLev.right = 0;
				}

				sbuf[posf & 0x3fff] = sndLev.left & 0xff;
				posf++;
				sbuf[posf & 0x3fff] = (sndLev.left >> 8) & 0xff;
				posf++;
				sbuf[posf & 0x3fff] = sndLev.right & 0xff;
				posf++;
				sbuf[posf & 0x3fff] = (sndLev.right >> 8) & 0xff;
				posf++;
			}
			smpCount++;
		}
	}
	if (smpCount < sndChunks) return 0;
	conf.snd.fill = 0;
	smpCount = 0;
	return 1;
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
	nsPerSample = 1e9 / conf.snd.rate / DISCRATE;
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

static SDL_TimerID tid;
#if USEMUTEX
extern QMutex emutex;
extern QWaitCondition qwc;
#else
extern int sleepy;
#endif

Uint32 sdl_timer_callback(Uint32 iv, void* ptr) {
#if USEMUTEX
	qwc.wakeAll();
#else
	sleepy = 0;
#endif
	return iv;
}

// NULL

int null_open() {
	printf("NULL device opening...\n");
	tid = SDL_AddTimer(20, sdl_timer_callback, NULL);
#ifdef HAVESDL1
	if (tid == NULL) {
#else
	if (tid < 0) {
#endif
		printf("Can't create SDL_Timer, syncronisation unavailable\n");
		throw(0);
	}
	sndChunks = conf.snd.rate / 50 * DISCRATE;
	return 1;
}

void null_play() {}
void null_close() {
	SDL_RemoveTimer(tid);
}

// SDL

#include <QDebug>

void sdlPlayAudio(void*, Uint8* stream, int len) {
//	printf("len = %i\n",len);
	int dist = posf - posp;
	while (dist < 0) dist += 0x4000;
	while (dist > 0x3fff) dist -= 0x4000;
	if ((dist < len) || conf.emu.fast || conf.emu.pause) {				// overfill : fill with last sample of previous buf
//		printf("overfill : %i %i\n", posf, posp);
		while(len > 0) {
			*(stream++) = sbuf[(posp - 4) & 0x3fff];
			*(stream++) = sbuf[(posp - 3) & 0x3fff];
			*(stream++) = sbuf[(posp - 2) & 0x3fff];
			*(stream++) = sbuf[(posp - 1) & 0x3fff];
			len -= 4;
		}
	} else {						// normal : play buffer
		while(len > 0) {
			*(stream++) = sbuf[posp & 0x3fff];
			posp++;
			len--;
		}
	}
#if USEMUTEX
	qwc.wakeAll();
#else
	sleepy = 0;
#endif
}

int sdlopen() {
	int res;
	SDL_AudioSpec asp;
	SDL_AudioSpec dsp;
	asp.freq = conf.snd.rate;
	asp.format = AUDIO_S16LSB;
	asp.channels = conf.snd.chans;
	asp.samples = conf.snd.rate / 50;
	asp.callback = &sdlPlayAudio;
	asp.userdata = NULL;
#if defined(HAVESDL2)
	sdldevid = SDL_OpenAudioDevice(NULL, 0, &asp, &dsp, 0);
	if (sdldevid == 0) {
#else
	res = SDL_OpenAudio(&asp, &dsp);
	if (res != 0) {
#endif
		printf("SDL audio device opening...failed (%s)\n", SDL_GetError());
		res = 0;
	} else {
		printf("SDL audio device opening...success: %i %i (%i / %i)\n",dsp.freq, dsp.samples,dsp.format,AUDIO_S16LSB);
		sndChunks = dsp.samples * DISCRATE;
#if defined(HAVESDL2)
		SDL_PauseAudioDevice(sdldevid, 0);
#else
		SDL_PauseAudio(0);
#endif
		res = 1;
	}
	posp = 0x0004;
	posf = 0x1000;
	memset(sbuf, 0x00, 0x4000);
	return res;
}

void sdlplay() {
}

void sdlclose() {
#if defined(HAVESDL2)
	SDL_CloseAudioDevice(sdldevid);
#else
	SDL_CloseAudio();
#endif
}

// init

OutSys sndTab[] = {
	{xOutputNone,"NULL",&null_open,&null_play,&null_close},
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

void init_kih() {
	long double Fd = conf.snd.rate * DISCRATE;
	long double Fs = 20;
	long double Fx = 40;
	long double H_id [DISCRATE] = {0};
	long double W[DISCRATE] = {0};
	long double Fc = (Fs + Fx) / (2 * Fd);
	int i;
	for (i = 0; i < DISCRATE; i++) {
		if (i == 0) {
			H_id[i] = 2 * M_PI * Fc;
		} else {
			H_id[i] = sinl(2 * M_PI * Fc * i) / (M_PI * i);
		}
		W[i] = 0.42 + 0.5 * cosl((2 * M_PI * i) / (DISCRATE - 1)) + 0.08 * cosl((4 * M_PI * i) / (DISCRATE - 1));
		H[i] = H_id[i] * W[i];
	}
	double sum = 0;
	for (i = 0; i < DISCRATE; i++) sum += H[i];
	for (i = 0; i < DISCRATE; i++) H[i] /= sum;
}

void sndInit() {
	conf.snd.rate = 44100;
	conf.snd.chans = 2;
	conf.snd.enabled = 1;
	sndOutput = NULL;
	conf.snd.vol.beep = 100;
	conf.snd.vol.tape = 100;
	conf.snd.vol.ay = 100;
	conf.snd.vol.gs = 100;
	conf.snd.wavout = 0;
	conf.snd.wavfile = NULL;
	initNoise();
	init_kih();
}

// output to wav

wavHead wav_prepare(unsigned int rate, unsigned short chans) {
	wavHead hd;
	memcpy(hd.chunkId, "RIFF", 4);
	hd.chunkSize = 0;
	memcpy(hd.format, "WAVE", 4);
	memcpy(hd.subchunk1Id, "fmt ", 4);
	hd.subchunk1Size = 16;
	hd.audioFormat = 1;
	hd.numChannels = chans;
	hd.sampleRate = rate;
	hd.byteRate = rate * chans;
	hd.blockAlign = chans;
	hd.bitsPerSample = 8;
	memcpy(hd.subchunk2Id, "data", 4);
	hd.subchunk2Size = 0;				// later
	return hd;
}

void snd_wav_close() {
	if (conf.snd.wavfile) {
		int sz = ftell(conf.snd.wavfile);			// file size
		fseek(conf.snd.wavfile, 4, SEEK_SET);
		fputi(sz - 8, conf.snd.wavfile);
		fseek(conf.snd.wavfile, sizeof(wavHead) - 4, SEEK_SET);
		fputi(sz - sizeof(wavHead), conf.snd.wavfile);
		fclose(conf.snd.wavfile);
		conf.snd.wavfile = NULL;
		conf.snd.wavout = 0;
	}
}

int snd_wav_open(const char* path) {
	int res = ERR_OK;
	wavHead hd = wav_prepare(44100, 2);
	snd_wav_close();
	conf.snd.wavfile = fopen(path, "wb");
	if (conf.snd.wavfile) {
		fwrite(&hd, sizeof(wavHead), 1, conf.snd.wavfile);
		conf.snd.wavout = 1;
	} else {
		res = ERR_CANT_OPEN;
	}
	return res;
}

void snd_wav_write() {
	if (conf.snd.wavfile) {
		fputc(sndLev.left >> 8, conf.snd.wavfile);
		fputc(sndLev.right >> 8, conf.snd.wavfile);
	}
}

// debug

void sndDebug() {
	printf("%i - %i = %i\n",posf, posp, posf - posp);
}
