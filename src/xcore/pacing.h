#pragma once

// Paces the emulation thread from a steady wall-clock timer instead of the
// audio callback.
//
// Why:
// The emulation thread runs until it has made conf.snd.need samples. That
// counter used to be filled by the SDL audio callback - once per audio
// buffer. So the emulation ran in bursts the size of the audio buffer.
// Two problems came from that:
//  1) The buffer is not a whole number of frames, so the number of frames
//     made per burst slowly drifts. About every 3 seconds one frame comes
//     out late or is skipped - a visible hitch, very clear on gigascreen
//     (it alternates two pictures every frame, so a lost frame flashes).
//  2) Input is only read while the thread runs, so a big audio buffer means
//     big input lag.
//
// Fix:
// A small SDL timer fills conf.snd.need from real elapsed time, evenly, no
// matter how big the audio buffer is. Frames are then made at a steady rate
// and input is read often. The audio ring buffer already separates making
// sound from playing it (see sndGetRingDistance in sound.cpp), so pacing
// from the clock instead of the callback does not hurt audio.

void pacingInit();
void pacingClose();
