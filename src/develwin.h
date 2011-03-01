#ifndef _DEVWIN_H
#define _DEVWIN_H

#include "ui_develwin.h"
#include "ui_selname.h"

#include <QCloseEvent>
#include <QDialog>
#include <QMenu>
#include <QDir>

#define PF_MAIN 1

struct ProjectFile {
	QString name;
	QTextEdit* text;
};

struct Project {
	QString name;
	QList<ProjectFile> files;
	int build;
	void clear();
	bool havefile(QString);
};

class DevelWin : public QDialog {
	Q_OBJECT
	public:
		DevelWin();
	public slots:
		void start();
	private slots:
		void newproj(); void cnfnewprj();
		void newfile(); void cnfnewfile();
		void changeprj(QAction*);
		void removefile(int);
		void compile();
	private:
		Project prj;
		QMenu *pmenu;
		QDialog *adpwid,*adfwid;
		Ui::DevelWin ui;
		Ui::IName adpui,adfui;
		QDir prjdir;
		void makepmenu();
		void createprj(QString);
		void openprj(QString);
		void saveprj();
	protected:
		void closeEvent(QCloseEvent*);
};

#endif
