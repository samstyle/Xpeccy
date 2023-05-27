#pragma once

#include <QApplication>
#include <QEvent>

class xApp : public QApplication {
	Q_OBJECT
	public:
		xApp(int& ac, char** av, int iv):QApplication(ac, av, iv) {}
	public slots:
		void d_frame();
	signals:
		void s_frame();
	protected:
		bool event(QEvent*);
};
