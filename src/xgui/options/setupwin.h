#pragma once

#include <QDialog>
#include <QKeyEvent>
#include <QShortcut>
#include <QModelIndex>
#include <QKeySequence>
#include <QAbstractTableModel>

#include "xcore.h"
#include "padbinder.h"
#include "opt_romset.h"
#include "opt_diskcat.h"
#include "opt_tapecat.h"

#include "ui_rsedit.h"
#include "ui_setupwin.h"
#include "ui_umadial.h"
#include "ui_layedit.h"

// Since Qt5.2 there is QKeySequenceEdit class

class xKeyEditor : public QDialog {
	Q_OBJECT
	public:
		xKeyEditor(QWidget* p = NULL);
		void edit(int);
	signals:
		void s_done(int, QKeySequence);
	private:
		int foo;
		QLabel lab;
		QPushButton but;
		QKeySequence kseq;
		void keyPressEvent(QKeyEvent*);
		void keyReleaseEvent(QKeyEvent*);
	private slots:
		void okay();
		void reject();
};

class SetupWin : public QDialog {
	Q_OBJECT
	public:
		//unsigned block:1;
		//unsigned prfChanged:1;

		SetupWin(QWidget*);

	public slots:
		void start(xProfile*);
	private:
		xProfile* prof;
		Computer* comp;

		Ui::SetupWin ui;
		Ui::UmaDial uia;
		Ui::LayEditor layUi;

		xRomsetEditor* rseditor;
		xRomsetModel* rsmodel;

		QDialog* layeditor;
		QDialog *umadial;
		xPadMapModel* padModel;
		xPadBinder* padial;
		xKeyEditor* kedit;

		int eidx;

		xLayout nlay;
		void editLayout();

		int bindidx;
		int umidx;
		void buildtapelist();
		void buildmenulist();
		void buildkeylist();
		void buildproflist();
		void buildpadlist();

	signals:
		void closed();
		void s_prf_change(std::string);
	private slots:
		void reject();
		void apply();
		void okay();
		void buildrsetlist();
		void fillKeys();
		void setmszbox(int);
		void selsspath();
		void chabsz();
		void chaflc();
		void updatedisknams();
		void loada(); void loadb(); void loadc(); void loadd();
		void savea(); void saveb(); void savec(); void saved();
		void ejcta(); void ejctb(); void ejctc(); void ejctd();
		void newa(); void newb(); void newc(); void newd(); void newdisk(int);
		void loatape(); void savtape(); void ejctape();
		void tblkup(); void tblkdn(); void tblkrm();
		void hddMasterImg(); void hddSlaveImg();
		void umadd(); void umdel(); void umup(); void umdn();
		void umedit(QModelIndex);
		void umaselp(); void umaconf();
		void chablock(QModelIndex);
		void tlistclick(QModelIndex);
		void hddcap();

		void selSDCimg();

		void openSlot();
		void ejectSlot();

		void addNewRomset();
		void rmRomset();
		void addRom();
		void editRom();
		void delRom();
		void setRom(xRomFile);
		void romPreset();

		void newPadMap();
		void delPadMap();
		void chaPadMap(int);
		void addBinding();
		void editBinding();
		void delBinding();
		void bindAccept(xJoyMapEntry);

		void fillDiskCat();
		void copyToTape();
		void copyToDisk();
		void diskToHobeta();
		void diskToRaw();

		void newProfile();
		void copyProf();
		void chProfile(int, int);
		void rmProfile();

		void edLayout();
		void addNewLayout();
		void delLayout();
		void layEditorChanged();
		void layEditorOK();
		void layNameCheck(QString);

		void doHotKey();
		void setHotKey(int, QKeySequence);

		void selectColor();
		void triggerColor();
};

int getRFIData(QComboBox*);
