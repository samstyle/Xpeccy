#include <stdio.h>

#include "sound.h"
#include "xcore.h"

#include <iostream>
#include <QMutex>

#ifdef HAVESDL2
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif

// new
static unsigned char sbuf[0x4000];
static int posf = 0x1000;			// fill pos
static int posp = 0x0004;			// play pos

static int smpCount = 0;
OutSys *sndOutput = NULL;
static int sndChunks = 882;

#define DISCRATE 32
int nsPerSample = 22675;
static int disCount = 0;
static sndPair tmpLev = {0, 0};
static sndPair sndLev;

OutSys* findOutSys(const char*);

// output

#include "hardware.h"

// return 1 when buffer is full
// NOTE: need sync|flush devices if debug
int sndSync(Computer* comp) {
	if (!conf.emu.pause || comp->debug) {
		gsFlush(comp->gs);
		saaFlush(comp->saa);
		if (!conf.emu.fast && !conf.emu.pause) {
			sndLev = comp->hw->vol(comp, &conf.snd.vol);
			sndLev.left = sndLev.left * conf.snd.vol.master / 100;
			sndLev.right = sndLev.right * conf.snd.vol.master / 100;
			if (sndLev.left > 0x7fff) sndLev.left = 0x7fff;
			if (sndLev.right > 0x7fff) sndLev.right = 0x7fff;

			tmpLev.left += sndLev.left;
			tmpLev.right += sndLev.right;
			disCount--;
			if (disCount < 0) {
				sndLev.left = tmpLev.left / DISCRATE;
				sndLev.right = tmpLev.right / DISCRATE;
				tmpLev.left = 0;
				tmpLev.right = 0;
				disCount = DISCRATE - 1;

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
extern int sleepy;

Uint32 sdl_timer_callback(Uint32 iv, void* ptr) {
	sleepy = 0;
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
	sleepy = 0;
}

int sdlopen() {
	int res;
	SDL_AudioSpec asp;
	SDL_AudioSpec dsp;
	asp.freq = conf.snd.rate;
	asp.format = AUDIO_S16LSB;
	asp.channels = conf.snd.chans;
	asp.samples = conf.snd.rate / 50 * conf.snd.chans;
	asp.callback = &sdlPlayAudio;
	asp.userdata = NULL;
	if (SDL_OpenAudio(&asp, &dsp) != 0) {
		printf("SDL audio device opening...failed (%s)\n", SDL_GetError());
		res = 0;
	} else {
		printf("SDL audio device opening...success: %i %i (%i / %i)\n",dsp.freq, dsp.samples,dsp.format,AUDIO_S16LSB);
		sndChunks = dsp.samples * DISCRATE;
		SDL_PauseAudio(0);
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
//	SDL_PauseAudio(1);
	SDL_CloseAudio();
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

void sndInit() {
	conf.snd.rate = 44100;
	conf.snd.chans = 2;
	conf.snd.enabled = 1;
	sndOutput = NULL;
	conf.snd.vol.beep = 100;
	conf.snd.vol.tape = 100;
	conf.snd.vol.ay = 100;
	conf.snd.vol.gs = 100;
	initNoise();							// ay/ym
}

// debug

void sndDebug() {
	printf("%i - %i = %i\n",posf, posp, posf - posp);
}
