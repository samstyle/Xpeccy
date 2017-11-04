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

int loadWAV(Tape* tap, const char* name) {
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
	} else if ((hd.audioFormat != 1) || (hd.sampleRate == 0)) {
		err = ERR_WAV_FORMAT;
	} else {
		int tPerSample = 3584000 / hd.sampleRate;	// 3.5MHz ticks per sample
		int mask = 0x80 << (hd.bitsPerSample - 8);
		unsigned int amp;
		int lev = 0;
		int dur = 0;
		int i;
		TapeBlock blk;
		blkClear(&blk);

		while (!feof(file)) {
			amp = 0;
			for (i = 0; i < hd.numChannels; i++) {			// read (numChans) amps and get middle
				amp += freadLen(file, hd.bitsPerSample >> 3);
			}
			amp = hd.numChannels ? amp / hd.numChannels : 0;
			if (((amp & mask) && lev) || (!(amp & mask) && !lev)) {	// check level change
				dur += tPerSample;
			} else {
				blkAddPulse(&blk, dur);
				if (dur > 1792000) {			// 0.5 sec pause is block-breaking signal
					tapAddBlock(tap, blk);
					blkClear(&blk);
				}
				dur = tPerSample;
			}
		}
		if (blk.sigCount) tapAddBlock(tap, blk);
	}
	fclose(file);
	return err;
}

int saveWAV(Tape* tap, const char* name) {

	return ERR_OK;
}
