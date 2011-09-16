#ifndef _DEBUGER_H
#define _DEBUGER_H

#include <QDialog>
#include <QLineEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QKeyEvent>

#include <stdint.h>

#define DASMROW 25
#define DMPSIZE 16

struct BPoint {
	BPoint(uint8_t pg,uint16_t ad) {page = pg; adr = ad;}
	BPoint() {}
	uint8_t page;		// ram (adr>0x3fff) or rom (adr<0x4000)
	uint16_t adr;		// addr (0x0000..0xffff);
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
		uint8_t getbpage(uint16_t);
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
		uint32_t t;
	protected:
		void keyPressEvent(QKeyEvent*);
};

void dbgInit(QWidget*);
void dbgShow();
bool dbgIsActive();
int dbgFindBreakpoint(BPoint);

#endif
