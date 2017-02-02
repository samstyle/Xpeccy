#ifndef _ETHREAD_H
#define _ETHREAD_H

#include <QThread>
#include <QMutex>
#include "xcore/xcore.h"

class xThread : public QThread {
	Q_OBJECT
	public:
		xThread();
		unsigned fast:1;
		unsigned block:1;
		unsigned finish:1;
		xConfig* conf;
		Computer* comp;
		QMutex mtx;
		int sndNs;
		void run();
	private:
		void emuCycle();
		void tapeCatch();
	signals:
		void dbgRequest();
		void tapeSignal(int,int);
};

#endif
