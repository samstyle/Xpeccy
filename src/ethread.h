#ifndef _ETHREAD_H
#define _ETHREAD_H

#include <QThread>
#include <mutex>

#include "xcore/xcore.h"

class xThread : public QThread {
	Q_OBJECT
	public:
		xThread();
		unsigned fast:1;
		unsigned silent:1;	// don't produce sound
		unsigned block:1;
		unsigned finish:1;
		unsigned waitpic:1;	// display thread waits for new picture (basicly: picReady signal enabled)
		xConfig* conf;
		Computer* comp;
		int sndNs;
		void run();
	signals:
		void picReady();	// picture ready for display
	private:
		void emuCycle();
		void tapeCatch();
	signals:
		void dbgRequest();
		void tapeSignal(int,int);
};

extern std::mutex emutex;

#endif
