#include "gbcvideo.h"

void gbcvDraw(GBCVid* vid) {

}

void gbcvLine(GBCVid* vid) {

}

void gbcvFram(GBCVid* vid) {

}

void gbcvInit(GBCVid* vid) {
	vid->draw = gbcvDraw;
	vid->cbLine = gbcvLine;
	vid->cbFram = gbcvFram;
}
