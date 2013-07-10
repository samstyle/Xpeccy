#include <QtGui>
#include <unistd.h>
#include "sdkwin.h"

#include "ui_sdkwin.h"

#define PF_OPEN		1
#define	PF_CHANGED	(1<<1)
#define	PF_NOCHA	(1<<2)
#define	PF_CURRENT	(1<<3)

struct opFiles {
	int flag;
	int line;
	int curPos;
	QString path;
	QString text;
};

struct Project {
	int flag;
	int build;
	QString name;
	QList<opFiles> files;
};

bool olden;
Project prj;
QString prjDir = QDir::homePath();
QString prjPath;
QString compPath = "/usr/local/bin/sjasmplus";
QStringList compArgs;
Ui::SDKWinForm ui;
QFileSystemModel dmod;

SDKWindow* sdkwin;
MSyn* syn;

// external

void devInit() {
	sdkwin = new SDKWindow;
}

void devShow() {
	sdkwin->show();
}


// internal

SDKWindow::SDKWindow(QWidget *p):QMainWindow(p) {
	ui.setupUi(this);

	syn = new MSyn(this);

	connect(ui.prjNew,SIGNAL(clicked()),this,SLOT(newProject()));
	connect(ui.prjOpen,SIGNAL(clicked()),this,SLOT(openProject()));
	connect(ui.prjSave,SIGNAL(clicked()),this,SLOT(saveProject()));
	connect(ui.prjBuild,SIGNAL(clicked()),this,SLOT(buildProject()));

//	connect(ui.docedit,SIGNAL(textChanged()),this,SLOT(textChanged()));
	connect(ui.prjtree,SIGNAL(clicked(QModelIndex)),this,SLOT(prjFileChanged(QModelIndex)));

	connect(&prc,SIGNAL(finished(int)),this,SLOT(buildFinish()));
}

void SDKWindow::closeEvent(QCloseEvent *ev) {
	saveProject();
	ev->accept();
}

// build

void SDKWindow::buildProject() {
	if (prjPath.isEmpty()) return;
	if (compPath.isEmpty()) {
		ui.statusbar->showMessage("Compiler not defined",3000);
		return;
	}
	if (!QFileInfo(compPath).exists()) {
		ui.statusbar->showMessage("Compiler not found",3000);
		return;
	}
	prj.build++;
	saveProject();
	olden = ui.docedit->isEnabled();
	ui.butWidget->setEnabled(false);
	ui.docedit->setEnabled(false);
	QStringList args = compArgs;
	args << "main.asm";
	prc.setWorkingDirectory(prjPath);
	prc.start(compPath,args);
}

void SDKWindow::buildFinish() {
	ui.butWidget->setEnabled(true);
	ui.docedit->setEnabled(olden);
	QString out = trUtf8(prc.readAllStandardOutput());
	ui.console->setPlainText(out);
	ui.statusbar->showMessage("Build finished",3000);
}

// tree

void SDKWindow::buildTree() {
	dmod.setRootPath(prjPath);
	dmod.setNameFilters(QStringList() << "*.asm");
	dmod.setNameFilterDisables(false);
	ui.prjtree->setModel(&dmod);
	ui.prjtree->setRootIndex(dmod.index(prjPath));
	ui.prjtree->hideColumn(1);
	ui.prjtree->hideColumn(2);
	ui.prjtree->hideColumn(3);
	ui.prjtree->hideColumn(4);
}

void saveCurrentText(QPlainTextEdit* ed, bool res) {
	for(int i = 0; i < prj.files.size(); i++) {
		if (prj.files[i].flag & PF_CURRENT) {
			if (res) prj.files[i].flag &= ~PF_CURRENT;
			prj.files[i].line = ed->verticalScrollBar()->value();
			prj.files[i].curPos = ed->textCursor().position();
			prj.files[i].text = ed->toPlainText();
			break;
		}
	}
}

