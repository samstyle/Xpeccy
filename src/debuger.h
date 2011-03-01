#ifndef _DEBUGER_H
#define _DEBUGER_H

#include <QDialog>
#include <QLineEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QKeyEvent>

#define DASMROW 25
#define DMPSIZE 16

struct BPoint {
	BPoint(unsigned char pg,unsigned short ad) {page = pg; adr = ad;}
	BPoint() {}
	unsigned char page;		// ram (adr>0x3fff) or rom (adr<0x4000)
	unsigned short adr;		// addr (0x0000..0xffff);
};

struct DasmRow {
	ushort adr;
	QString bytes;
	QString dasm;
};

struct CatchPoint {
	bool active;
	ushort adr;
	ushort sp;
};

class DebugWin : public QDialog {
	Q_OBJECT
	public:
		DebugWin(QWidget*);
		QList<BPoint> bpoint;
		CatchPoint cpoint;
		bool active;
		void reject();
		void start();
		void stop();
		int findbp(BPoint);
		unsigned char getbpage(unsigned short);
		void switchbp(BPoint);
	private:
		QLineEdit *ledit;
		QGridLayout *rglay,*asmlay,*dmplay,*raylay,*vglay;
		QLabel *dbgscrn,*tlab;
		uchar rowincol[16];
		ushort upadr,adr,dmpadr;
		uchar curcol, currow;
		QList<DasmRow> fdasm;
		bool fillall();
		void fillregz();
		bool filldasm();
		void filldump();
		void fillrays();
		void fillvg();
		void showedit(QLabel*,QString);
		ushort getprevadr(ushort);
		DasmRow getdisasm();
		bool tmpb;
		unsigned int t;
	protected:
		void keyPressEvent(QKeyEvent*);
};

extern DebugWin *dbg;

#endif
