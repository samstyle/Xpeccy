#pragma once

#include <QComboBox>
#include <QDialog>
#include <QFileSystemModel>
#include <QItemDelegate>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QTreeView>
#include <QWheelEvent>

#include "classes.h"

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
	#include <QRegExpValidator>
#else
	#include <QRegularExpressionValidator>
	typedef QRegularExpressionValidator QRegExpValidator;
#endif

// common

void shitHappens(const char*);
bool areSure(const char*);
int askYNC(const char*);
void showInfo(const char*);

int getRFIData(QComboBox*);
void setRFIndex(QComboBox*, QVariant, int = 0);

// subclasses

#define	XHS_BGR	1		// change background if value changed
#define	XHS_DEC	(1<<1)		// hex/dec switch enabled
#define XHS_FILL (1<<2)		// leading zeros
#define XHS_UPD (1<<3)		// force update text, reset after using

class xHexSpin : public QLineEdit {
	Q_OBJECT
	public:
		xHexSpin(QWidget* = NULL);
		void setValue(int);
		int getValue();
		void setXFlag(int);
		void setBase(int);
		void updatePal();
		int getMax();
	signals:
		void valueChanged(int);
	public slots:
		void setMin(int);
		void setMax(int);
	private slots:
		void onChange(int);
		void onTextChange(QString);
	private:
		unsigned changed:1;
		int hsflag;
		int base;
		int value;
		int min;
		int max;
		int len;
		QString vtxt;
		QRegExpValidator vldtr;
		void updateMask();
	protected:
		void keyPressEvent(QKeyEvent*);
		void wheelEvent(QWheelEvent*);
};

class xLabel : public QLabel {
	Q_OBJECT
	public:
		xLabel(QWidget* p = NULL);
		int id;
	signals:
		void clicked(QMouseEvent*);
	protected:
		void mousePressEvent(QMouseEvent*);
};

class xTreeBox : public QComboBox {
	Q_OBJECT
	public:
		xTreeBox(QWidget* p = NULL);
		void setDir(QString);
		void setCurrentFile(QString);
		QString currentFile();
	private:
		void showPopup();
		void hidePopup();
		QTreeView* tree;
		QFileSystemModel* mod;
};

enum {
	XTYPE_NONE = -1,
	XTYPE_ADR = 0,
	XTYPE_LABEL,
	XTYPE_DUMP,
	XTYPE_BYTE,
	XTYPE_OCTWRD,
};

class xItemDelegate : public QItemDelegate {
	public:
		xItemDelegate(int);
		int type;
		// QWidget* createEditor (QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;
	private:
		QRegExpValidator vld;
		QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;
};

// tape player

#include "options/opt_tapecat.h"
#include "ui_tapewin.h"
#include "../libxpeccy/tape.h"

enum {
	TW_STATE = 0,
	TW_REWIND,
	TW_BREAK
};

enum {
	TWS_PLAY = 1,
	TWS_REC,
	TWS_STOP,
	TWS_OPEN,
	TWS_REWIND
};

class TapeWin : public QDialog {
	Q_OBJECT
	public:
		TapeWin(QWidget*);
	public slots:
		void updProgress(Tape*);
		void updList(Tape*);
		void upd(Tape*);
		void show();
	private:
		Ui::TapeWin ui;
		int state;
	private slots:
		void doPlay();
		void doRec();
		void doStop();
		void doLoad();
		void doRewind();
		void doDClick(QModelIndex);
		void doClick(QModelIndex);
		void setSpeed(int);
};

// rzx player

#include "ui_rzxplayer.h"
#include "../libxpeccy/spectrum.h"

enum {
	RWS_PLAY = 1,
	RWS_STOP,
	RWS_PAUSE,
	RWS_OPEN
};

class RZXWin : public QDialog {
	Q_OBJECT
	public:
		RZXWin(QWidget*);
		void setProgress(int,int);
	public slots:
		void startPlay();
		void stop();
		void upd(Computer*);
	signals:
		void stateChanged(int);
	private:
		Ui::rzxPlayer ui;
		int state;
	private slots:
		void playPause();
		void open();
};
