#ifndef X_DBG_DISASM_H
#define X_DBG_DISASM_H

#include <QTableView>
#include <QAbstractTableModel>
#include <QKeyEvent>
#include <QMouseEvent>

#include "libxpeccy/spectrum.h"
#include "xcore/xcore.h"

// memory cell type (bits 4..7)

typedef struct {
	unsigned ispc:1;		// address is PC
	unsigned issel:1;		// address inside selection
	unsigned isbrk:1;		// breakpoint here
	unsigned islab:1;		// address have label
	unsigned isequ:1;		// this is equ
	unsigned short adr;		// command addr
	int oadr;			// word operand like nn : jp nn; ld hl,(nn)
	int flag;			// address cell flags
	int oflag;			// opcode flag
	QString aname;			// label/segment/address
	QString bytes;			// all bytes inside command
	QString command;		// command with replace addr->label
	QString info;			// memory argument if any
	QString icon;			// icon path if any
} dasmData;

class xDisasmModel : public QAbstractTableModel {
	Q_OBJECT
	public:
		xDisasmModel(QObject* = NULL);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		Qt::ItemFlags flags(const QModelIndex&) const;
		QVariant data(const QModelIndex&, int) const;
		bool setData(const QModelIndex&, const QVariant&, int);
		Computer** cptr;
		QList<dasmData> dasm;
	signals:
		void rqRefill();
		void s_adrch(int, int);
	public slots:
		int update();
	private:
		int fill();
};

class xDisasmTable : public QTableView {
	Q_OBJECT
	public:
		xDisasmTable(QWidget* = NULL);
		QVariant getData(int, int, int);
		int rows();
		void setComp(Computer**);
		void setMode(int, int);
		int getMode(int);
	signals:
		void rqRefill();
		void rqRefillAll();
	public slots:
		int updContent();
		void t_update(int, int);
	private:
		int markAdr;
		xDisasmModel* model;
		Computer** cptr;
		QList<unsigned short> history;

		void scrolUp(Qt::KeyboardModifiers = 0);
		void scrolDn(Qt::KeyboardModifiers = 0);

		void keyPressEvent(QKeyEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
		void resizeEvent(QResizeEvent*);
};

QList<dasmData> getDisasm(Computer*, unsigned short&);

#endif
