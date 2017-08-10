#ifndef X_NESAPU_H
#define X_NESAPU_H

typedef struct {
	unsigned en:1;	// channel is enabled
	unsigned lev:1;
	unsigned lenen:1;	// length counter enabled
	unsigned env:1;		// envelope active
	unsigned eloop:1;	// envelope loop enabled
	int vol;	// current volume
	int eper;	// envelope period
	int per;	// 50/50 duty period calculated from frequence
	int per0;	// low & high periods according duty cycle
	int per1;
	int pcount;	// period countdown
	int tcount;	// ticks counter
	int step;	// halfwave counter
	int len;	// decrease every 4 ticks. if (len == 0) there is no sound
} nesapuChan;

typedef struct {
	unsigned pal:1;		// PAL mode
	unsigned inten:1;	// enable frame IRQ signal
	unsigned frm:1;		// frame IRQ
	nesapuChan ch0;		// tone 0
	nesapuChan ch1;		// tone 1
	nesapuChan ch2;		// triangle
	nesapuChan ch3;		// noise
	nesapuChan ch4;		// DMC wave
	long period;		// ns per tick @ 240Hz = CPUfrq/14915 (NTSC) | PAL skips every 5th tick ~ 192Hz
	long tick;		// ns tick counter
	int tcount;		// 0 to 4
} nesAPU;

nesAPU* apuCreate();
void apuDestroy(nesAPU*);
void apuSync(nesAPU*, long);

#endif
