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
//		unsigned silent:1;	// don't produce sound
//		unsigned block:1;
		unsigned finish:1;
		int sndNs;
		void stop();
	signals:
		void s_frame();
		//void picReady();	// picture ready for display
		void dbgRequest();
		void tapeSignal(int,int);
	private:
		void run();
		void emuCycle(Computer*);
		void tapeCatch(Computer*);
};

// extern QMutex emutex;

#endif
