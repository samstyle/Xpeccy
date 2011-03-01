#ifndef _FILER_H
#define _FILER_H

#include <QFileDialog>

struct MFResult {
	QString selfile;
	QString selfilt;
	int fidx;
};

class MFiler : public QFileDialog {
	public:
		MFiler(QWidget*);
		MFResult open(QWidget*,QString,QString,QStringList);
		MFResult save(QWidget*,QString,QString,QStringList);
		QDir lastdir;
		void loadtape(std::string,bool);
		void savetape(std::string,bool);
		void loaddisk(std::string,unsigned char,bool);
		bool savedisk(std::string,unsigned char,bool);
		void savesnapshot(std::string,bool);
		void saveonf2();
		void opensomewhat();
		void loadsomefile(std::string,unsigned char);
	private:
		MFResult execute(QWidget*,QString,QString,QStringList);
};

extern MFiler *filer;

#endif
