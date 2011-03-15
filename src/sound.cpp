//#include "video.h"
#include "tape.h"
//#include "z80.h"
#include "sound.h"
#include "spectrum.h"
#include "gs.h"

#include <iostream>
#if HAVESDLSOUND
	#include <SDL_mixer.h>
#endif

extern ZXComp* zx;
//extern Tape* tape;
extern GS* gs;
extern Sound* snd;

bool noizes[0x20000];		// here iz noize values [generated at start]

uint8_t envforms[16][33]={
/*	  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32	*/
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,253},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,253},
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,253},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,255},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,253},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,255}
};

AYChan::AYChan() {lev = false; vol = lim = bgn = pos = cur = 0;}

void Sound::defpars() {
	chunks = (int)(rate/50.0);
	bufsize = (int)(rate*chans/50.0);
	tatbyte = (int)(zx->sys->vid->frmsz / ((float)chunks * 1.0));	// 2.0
}

void Sound::setoutptr(std::string nam) {
	if (outsys != NULL) outsys->close();
//	printf("Finding %s from %i elements\n",nam.c_str(),outsyslist.size());
	outsys = NULL;
//	beepvol = tapevol = ayvol = gsvol = 16;
	uint32_t i;
	for (i=0; i<outsyslist.size();i++) {
//		printf("'%s' - '%s'\n",nam.c_str(),outsyslist[i].name.c_str());
		if (outsyslist[i].name == nam) {
			outsys = &outsyslist[i];
			break;
		}
	}
	if (outsys == NULL) {
		printf("Can't find sound system. Reset to NULL\n");
		outsys = &outsyslist[0];
	}
	if (!outsys->open()) {
		printf("Can't open sound system. Reset to NULL\n");
		outsys = &outsyslist[0];
	}
}

void Sound::sync() {
//	printf("%i + %i ? %i\n",t,tatbyte,sys->vid->t);
	t += tatbyte;
	zx->tape->sync();
	gs->sync(zx->sys->vid->t);
	lev = beeplev?beepvol:0;
	if (zx->tape->flags & TAPE_ON) lev += (((zx->tape->outsig & 1)?tapevol:0) + ((zx->tape->signal & 1)?tapevol:0));
#if 0
	SndData tmpl = (sc1->getvol() + sc2->getvol()) * (ayvol / 16.0) + gs->getvol() * (gsvol / 16.0);
	*(sbptr++) = lev + tmpl.l;
	*(sbptr++) = lev + tmpl.r;
#else
	levl = lev;
	levr = lev;
	SndData tmpl = sc1->getvol();
	levl += tmpl.l * ayvol / 16.0;
	levr += tmpl.r * ayvol / 16.0;
	tmpl = sc2->getvol();
	levl += tmpl.l * ayvol / 16.0;
	levr += tmpl.r * ayvol / 16.0;
	tmpl = gs->getvol();
	levl += tmpl.l * gsvol / 16.0;
	levr += tmpl.r * gsvol / 16.0;	
	*(sbptr++) = levl;
	*(sbptr++) = levr;
#endif
//	if (sbptr - sndbuf > 1024) sbptr--;
}

void Sound::addoutsys(std::string nam, bool (*opfunc)(), void (*pfunc)(), void (*clofunc)()) {
	OutSys newsys;
	newsys.name = nam;
	newsys.open = opfunc;
 	newsys.play = pfunc;
	newsys.close = clofunc;
	outsyslist.push_back(newsys);
//	printf("Added sound sys %s\n",nam.c_str());
}

AYProc::AYProc(int t) {
	stereo = AY_MONO;
	settype(t);
}

void AYProc::settype(int t) {
	type = t;
	switch (type) {
		case SND_NONE:
			freq = 1;
			break;
		case SND_AY:
			freq = 1774400;
			break;
		case SND_YM:
			freq = 1750000;
			break;
		default: throw("Internal error\nUnexpected AY type in AYProc::settype"); break;
	}
	aycoe = 400 * zx->sys->vid->frmsz / (float)freq;		// vid ticks in half-period of note 1 (400)
}

