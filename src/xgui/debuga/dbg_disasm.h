#pragma once

#include <QTableView>
#include <QAbstractTableModel>
#include <QKeyEvent>
#include <QMouseEvent>

#include "../../libxpeccy/spectrum.h"
#include "../../xcore/xcore.h"
#include "../classes.h"

// memory cell type (bits 4..7)

typedef struct {
	unsigned ispc:1;		// address is PC
	unsigned issel:1;		// address inside selection
	unsigned isbrk:1;		// breakpoint here
	unsigned islab:1;		// address have label
	unsigned iscom:1;		// address have comment
	unsigned isequ:1;		// this is equ
	int adr;			// command addr (bus/cpu)
	int oadr;			// word operand like nn : jp nn; ld hl,(nn)
	int flag;			// address cell flags
	int oflag;			// opcode flag
	QString aname;			// label/segment/address
	QString bytes;			// all bytes inside command
	QString command;		// command with replaced addr->label
	QString info;			// memory argument if any
	QString icon;			// icon path if any
} dasmData;

class xDisasmModel : public xTableModel {
	Q_OBJECT
	public:
		xDisasmModel(QObject* = NULL);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		Qt::ItemFlags flags(const QModelIndex&) const;
		QVariant data(const QModelIndex&, int) const;
		bool setData(const QModelIndex&, const QVariant&, int);
		// void update();
		QList<dasmData> dasm;
		int asmadr;			// full memory address
	signals:
		void rqRefill();
		void s_comenter();
		void s_adrch(int, int);
	public slots:
		int update_lst();
	private:
		int fill();
};

class xDisasmTable : public QTableView {
	Q_OBJECT
	Q_PROPERTY(QColor pcbgr MEMBER pc_bgr NOTIFY colChanged)
	Q_PROPERTY(QColor pctxt MEMBER pc_txt NOTIFY colChanged)
	Q_PROPERTY(QColor blkbgr MEMBER blk_bgr NOTIFY colChanged)
	Q_PROPERTY(QColor blktxt MEMBER blk_txt NOTIFY colChanged)
	Q_PROPERTY(QColor brktxt MEMBER brk_txt NOTIFY colChanged)
	public:
		xDisasmTable(QWidget* = NULL);
		QVariant getData(int, int, int);
		int rows();
		void setMode(int, int);
		int getMode(int);
		int getAdr();
	signals:
		void rqRefill();
		void rqRefillAll();
		void s_adrch(int);
		void colChanged();
	public slots:
		int updContent();
		void update();
		void t_update(int, int);
		void setAdr(int, int = 0);
		void setAdrX(int);
		void rowDown();
	private slots:
		void updColors();
	private:
		int markAdr;
		xDisasmModel* model;
		QList<int> history;
		QColor pc_bgr,pc_txt,blk_bgr,blk_txt,brk_txt;

		void scrolUp(Qt::KeyboardModifiers = Qt::NoModifier);
		void scrolDn(Qt::KeyboardModifiers = Qt::NoModifier);

		void copyToCbrd();

		void keyPressEvent(QKeyEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
		void resizeEvent(QResizeEvent*);
};

QList<dasmData> getDisasm(Computer*, int&);
