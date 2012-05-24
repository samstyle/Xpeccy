#include "xcore/xcore.h"
#include "xgui/xgui.h"
#include "develwin.h"
#include "settings.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDir>

DevelWin* devWin;

void devInit() {
	devWin = new DevelWin();
}

void devShow() {
	devWin->show();
	devWin->prjdir = QDir(QString(optGetString(OPT_PROJDIR).c_str()));
}

DevelWin::DevelWin() {
	ui.setupUi(this);
	ui.ptabs->clear();

	pmenu = new QMenu(); ui.opnptb->setMenu(pmenu);		// list of projects

	QObject::connect(pmenu,SIGNAL(triggered(QAction*)),this,SLOT(changeprj(QAction*)));
	QObject::connect(ui.ptabs,SIGNAL(tabCloseRequested(int)),this,SLOT(removefile(int)));

	QObject::connect(ui.newptb,SIGNAL(released()),this,SLOT(newproj()));
	QObject::connect(ui.newpftb,SIGNAL(released()),this,SLOT(newfile()));

	QObject::connect(ui.comptb,SIGNAL(released()),this,SLOT(compile()));
}

void DevelWin::closeEvent(QCloseEvent *ev) {saveprj(); ev->accept();}

//=======================
// project

bool Project::havefile(QString nam) {
	int i;
	bool res = false;
	for(i=0;i<files.size();i++) {
		if (files[i].name == nam) {res=true; break;}
	}
	return res;
}

void Project::clear() {
	name = "";
	files.clear();
	build = 0;
}

//=======================
// all

void DevelWin::compile() {
	if (prj.name=="" || prj.files.size()==0) return;
	saveprj();
	ui.ficons->clear();
	QString sjap(optGetString(OPT_ASMPATH).c_str());
	if (!QFile::exists(sjap)) {
		shitHappens("<b>SJAsm file doesn't exist</b>");
		return;
	}
	QProcess proc;
	proc.setWorkingDirectory(prjdir.absolutePath() + "/" + prj.name + "/");
	proc.start(QString(optGetString(OPT_ASMPATH).c_str()),QStringList() << prj.files[0].name);
	proc.waitForFinished(60000);			// 1 minute for finishing compilation
	ui.ficons->setText(QDialog::trUtf8(proc.readAll()));
	prj.build++;
	setWindowTitle(prj.name + " (build " + QString::number(prj.build) + ")");
}

void DevelWin::removefile(int idx) {
	QMessageBox mbx;
	QString nam = ui.ptabs->tabText(idx);
	mbx.setIcon(QMessageBox::Warning);
	mbx.setText(QString("<b>Do you want remove file <i>%1</i> too</b>?").arg(nam));
	mbx.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	QFile file(prjdir.absolutePath() + "/" + prj.name + "/" + nam);
	int res = mbx.exec();
	switch(res) {
		case QMessageBox::Yes:
			file.remove();
		case QMessageBox::No:
			prj.files.removeAt(idx);
			ui.ptabs->removeTab(idx);
			break;
	}
}

void DevelWin::newfile(QString nam) {
	if (prj.name=="") return;
	if (nam.isEmpty()) nam = QInputDialog::getText(this,"Enter...","Input new file name");
	if (nam.isEmpty()) return;
	ProjectFile nfile;
	if (prj.havefile(nam)) {
		QMessageBox mbx;
		mbx.setIcon(QMessageBox::Warning);
		mbx.setText("<b>File exists in this project</b>");
		mbx.exec();
	} else {
		nfile.name = nam;
		nfile.text = new QTextEdit();
		nfile.text->clear();
		prj.files.append(nfile);
		ui.ptabs->addTab(nfile.text,nam);
	}
}

