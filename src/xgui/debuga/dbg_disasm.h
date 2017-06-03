#ifndef _DBG_DISASM_H
#define _DBG_DISASM_H

#include <QTableView>
#include <QAbstractItemModel>
#include <QKeyEvent>
#include <QMouseEvent>

#include "../../libxpeccy/spectrum.h"
#include "../../xcore/xcore.h"

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
	unsigned char flag;		// address cell flags
	QString aname;			// label/segment/address
	QString bytes;			// all bytes inside command
	QString command;		// command with replace addr->label
	QString info;			// memory argument if any
	QString icon;			// icon path if any
} dasmData;

class xDisasmModel : public QAbstractItemModel {
	Q_OBJECT
	public:
		xDisasmModel(Computer**, QObject* = NULL);
		int rowCount(const QModelIndex& = QModelIndex()) const;
		int columnCount(const QModelIndex& = QModelIndex()) const;
		QModelIndex index(int, int, const QModelIndex& = QModelIndex()) const;
		QModelIndex parent(const QModelIndex& = QModelIndex()) const;
		Qt::ItemFlags flags(const QModelIndex&) const;
		QVariant data(const QModelIndex&, int) const;
		bool setData(const QModelIndex&, const QVariant&, int);
	public slots:
		int update();
	private:
		Computer** cptr;
		QList<dasmData> dasm;
		int fill();
};

class xDisasmTable : public QTableView {
	Q_OBJECT
	public:
		xDisasmTable(QWidget* = NULL);
		QVariant getData(int, int, int);
		Computer** cptr;
	private:
		int markAdr;
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
