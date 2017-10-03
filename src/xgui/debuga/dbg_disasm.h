#ifndef X_DBG_DISASM_H
#define X_DBG_DISASM_H

#include <QTableView>
#include <QAbstractTableModel>
#include <QKeyEvent>
#include <QMouseEvent>

#include "libxpeccy/spectrum.h"
#include "xcore/xcore.h"

// memory cell type (bits 4..7)
enum {
	DBG_VIEW_CODE = 0x00,
	DBG_VIEW_BYTE = 0x10,
	DBG_VIEW_WORD = 0x20,
	DBG_VIEW_ADDR = 0x30,
	DBG_VIEW_TEXT = 0x40
};

typedef struct {
	unsigned ispc:1;		// address is PC
	unsigned issel:1;		// address inside selection
	unsigned isbrk:1;		// breakpoint here
	unsigned islab:1;		// address have label
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
	signals:
		void rqRefill();
	public slots:
		int update();
	private:
		QList<dasmData> dasm;
		int fill();
};

class xDisasmTable : public QTableView {
	Q_OBJECT
	public:
		xDisasmTable(QWidget* = NULL);
		QVariant getData(int, int, int);
		int rows();
		int updContent();
		void setComp(Computer**);
		void setMode(int, int);
	private:
		int markAdr;
		xDisasmModel* model;
		Computer** cptr;
	signals:
		void rqRefill();
		void rqRefillAll();
	private:
		void scrolUp(Qt::KeyboardModifiers = 0);
		void scrolDn(Qt::KeyboardModifiers = 0);

		void keyPressEvent(QKeyEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);
};

dasmData getDisasm(Computer*, unsigned short&);

#endif
