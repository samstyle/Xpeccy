#include "filetypes.h"

typedef struct {
	char chunkId[4];		// "RIFF"
	unsigned int chunkSize;
	char format[4];			// "WAVE"
	char subchunk1Id[4];		// "fmt "
	unsigned int subchunk1Size;	// 16
	unsigned short audioFormat;	// 1 = PCM
	unsigned short numChannels;
	unsigned int sampleRate;
	unsigned int byteRate;		// sampleRate * numChannels * bitsPerSample/8
	unsigned short blockAlign;	// numChannels * bitsPerSample/8
	unsigned short bitsPerSample;
	char subchunk2Id[4];		// "data"
	unsigned int subchunk2Size;
} wavHead;

int loadWAV(Computer* comp, const char* name, int drv) {
	Tape* tap = comp->tape;
	FILE* file = fopen(name, "rb");
	if (!file) return ERR_CANT_OPEN;
	int err = ERR_OK;
	wavHead hd;
	fread((char*)&hd, sizeof(wavHead), 1, file);
	if (strncmp(hd.chunkId, "RIFF", 4)
		|| strncmp(hd.format, "WAVE", 4)
		|| strncmp(hd.subchunk1Id, "fmt ", 4)
		|| strncmp(hd.subchunk2Id, "data", 4)) {
		err = ERR_WAV_HEAD;
	} else if ((hd.audioFormat != 1) || (hd.sampleRate == 0) || (hd.bitsPerSample & 7)) {
		err = ERR_WAV_FORMAT;
	} else {
		int tPerSample = (int)1e6 / hd.sampleRate;		// mks per sample
//		int mask = 0x80 << (hd.bitsPerSample - 8);
		int amp;		// cur amp
		int lev = -1;		// last amp
		int dur = 0;		// signal duration
		int i;
		TapeBlock blk;
		blk.data = NULL;
		blkClear(&blk);

		while (!feof(file)) {
			amp = 0;
			for (i = 0; i < hd.numChannels; i++) {			// read (numChans) amps and get avg
				amp += freadLen(file, hd.bitsPerSample >> 3);
			}
			amp = hd.numChannels ? amp / hd.numChannels : 0;
			if (amp == lev) {				// check amp change
				dur += tPerSample;
			} else {
				lev = amp;
				blkAddPulse(&blk, dur, (amp >> (hd.bitsPerSample - 8)));
				if (dur > 5e5) {				// 0.5 sec pause is block-breaking signal
					tapAddBlock(tap, blk);
					blkClear(&blk);
				}
				dur = 0;
			}
		}
		if (dur > 0) {
			blkAddPulse(&blk, dur, -1);
		}
		if (blk.sigCount > 0)
			tapAddBlock(tap, blk);
	}
	fclose(file);
	return err;
}

int saveWAV(Computer* comp, const char* name, int drv) {
	int err = ERR_OK;
	if (comp->tape->blkCount < 1) {
		err = ERR_TAP_EMPTY;
	} else {
		wavHead hd;
		memcpy(hd.chunkId, "RIFF", 4);
		memcpy(hd.format, "WAVE", 4);
		memcpy(hd.subchunk1Id, "fmt ", 4);
		hd.subchunk1Size = 16;
		hd.audioFormat = 1;
		hd.numChannels = 1;
		hd.sampleRate = 22050;
		hd.byteRate = 22050;
		hd.blockAlign = 1;
		hd.bitsPerSample = 8;
		memcpy(hd.subchunk2Id, "data", 4);
		hd.subchunk2Size = 0;

		FILE* file = fopen(name, "wb");
		if (file) {
			int i;
			int pos;
			int tm = 0;
			int sz = 0;
			unsigned char amp;
			int tPerSample = (int)1e6 / hd.sampleRate;
			TapeBlock* blk;
			fwrite((char*)&hd, sizeof(wavHead), 1, file);
			for (i = 0; i < comp->tape->blkCount; i++) {
				blk = &comp->tape->blkData[i];
				tm = 0;
				for (pos = 0; pos < blk->sigCount; pos++) {
					tm = blk->data[pos].size;
					amp = blk->data[pos].vol;
					while (tm > 0) {
						tm -= tPerSample;
						fputc(amp, file);
						sz++;
					}
				}
			}
			fseek(file, sizeof(wavHead) - 4, SEEK_SET);
			fputi(sz, file);
			fclose(file);
		} else {
			err = ERR_CANT_OPEN;
		}
	}
	return err;
}
