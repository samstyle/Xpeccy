#ifndef _SETWIN_H
#define _SETWIN_H

#include <QDialog>
#include <QModelIndex>

class SetupWin : public QDialog {
	Q_OBJECT
	public:
		SetupWin(QWidget*);
	public slots:
		void start();
	private slots:
		void reject();
		void apply();
		void okay();
		void buildrsetlist();
		void setmszbox(int);
		void selsspath();
		void chabsz();
		void chabrg();
		void updatedisknams();
		void loada(); void loadb(); void loadc(); void loadd();
		void savea(); void saveb(); void savec(); void saved();
		void ejcta(); void ejctb(); void ejctc(); void ejctd();
		void newa(); void newb(); void newc(); void newd(); void newdisk(int);
		void loatape(); void savtape(); void ejctape();
		void tblkup(); void tblkdn(); void tblkrm();
		void hddMasterImg(); void hddSlaveImg();
		void ssjapath(); void sprjpath();
		void umadd(); void umdel(); void umup(); void umdn();
		void umedit(QModelIndex);
		void umaselp(); void umaconf();
		void updvolumes();
		void updfrq();
		void chablock(QModelIndex);
		void setTapeBreak(int,int);
		void hddcap();
		void editrset(QModelIndex);
		void setrpart();
		void hidersedit();
		void addNewRomset();
		void rmRomset();
		void recheck_single(bool);
		void recheck_separate(bool);
		void fillDiskCat();
		void copyToTape();
		void copyToDisk();
		void diskToHobeta();
		void diskToRaw();
		void addJoyBind();
		void delJoyBind();
		void scanJoyBind();
		void newProfile();
		void rmProfile();
	private:
		QDialog *umadial;
		void buildtapelist();
		void buildmenulist();
		void buildkeylist();
		void buildjmaplist();
		void buildproflist();
		int umidx;
};

void optInit(QWidget*);
void optShow();

#endif
