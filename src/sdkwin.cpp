#include <QtGui>
#include <unistd.h>
#include "sdkwin.h"

#include "ui_sdkwin.h"

#define PF_OPEN		1
#define	PF_CHANGED	(1<<1)
#define	PF_NOCHA	(1<<2)

struct PrjFile {
	int flag;
	QString name;
	QString text;
	QTreeWidgetItem* itm;
};

struct Project {
	int flag;
	QString name;
	QList<PrjFile> files;
};

bool olden;
Project prj;
PrjFile* curFile = NULL;
QTreeWidgetItem* curItem = NULL;
QString prjDir = QDir::homePath();
QString prjPath;
QString compPath = "/usr/local/bin/sjasmplus";
QStringList compArgs;
Ui::SDKWinForm ui;

SDKWindow* sdkwin;

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

	connect(ui.prjNew,SIGNAL(clicked()),this,SLOT(newProject()));
	connect(ui.prjOpen,SIGNAL(clicked()),this,SLOT(openProject()));
	connect(ui.prjSave,SIGNAL(clicked()),this,SLOT(saveProject()));

	connect(ui.prjBuild,SIGNAL(clicked()),this,SLOT(buildProject()));

	connect(ui.addExisting,SIGNAL(clicked()),this,SLOT(addExFiles()));

	connect(ui.prjtree,SIGNAL(itemActivated(QTreeWidgetItem*,int)),this,SLOT(prjFileChanged(QTreeWidgetItem*)));
	connect(ui.docedit,SIGNAL(textChanged()),this,SLOT(textChanged()));

	connect(&prc,SIGNAL(finished(int)),this,SLOT(buildFinish()));
}

void SDKWindow::closeEvent(QCloseEvent *ev) {
	saveProject();
	ev->accept();
}

// build

void SDKWindow::buildProject() {
	saveProject();
	if (prj.files.size() == 0) return;
	if (compPath.isEmpty()) {
		ui.statusbar->showMessage("Compiler not defined",3000);
		return;
	}
	if (!QFileInfo(compPath).exists()) {
		ui.statusbar->showMessage("Compiler not found",3000);
		return;
	}
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
	ui.prjtree->clear();
	QFont fnt("",-1,QFont::Bold);
	for (int i = 0; i < prj.files.size(); i++) {
		prj.files[i].itm = new QTreeWidgetItem();
		if (prj.files[i].flag & PF_CHANGED) prj.files[i].itm->setFont(0,fnt);
		prj.files[i].itm->setText(0,prj.files[i].name);
		prj.files[i].itm->setData(0,Qt::UserRole,prj.files[i].name);
		ui.prjtree->invisibleRootItem()->addChild(prj.files[i].itm);
	}
}

void SDKWindow::prjFileChanged(QTreeWidgetItem *itm) {
	curItem = itm;
	prj.flag |= PF_NOCHA;
	if (itm == NULL) {
		ui.docedit->clear();
		ui.docedit->setEnabled(false);
		curFile = NULL;
		prj.flag &= ~PF_NOCHA;
		return;
	}
	QString name = itm->data(0,Qt::UserRole).toString();
	QString path;
	if (curFile) curFile->text = ui.docedit->toPlainText();
	for (int i = 0; i < prj.files.size(); i++) {
		if (name == prj.files[i].name) {
			curFile = &prj.files[i];
			if (~curFile->flag & PF_OPEN) {
				path = prjPath;
				path.append("/").append(curFile->name);
				QFile file(path);
				if (file.open(QFile::ReadOnly)) {
					curFile->flag |= PF_OPEN;
					curFile->text = trUtf8(file.readAll());
					file.close();
				} else {
					ui.docedit->clear();
					ui.docedit->setEnabled(false);
					break;
				}
			}
			ui.docedit->setPlainText(curFile->text);
			ui.docedit->setEnabled(true);
			break;
		}
	}
	prj.flag &= ~PF_NOCHA;
}