void DevelWin::newproj() {
	QString nam = QInputDialog::getText(this,"Enter...","Input new project name");
	if (nam=="") return;
	if (prjdir.entryList(QDir::AllDirs, QDir::Name).indexOf(nam)<0) {
		createprj(nam);
	} else {
		QMessageBox mbx;
		mbx.setIcon(QMessageBox::Warning);
		mbx.setText("<b>Project exist</b><br>It will be opened");
		mbx.exec();
		openprj(nam);
	}
}

void DevelWin::createprj(QString nam) {
	saveprj();
	ui.ptabs->clear();
	prj.clear();
	prj.name = nam;
	prjdir.mkdir(nam);
	newfile("main.asm");
	saveprj();
	makepmenu();
	setWindowTitle(prj.name + " (build " + QString::number(prj.build) + ")");
}

void DevelWin::changeprj(QAction *act) {openprj(act->text());}

void DevelWin::openprj(QString nam) {
	saveprj();
	QString dir = prjdir.absolutePath() + "/" + nam + "/";
	QFile file(dir + "project.conf");
	if (!file.open(QFile::ReadOnly)) return;			// TODO: can't open project config
	ui.ptabs->clear();
	prj.clear();
	prj.name = nam;
	int mod = 0;
	std::pair<std::string,std::string> spl;
	std::string line,pnam,pval;
	ProjectFile nfile;
	while (!file.atEnd()) {
		line = std::string(QDialog::trUtf8(file.readLine()).toUtf8().data());
		spl = splitline(line);
		pnam = spl.first;
		pval = spl.second;
		if (pval=="") {
			if (mod==2 && pnam!="") {
				nfile.name = QString(pnam.c_str());
				nfile.text = new QTextEdit;
				nfile.text->clear();
				prj.files.append(nfile);
				ui.ptabs->addTab(prj.files.last().text,nfile.name);
			} else {
				if (pnam=="[OPTIONS]") mod=1;
				if (pnam=="[FILES]") mod=2;
			}
		} else {
			switch (mod) {
				case 1:
					if (pnam=="build") prj.build = atoi(pval.c_str());
					break;
			}
		}
	}
	file.close();
	int i;
	for (i=0;i<prj.files.size();i++) {
		file.setFileName(dir + prj.files[i].name);
		file.open(QFile::ReadOnly);
		prj.files[i].text->setText(QDialog::trUtf8(file.readAll()));
		file.close();
	}
	makepmenu();
	setWindowTitle(prj.name + " (build " + QString::number(prj.build) + ")");
}

void DevelWin::saveprj() {
	if (prj.name=="") return;
	QString dir = prjdir.absolutePath() + "/" + prj.name + "/";
	QFile file(dir + "project.conf");
	if (!file.open(QFile::WriteOnly)) return;		// TODO can't open file for writing
	file.write(QString("[OPTIONS]\n\nbuild = %1\n\n[FILES]\n\n").arg(prj.build).toUtf8());
	QFile fle2;
	int i;
	for (i=0;i<prj.files.size();i++) {
		file.write(QString(prj.files[i].name + "\n").toUtf8());
		fle2.setFileName(dir + prj.files[i].name);
		fle2.open(QFile::WriteOnly);
		fle2.write(prj.files[i].text->toPlainText().toUtf8());
		fle2.close();
	}
	file.close();
}

void DevelWin::start() {
	QDir dir(QString(optGetString(OPT_PROJDIR).c_str()));
	if (!dir.exists()) {
		shitHappens("<b>Projects directory not exists</b><br>Select it correctly first");
		return;
	}
	prjdir = QDir(QString(optGetString(OPT_PROJDIR).c_str()));
	makepmenu();
	show();
}

void DevelWin::makepmenu() {
	pmenu->clear();
	QStringList flist = prjdir.entryList(QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name);
	int i;
	QAction *act;
	for(i=0;i<flist.size();i++) {
		act = new QAction(flist[i],pmenu);
		pmenu->addAction(act);
	}
	if (pmenu->isEmpty()) {
		act = new QAction("None",pmenu);
		act->setEnabled(false);
		pmenu->addAction(act);
	}
}
