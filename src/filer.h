#ifndef _FILER_H
#define _FILER_H

#include <stdint.h>
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
		void loaddisk(std::string,uint8_t,bool);
		bool savedisk(std::string,uint8_t,bool);
		void savesnapshot(std::string,bool);
		void saveonf2();
		void opensomewhat();
		void loadsomefile(std::string,uint8_t);
	private:
		MFResult execute(QWidget*,QString,QString,QStringList);
};

extern MFiler *filer;

#endif
