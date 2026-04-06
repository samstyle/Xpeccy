#include <stdlib.h>
#include <string.h>

#include "input.h"

// joystick

Joystick* joyCreate() {
	Joystick* joy = (Joystick*)malloc(sizeof(Joystick));
	joy->type = XJ_KEMPSTON;
	joy->state = 0;
	return joy;
}

void joyDestroy(Joystick* joy) {
	free(joy);
}

void joyPress(Joystick* joy, int mask) {
	joy->state |= mask;
}

void joyRelease(Joystick* joy, int mask) {
	joy->state &= ~mask;
}

unsigned char joyInput(Joystick* joy) {
	unsigned char res = 0xff;
	joy->used = 1;
	switch (joy->type) {
		case XJ_KEMPSTON:
			res = joy->state;
			if (!joy->extbuttons)
				res |= ~0x1f;
			break;
	}
	return res;
}
