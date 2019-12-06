#ifndef X_ETHREAD_H
#define X_ETHREAD_H

#include <QThread>
#include <QMutex>

#include "xcore/xcore.h"

class xThread : public QThread {
	Q_OBJECT
	public:
		xThread();
		unsigned finish:1;
		int sndNs;
	public slots:
		void stop();
	signals:
		void s_frame();
		void dbgRequest();
		void tapeSignal(int,int);
	private:
		void run();
		void emuCycle(Computer*);
		void tapeCatch(Computer*);
};

// extern QMutex emutex;

#endif
