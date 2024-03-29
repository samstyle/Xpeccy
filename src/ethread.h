#pragma once

#include <QThread>
#include <QMutex>

#include "xcore/xcore.h"

class xThread : public QThread {
	Q_OBJECT
	public:
		xThread();
		unsigned finish:1;
		int sndNs;
		int wavNs;
	public slots:
		void stop();
	signals:
		void s_frame();
		void dbgRequest();
		void scrRequest();
		void tapeSignal(int,int);
	private:
		void run();
		void emuCycle(Computer*);
		void tap_catch_load(Computer*);
		void tap_catch_save(Computer*);
};