// text

void SDKWindow::textChanged() {
	prj.flag |= PF_CHANGED;
	if (curFile) curFile->flag |= PF_CHANGED;
	if (curItem && (~prj.flag & PF_NOCHA)) {
		QFont fnt("",-1,QFont::Bold);
		curItem->setFont(0,fnt);
	}
}

// project

void addFile(PrjFile & npf) {
	PrjFile pf;
	bool fnd = false;
	foreach (pf, prj.files) {
		if (pf.name == npf.name) fnd = true;
	}
	if (fnd) return;
	prj.files.append(npf);
}

void SDKWindow::newProject() {
	QString fpath;
	QFile file;
	PrjFile pf;
	QString path = QFileDialog::getExistingDirectory(this,"Select new project directory",prjDir);
	if (path.isEmpty()) return;
	prj.name = "Xpeccy SDK Project";
	prjPath = path;
	prj.files.clear();
	pf.flag = 0;
	pf.name = "main.asm";
	pf.text.clear();
	prj.files << pf;
	fpath = path;				// create main.asm
	fpath.append("/main.asm");
	file.setFileName(fpath);
	file.open(QFile::WriteOnly);
	file.close();
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
	file.write("files {\n");
	foreach(PrjFile pf, prj.files) {
		file.write(pf.name.toUtf8());
		file.putChar('\n');
	}
	file.write("}\n");
	file.close();
}

void SDKWindow::openProject() {
	QString path = QFileDialog::getOpenFileName(this,"Open project",prjDir,"Project files (xsdk.conf)");
	if (path == "") return;
	QFile file(path);
	if (file.open(QFile::ReadOnly)) {
		QString line;
		PrjFile pf;
		prj.name = "Xpeccy SDK project";
		prj.files.clear();
		prjPath = QFileInfo(path).absoluteDir().absolutePath();
		while (!file.atEnd()) {
			line = trUtf8(file.readLine()).remove("\n").remove("\r");
			if (line.startsWith("name = ")) prj.name = line.mid(7);
			if (line.startsWith("files {")) {
				line = trUtf8(file.readLine()).remove("\n").remove("\r");
				while (!line.startsWith("}") && !file.atEnd()) {
					pf.flag = 0;
					pf.name = line;
					pf.text.clear();
					prj.files.append(pf);
					line = trUtf8(file.readLine()).remove("\n").remove("\r");
				}
			}
		}
		file.close();
		ui.docedit->clear();
		ui.docedit->setEnabled(false);
		buildTree();
	}
}

void SDKWindow::saveProject() {
	if (prj.flag == 0) return;
	QFile file;
	QString path;
	if (curFile) curFile->text = ui.docedit->toPlainText();
	for (int i = 0; i < prj.files.size(); i++) {
		if (prj.files[i].flag & PF_CHANGED) {
			path = prjPath;
			path.append("/").append(prj.files[i].name);
			file.setFileName(path);
			if (file.open(QFile::WriteOnly)) {
				file.write(prj.files[i].text.toUtf8());
				prj.files[i].flag &= ~PF_CHANGED;
				prj.files[i].itm->setFont(0,QFont());
				file.close();
			}
		}
	}
	savePrjFile();
	ui.statusbar->showMessage("All files saved",3000);
	prj.flag &= PF_CHANGED;
}

// files

void SDKWindow::addExFiles() {
	if (prjPath.isEmpty()) return;
	QStringList flist = QFileDialog::getOpenFileNames(this,"Add files to project",prjPath,"Spectrum asm files (*.asm)");
	if (flist.size() == 0) return;
	QString fnam;
	PrjFile pf;
	foreach(fnam,flist) {
		if (fnam.startsWith(prjPath)) {
			fnam.remove(0,prjPath.size() + 1);
			pf.flag = 0;
			pf.name = fnam;
			pf.itm = NULL;
			pf.text.clear();
			addFile(pf);
		}
	}
	buildTree();
}