SndData AYProc::getvol() {
// calculating A,B,C,envelope & noise levels
	SndData res; res.l = res.r = 8;
	Video* p = zx->sys->vid;
	if (type == SND_NONE) return res;
	if (a.lim != 0) {if ((p->t - a.bgn) > a.lim) {a.lev = !a.lev; a.bgn += (uint32_t)a.lim;}} else {a.bgn = p->t;}
	if (b.lim != 0) {if ((p->t - b.bgn) > b.lim) {b.lev = !b.lev; b.bgn += (uint32_t)b.lim;}} else {b.bgn = p->t;}
	if (c.lim != 0) {if ((p->t - c.bgn) > c.lim) {c.lev = !c.lev; c.bgn += (uint32_t)c.lim;}} else {c.bgn = p->t;}
	if (e.lim != 0) {
		if ((p->t - e.bgn) > e.lim) {
			e.pos++;
			e.bgn += (uint32_t)e.lim;
			if (envforms[e.cur][e.pos]==255) e.pos--;
			if (envforms[e.cur][e.pos]==253) e.pos=0;
		}
	} else {e.bgn = p->t;}
	if (n.lim != 0) {if ((p->t - n.bgn) > n.lim) {n.pos++; n.bgn += (uint32_t)n.lim;}} else {n.bgn = p->t;}
	n.lev = noizes[n.pos & 0x1ffff];
// mix channels, envelope & noise
	a.vol=(reg[8]&16)?envforms[e.cur][e.pos]:(reg[8]&15);
	b.vol=(reg[9]&16)?envforms[e.cur][e.pos]:(reg[9]&15);
	c.vol=(reg[10]&16)?envforms[e.cur][e.pos]:(reg[10]&15);
	if ((reg[7]&0x09)!=0x09) {a.vol *= ((reg[7]&1)?0:a.lev)+((reg[7]&8)?0:n.lev);}
	if ((reg[7]&0x12)!=0x12) {b.vol *= ((reg[7]&2)?0:b.lev)+((reg[7]&16)?0:n.lev);}
	if ((reg[7]&0x24)!=0x24) {c.vol *= ((reg[7]&4)?0:c.lev)+((reg[7]&32)?0:n.lev);}
	switch (stereo) {
		case AY_ABC:
			res.l = a.vol + 0.7 * b.vol;
			res.r = c.vol + 0.7 * b.vol;
			break;
		case AY_ACB:
			res.l = a.vol + 0.7 * c.vol;
			res.r = b.vol + 0.7 * c.vol;
			break;
		case AY_BAC:
			res.l = b.vol + 0.7 * a.vol;
			res.r = c.vol + 0.7 * a.vol;
			break;
		case AY_BCA:
			res.l = b.vol + 0.7 * c.vol;
			res.r = a.vol + 0.7 * c.vol;
			break;
		case AY_CAB:
			res.l = c.vol + 0.7 * a.vol;
			res.r = b.vol + 0.7 * a.vol;
			break;
		case AY_CBA:
			res.l = c.vol + 0.7 * b.vol;
			res.r = a.vol + 0.7 * b.vol;
			break;
		default:
			res.l = res.r = (a.vol + b.vol + c.vol) * 0.7;	// mono
			break;
	}
	return res;
}

void AYProc::reset() {
	int i; for (i = 0;i < 16;i++) reg[i] = 0;
	n.cur = 0xffff; e.cur = 0;
	n.pos = e.pos = 0;
	a.bgn = b.bgn = c.bgn = n.bgn = e.bgn = zx->sys->vid->t;
	a.lim = b.lim = c.lim = n.lim = e.lim = 0;
}

void AYProc::setreg(uint8_t value) {
	if (curreg > 15) return;
	if (curreg < 14) reg[curreg]=value;
	switch (curreg) {
		case 0x00:
		case 0x01: a.lim = aycoe*(reg[0]+((reg[1]&0x0f)<<8)); break;
		case 0x02:
		case 0x03: b.lim = aycoe*(reg[2]+((reg[3]&0x0f)<<8)); break;
		case 0x04:
		case 0x05: c.lim = aycoe*(reg[4]+((reg[5]&0x0f)<<8)); break;
		case 0x06: n.lim = 2*aycoe*(value&31); break;
		case 0x0b:
		case 0x0c: e.lim = 2*aycoe*(reg[11]+(reg[12]<<8)); break;
		case 0x0d: e.cur = value&15; e.pos = 0; e.bgn = zx->sys->vid->t; break;
		case 0x0e: if (reg[7]&0x40) reg[14]=value; break;
		case 0x0f: if (reg[7]&0x80) reg[15]=value; break;
	}
}

