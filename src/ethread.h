#ifndef X_ETHREAD_H
#define X_ETHREAD_H

#include <QThread>
#include <QMutex>

#include "xcore/xcore.h"

class xThread : public QThread {
	Q_OBJECT
	public:
		xThread();
//		unsigned fast:1;
		unsigned silent:1;	// don't produce sound
		unsigned block:1;
		unsigned finish:1;
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

extern QMutex emutex;

#endif
