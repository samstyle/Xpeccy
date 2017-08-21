#ifndef _SETWIN_H
#define _SETWIN_H

#include <QDialog>
#include <QModelIndex>
#include <QAbstractTableModel>

#include "xcore.h"
#include "padbinder.h"
#include "opt_romset.h"

#include "ui_rsedit.h"
#include "ui_setupwin.h"
#include "ui_umadial.h"
#include "ui_layedit.h"

class SetupWin : public QDialog {
	Q_OBJECT
	public:
		unsigned block:1;
		unsigned prfChanged:1;

		SetupWin(QWidget*);
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

		int eidx;
		void editRomset();

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
	public slots:
		void start(xProfile*);
	private slots:
		void reject();
		void apply();
		void okay();
		void buildrsetlist();
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
		void updvolumes();
		void chablock(QModelIndex);
		void setTapeBreak(int,int);
		void hddcap();

		void selSDCimg();

		void openSlot();
		void ejectSlot();

		void editrset();
		void addNewRomset();
		void rmRomset();
		void rscomplete(int, QString);

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
		void rmProfile();

		void edLayout();
		void addNewLayout();
		void delLayout();
		void layEditorChanged();
		void layEditorOK();
		void layNameCheck(QString);
};

#endif