//------------------------
// Sound output
//------------------------

bool null_open() {return true;}
void null_play() {}
void null_close() {}

#if HAVESDLSOUND

bool sdlopen() {
	printf("Open SDL audio device...");
	if (Mix_OpenAudio(snd->rate,AUDIO_S8,1,4096) < 0) {
		printf("failed: %s\n",Mix_GetError());
		return false;
	}
//	Mix_AllocateChannels(8);
//	Mix_Volume(-1,MIX_MAX_VOLUME);
	printf("OK\n");
	return true;
}

void sdlplay() {
	int len = snd->sbptr - snd->sndbuf;
	if (len == 0) return;
	Mix_Chunk *ch = Mix_QuickLoad_RAW(snd->sndbuf,len);
//printf("chunk data:\n");
//int i;
//for (i=0;i<len;i++) printf("%.2X  ",*(ch->abuf + i));
//printf("\n"); throw(0);
	if (Mix_PlayChannel(-1,ch,0) < 0) {
		printf("Mix_PlayChannel error: %s\n",Mix_GetError());
	}
	snd->sbptr = snd->sndbuf;
}

void sdlclose() {
	printf("Close SDL audio device\n");
	Mix_CloseAudio();
}

#endif

#ifndef WIN32

bool oss_open() {
	printf("Open OSS audio device\n");
	snd->audio=open("/dev/dsp",O_WRONLY,0777);
	if (snd->audio < 0) return false;
	ioctl(snd->audio,SNDCTL_DSP_SETFMT,&snd->sndformat);	// формат вывода
	ioctl(snd->audio,SNDCTL_DSP_CHANNELS,&snd->chans);	// число каналов
	ioctl(snd->audio,SNDCTL_DSP_SPEED,&snd->rate);		// частота дискретизации;
	return true;
}

void oss_play() {
	if (snd->audio < 0) return;
	int res;
	uint8_t* ptr = snd->sndbuf;
	int fsz = snd->sbptr - ptr;
	while (fsz > 0) {
		res = write(snd->audio, ptr, fsz);
		fsz -= res;
		ptr += res;//  * snd->chans;
	}
	snd->sbptr = snd->sndbuf;
}

void oss_close() {
	if (snd->audio < 0) return;
	printf("Close OSS audio device\n");
	close(snd->audio);
}

bool alsa_open() {
	int err;
	bool res = true;
	printf("libasound: open audio device... ");
	if ((err = snd_pcm_open(&snd->ahandle,snd->device,SND_PCM_STREAM_PLAYBACK,0))<0) {
		printf("playback open error: %s\n",snd_strerror(err));
		res=false;
	} else {
		if (snd->ahandle==NULL) {
			printf("shit happens\n");
			res = false;
		} else {
			printf("OK\n");
			printf("libasound: set audio paramz...");
			if ((err = snd_pcm_set_params(snd->ahandle,SND_PCM_FORMAT_U8,SND_PCM_ACCESS_RW_INTERLEAVED,snd->chans,snd->rate,1,100000)) < 0) {
				printf("playback open error: %s\n",snd_strerror(err));
				res=false;
			} else {
				printf("OK\n");
			}
		}
	}
	return res;
}

void alsa_play() {
	snd_pcm_sframes_t res;
	uint8_t* ptr = snd->sndbuf;
	int fsz = (snd->sbptr - ptr) / snd->chans;
	while (fsz > 0) {
		res = snd_pcm_writei(snd->ahandle, ptr, fsz);
		if (res<0) res = snd_pcm_recover(snd->ahandle,res,1);
		if (res<0) break;
		fsz -= res;
		ptr += res * snd->chans;
	}
	snd->sbptr = snd->sndbuf;
}

void alsa_close() {printf("libasound: close device\n");snd_pcm_close(snd->ahandle);}

#else


bool wave_open() {
	snd->wfx.wFormatTag = WAVE_FORMAT_PCM;
	snd->wfx.nChannels = snd->chans;
	snd->wfx.nSamplesPerSec = snd->rate;
	snd->wfx.nAvgBytesPerSec = snd->rate;
	snd->wfx.nBlockAlign = snd->wfx.nChannels;
	snd->wfx.wBitsPerSample = 8;
	snd->wfx.cbSize = 0;
	if (waveOutOpen(&snd->hwaveout,WAVE_MAPPER,&snd->wfx,0,0,CALLBACK_NULL) != MMSYSERR_NOERROR) {
		printf("Can't open wave out device\n");
		return false;
	}
	return true;
}

