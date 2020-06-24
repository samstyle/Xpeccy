#pragma once

#include <QApplication>
#include <QEvent>

class xApp : public QApplication {
	Q_OBJECT
	public:
		xApp(int& ac, char** av, int iv):QApplication(ac, av, iv) {}
	protected:
		bool event(QEvent*);
};