void SDKWindow::prjFileChanged(QModelIndex midx) {
	QFile file;
	QFileInfo finf = dmod.fileInfo(midx);
	if (!finf.isFile()) return;
	QString fname = finf.filePath();

	saveCurrentText(ui.docedit,true);

	bool newfile = true;
	for (int i = 0; i < prj.files.size(); i++) {
		if (prj.files[i].path == fname) {
			newfile = false;
			prj.files[i].flag |= PF_CURRENT;
			ui.docedit->setPlainText(prj.files[i].text);
			syn->setDocument(ui.docedit->document());
			ui.docedit->setEnabled(true);
			ui.docedit->verticalScrollBar()->setValue(prj.files[i].line);
			QTextCursor curs = ui.docedit->textCursor();
			curs.setPosition(prj.files[i].curPos);
			ui.docedit->setTextCursor(curs);
			break;
		}
	}
	if (!newfile) return;

	opFiles of;
	of.flag = PF_CURRENT;
	of.path = fname;
	of.text.clear();
	file.setFileName(fname);
	if (file.open(QFile::ReadOnly)) {
		of.text = trUtf8(file.readAll());
		prj.files.append(of);
	}
	ui.docedit->setPlainText(of.text);
	ui.docedit->setEnabled(true);
}

// project

void SDKWindow::newProject() {
	QString fpath;
	QFile file;
	QString path = QFileDialog::getExistingDirectory(this,"Select new project directory",prjDir);
	if (path.isEmpty()) return;
	prj.name = "Xpeccy SDK Project";
	prjPath = path;
	fpath = path;				// create main.asm
	fpath.append("/main.asm");
	file.setFileName(fpath);
	if (!file.exists()) {
		file.open(QFile::WriteOnly);
		file.close();
	}
	savePrjFile();
	buildTree();
}

void SDKWindow::savePrjFile() {
	QString fpath;
	QFile file;
	fpath = prjPath;
	fpath.append("/xsdk.conf");
	file.setFileName(fpath);
	file.open(QFile::WriteOnly);
	file.write("name = ");
	file.write(prj.name.toUtf8());
	file.putChar('\n');
	file.write("build = ");
	file.write(QString::number(prj.build).toUtf8());
	file.putChar('\n');
	file.close();
}

void SDKWindow::openProject() {
	QString path = QFileDialog::getOpenFileName(this,"Open project",prjDir,"Project files (xsdk.conf)");
	if (path == "") return;
	QFile file(path);
	if (file.open(QFile::ReadOnly)) {
		QString line;
		prj.name = "Xpeccy SDK project";
		prjPath = QFileInfo(path).absoluteDir().absolutePath();
		while (!file.atEnd()) {
			line = trUtf8(file.readLine()).remove("\n").remove("\r");
			if (line.startsWith("name = ")) prj.name = line.mid(7);
			if (line.startsWith("build = ")) prj.build = line.mid(8).toInt();
		}
		file.close();
		ui.docedit->clear();
		ui.docedit->setEnabled(false);
		buildTree();
	}
}

void SDKWindow::saveProject() {
	saveCurrentText(ui.docedit,false);
	opFiles of;
	QFile file;
	foreach(of,prj.files) {
		file.setFileName(of.path);
		if (file.open(QFile::WriteOnly)) {
			file.write(of.text.toUtf8());
			file.close();
		}
	}
/*
	for (int i = 0; i < dmod.rowCount(); i++) {
		dmod.setData(dmod.index(i,0),QFont(),Qt::FontRole);
	}
*/
	ui.prjtree->setModel(&dmod);
	savePrjFile();
	ui.statusbar->showMessage("Project saved",3000);
}

// text

void SDKWindow::textChanged() {
/*
	if (!ui.docedit->isEnabled()) return;
	QModelIndex idx = ui.prjtree->currentIndex();
	if (!idx.isValid()) return;
	dmod.setData(idx,QFont("",-1,QFont::Bold),Qt::FontRole);
	ui.prjtree->setModel(&dmod);
*/
}

// highlighter

void MSyn::highlightBlock(const QString &text) {
	// do something
}