void wave_play() {
	if (snd->hwaveout!=NULL) {
		snd->whdr.dwFlags = 0;
		snd->whdr.lpData = (CHAR*)snd->sndbuf;
		snd->whdr.dwBufferLength = snd->sbptr - snd->sndbuf;
		snd->whdr.dwLoops = 0;
		waveOutPrepareHeader(snd->hwaveout,&snd->whdr,sizeof(WAVEHDR));
		waveOutWrite(snd->hwaveout,&snd->whdr,sizeof(WAVEHDR));
		waveOutUnprepareHeader(snd->hwaveout,&snd->whdr,sizeof(WAVEHDR));
	}
	snd->sbptr = snd->sndbuf;
}

void wave_close() {
	waveOutClose(snd->hwaveout);
}

bool ds_open() {
	snd->wfx.wFormatTag = WAVE_FORMAT_PCM;
	snd->wfx.nChannels = snd->chans;
	snd->wfx.nSamplesPerSec = snd->rate;
	snd->wfx.nAvgBytesPerSec = snd->rate;
	snd->wfx.nBlockAlign = snd->wfx.nChannels;
	snd->wfx.wBitsPerSample = 8;
	snd->wfx.cbSize = 0;

	DirectSoundCreate(0,&snd->ds,0);

	snd->dsbdsc.dwFlags = DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME;
	snd->dsbdsc.dwBufferBytes = snd->bufsize;
	snd->dsbdsc.lpwfxFormat = &snd->wfx;
	snd->dsbdsc.dwSize = sizeof(DSBUFFERDESC);

	snd->ds->CreateSoundBuffer(&snd->dsbdsc,&snd->dsbuf,0);
	snd->dsbuf->SetFrequency(snd->rate);
	snd->dsbuf->SetVolume(DSBVOLUME_MAX);

	return true;
}

void ds_play() {
	void* zbuf;
	DWORD sze;
	snd->dsbuf->Lock(0,0,&zbuf,&sze,0,0,DSBLOCK_ENTIREBUFFER);
	memcpy(zbuf,snd->sndbuf,snd->bufsize);
	snd->dsbuf->Unlock(zbuf,sze,0,0);
	snd->dsbuf->Play(0,0,0);
	snd->sbptr = snd->sndbuf;
}

void ds_close() {
	snd->ds->Release();
}

#endif

//=============

Sound::Sound() {
#ifndef WIN32
	device = (char*)"default";
	output = NULL;
	sndformat = AFMT_U8;
#endif
	sbptr = &sndbuf[0];
	rate = 44100;
	chans = 2;
//	ay.freq = 1774400;
	enabled = true;
	mute = true;
	outsys = NULL;
	t = 0;
	beepvol = tapevol = ayvol = 16;
	sbptr = sndbuf;

	sc1 = new AYProc(SND_AY);
	sc2 = new AYProc(SND_NONE);
	tstype = TS_NEDOPC;

	int i;
	sc1->n.cur = 0xffff;
	for (i=0;i<0x20000;i++) {
		sc1->n.lev = sc1->n.cur&0x10000;
		noizes[i] = sc1->n.lev;
		sc1->n.cur = ((sc1->n.cur<<1) + ((sc1->n.lev==((sc1->n.cur&0x2000)==0x2000))?0:1)) & 0x1ffff;
	}
	sc2->n.cur = sc1->n.cur;

	addoutsys("NULL",&null_open,&null_play,&null_close);
#ifndef WIN32
	addoutsys("OSS",&oss_open,&oss_play,&oss_close);
	addoutsys("ALSA",&alsa_open,&alsa_play,&alsa_close);
#else
	addoutsys("WaveOut",&wave_open,&wave_play,&wave_close);
	addoutsys("DirectSound",&ds_open,&ds_play,&ds_close);
#endif
#if HAVESDLSOUND
	addoutsys("SDL",&sdlopen,&sdlplay,&sdlclose);	// TODO: do something with SDL output
#endif
}
