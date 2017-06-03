#include "dbg_disasm.h"

#include <QDebug>

extern unsigned short disasmAdr;
extern int blockStart;
extern int blockEnd;

extern QColor colPC;
extern QColor colBRK;
extern QColor colSEL;

extern QMap<QString, xAdr> labels;
QString findLabel(int, int, int);

// MODEL

xDisasmModel::xDisasmModel(Computer** pt, QObject* p):QAbstractItemModel(p) {
	cptr = pt;
}

QModelIndex xDisasmModel::index(int row, int col, const QModelIndex& idx) const {
	return createIndex(row, col);
}

QModelIndex xDisasmModel::parent(const QModelIndex& idx) const {
	return QModelIndex();
}

int xDisasmModel::columnCount(const QModelIndex& idx) const {
	return 4;
}

int xDisasmModel::rowCount(const QModelIndex& idx) const {
	return 19;
}

QVariant xDisasmModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if (row < 0) return res;
	if (col < 0) return res;
	if (row >= rowCount()) return res;
	if (col >= columnCount()) return res;
	if (row >= dasm.size()) return res;
	QFont font;
	switch(role) {
		case Qt::FontRole:
			if ((col == 0) && dasm[row].islab) {
				font = QFont();
				font.setBold(true);
				res = font;
			}
			break;
		case Qt::TextColorRole:
			if (!dasm[row].islab && conf.dbg.hideadr && (col == 0))
				res = QColor(Qt::gray);
			break;
		case Qt::BackgroundColorRole:
			if (dasm[row].isbrk && (col == 3)) {
				res = colBRK;			// breakpoint
			} else if (dasm[row].ispc) {
				res = colPC;			// pc
			} else if (dasm[row].issel) {
				res = colSEL;			// selected
			}
			break;
		case Qt::DecorationRole:
			if ((col == 3) && !dasm[row].icon.isEmpty())
				res = QIcon(dasm[row].icon);
			break;
		case Qt::TextAlignmentRole:
			if (col == 3) res = Qt::AlignRight;
			break;
		case Qt::DisplayRole:
			switch(col) {
				case 0: res = dasm[row].aname; break;
				case 1: res = dasm[row].bytes; break;
				case 2: res = dasm[row].command; break;
				case 3: res = dasm[row].info; break;
			}
			break;
		case Qt::UserRole:
			switch (col) {
				case 0: res = dasm[row].adr; break;
				case 2: res = dasm[row].oadr; break;
			}

			break;
	}
	return res;
}

unsigned char dasmrd(unsigned short adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	return memRd(comp->mem, adr);
}

QMap<int, QString> mtName = {
	{MEM_RAM, "RAM"},
	{MEM_ROM, "ROM"},
	{MEM_SLOT, "SLT"},
	{MEM_EXT, "EXT"}
};

void placeLabel(dasmData& drow) {
	QString lab = findLabel(drow.oadr, -1, -1);
	if (lab.isEmpty()) return;
	QString num = QString::number(drow.oadr, 16).prepend("#");
	drow.command.replace(num, lab);
}

int dasmByte(Computer* comp, unsigned short adr, dasmData& drow) {
	drow.command = QString("DB #%0").arg(gethexbyte(memRd(comp->mem, adr)));
	return 1;
}

int dasmWord(Computer* comp, unsigned short adr, dasmData& drow) {
	int word = memRd(comp->mem, adr);
	word |= (memRd(comp->mem, adr + 1) << 8);
	drow.command = QString("DW #%0").arg(gethexword(word));
	return 2;
}

int dasmAddr(Computer* comp, unsigned short adr, dasmData& drow) {
	int word = memRd(comp->mem, adr);
	word |= (memRd(comp->mem, adr + 1) << 8);
	QString lab = findLabel(word & 0xffff, -1, -1);
	if (lab.isEmpty()) {
		lab = gethexword(word).prepend("#");
	}
	drow.command = QString("DW %0").arg(lab);
	placeLabel(drow);
	return 2;
}

int dasmText(Computer* comp, unsigned short adr, dasmData& drow) {
	int clen = 0;
	drow.command = QString("DB \"");
	unsigned char fl = getBrk(comp, adr);
	unsigned char bt = memRd(comp->mem, adr);
	while (((fl & 0xf0) == DBG_VIEW_TEXT) && (bt > 31) && (bt < 128) && (clen < 250)) {
		drow.command.append(QChar(bt));
		clen++;
		bt = memRd(comp->mem, (adr + clen) & 0xffff);
		fl = getBrk(comp, (adr + clen) & 0xffff);
	}
	if (clen == 0) {
		drow.flag = (getBrk(comp, adr) & 0x0f) | DBG_VIEW_BYTE;
		setBrk(comp, adr, drow.flag);
		clen = 1;
		drow.command = QString("DB #%0").arg(memRd(comp->mem, adr));
	} else {
		drow.command.append("\"");
	}
	return clen;
}

int dasmCode(Computer* comp, unsigned short adr, dasmData& drow) {
	char buf[1024];
	xMnem mnm = cpuDisasm(comp->cpu, adr, buf, dasmrd, comp);
	drow.command = QString(buf).toUpper();
	drow.oadr = mnm.oadr;
	placeLabel(drow);
	if (drow.ispc) {
		if (mnm.mem) {
			drow.info = gethexbyte(mnm.mop);
		} else if (mnm.cond && mnm.met && (drow.oadr >= 0)) {
			if (drow.adr < drow.oadr) {
				drow.icon = QString(":/images/arrdn.png");
			} else if (drow.adr > drow.oadr) {
				drow.icon = QString(":/images/arrup.png");
			} else {
				drow.icon = QString(":/images/redCircle.png");
			}
		}
	}
	return mnm.len;
}

int dasmSome(Computer* comp, unsigned short adr, dasmData& drow) {
	int clen = 0;
	drow.adr = adr;
	drow.flag = getBrk(comp, drow.adr);
	drow.oadr = -1;
	switch(drow.flag & 0xf0) {
		case DBG_VIEW_WORD: clen = dasmWord(comp, adr, drow); break;
		case DBG_VIEW_ADDR: clen = dasmAddr(comp, adr, drow); break;
		case DBG_VIEW_TEXT: clen = dasmText(comp, adr, drow); break;
		case DBG_VIEW_CODE: clen = dasmCode(comp, adr, drow); break;
		default: clen = dasmByte(comp, adr, drow); break;		// DBG_VIEW_BYTE
	}
	return clen;
}

dasmData getDisasm(Computer* comp, unsigned short& adr) {
	dasmData drow;
	drow.adr = adr;
	drow.flag = getBrk(comp, drow.adr);
	drow.ispc = (adr == comp->cpu->pc) ? 1 : 0;
	drow.isbrk = (drow.flag & MEM_BRK_ANY) ? 1 : 0;
	drow.issel = ((adr >= blockStart) && (adr <= blockEnd)) ? 1 : 0;
	drow.info.clear();
	drow.icon.clear();
	int clen = 0;
	// 0:adr
	xAdr xadr = memGetXAdr(comp->mem, adr);
	QString lab;
	if (conf.dbg.labels) {
		lab = findLabel(xadr.adr, xadr.type, xadr.bank);
	}
	if (lab.isEmpty()) {
		drow.islab = 0;
		if (conf.dbg.segment) {
			drow.aname = QString("%0:%1:%2").arg(mtName[xadr.type]).arg(gethexbyte(xadr.bank)).arg(gethexword(xadr.adr & 0x3fff));
		} else {
			drow.aname = gethexword(adr);
		}
	} else {
		drow.islab = 1;
		drow.aname = lab;
	}
	// 2:command / 3:info
	clen = dasmSome(comp, adr, drow);
	// 1:bytes
	drow.bytes.clear();
	while(clen > 0) {
		drow.bytes.append(gethexbyte(memRd(comp->mem, adr)));
		adr++;
		clen--;
	}
	return drow;
}

int xDisasmModel::fill() {
	dasmData drow;
	int row;
	unsigned short adr = disasmAdr;
	int res = 0;
	dasm.clear();
	for(row = 0; row < rowCount(); row++) {
		drow = getDisasm(*cptr, adr);
		dasm.append(drow);
		res |= drow.ispc;
	}
	return res;
}

int xDisasmModel::update() {
	int res = fill();
	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
	return res;
}

Qt::ItemFlags xDisasmModel::flags(const QModelIndex& idx) const {
	Qt::ItemFlags res = QAbstractItemModel::flags(idx);
	if (idx.column() < 3)
		res |= Qt::ItemIsEditable;
	return res;
}

unsigned short getPrevAdr(Computer* comp, unsigned short adr) {
	dasmData drow;
	int i;
	for(i = 16; i > 0; i--) {
		if (dasmSome(comp, (adr - i) & 0xffff, drow) == i) {
			adr = adr - i;
			break;
		}
	}
	if (i == 0)
		adr--;
	return adr;
}

int asmAddr(QVariant val, xAdr xadr) {
	QString str = val.toString();
	QString lab;
	int res = -1;
	int adr;
	bool flag;
	if (str.isEmpty()) {
		str = findLabel(xadr.adr, xadr.type, xadr.bank);
		if (!str.isEmpty())
			labels.remove(str);
	} else {
		adr = str.toInt(&flag, 16);
		if (flag) {
			res = adr;
		} else {
			// if there is such label
			if (labels.contains(str)) {
				res = labels[str].adr;
			} else {
				lab = findLabel(xadr.adr, xadr.type, xadr.bank);
				if (!lab.isEmpty())
					labels.remove(lab);
				labels[str] = xadr;
			}
		}
	}
	return res;
}

bool xDisasmModel::setData(const QModelIndex& cidx, const QVariant& val, int role) {
	if (!cidx.isValid()) return false;
	if (role != Qt::EditRole) return false;
	int row = cidx.row();
	int col = cidx.column();
	if ((row < 0) || (row >= rowCount())) return false;
	if ((col < 0) || (col >= columnCount())) return false;
	bool flag;
	unsigned char cbyte;
	char buf[256];
	unsigned char* ptr;
	int idx;
	int len;
	QString str;
	QString lab;
	QStringList lst;
	xAdr xadr;
	int adr = dasm[row].adr;
	switch(col) {
		case 0:
			xadr = memGetXAdr((*cptr)->mem, dasm[row].adr);
			idx = asmAddr(val, xadr);
			if (idx >= 0) {
				while (row > 0) {
					idx = getPrevAdr(*cptr, idx);
					row--;
				}
				disasmAdr = idx;
			}
			update();
			break;
		case 1:	// bytes
			lst = val.toString().split(":", QString::SkipEmptyParts);
			foreach(str, lst) {
				cbyte = str.toInt(NULL,16);
				memWr((*cptr)->mem, adr, cbyte);
				adr++;
			}
			break;
		case 2:	// command
			ptr = getBrkPtr(*cptr, adr & 0xffff);
			str = val.toString();
			if (!str.startsWith("db \"", Qt::CaseInsensitive))		// #NUM -> 0xNUM if not db "..."
				str.replace("#", "0x");
			if (str.startsWith("db ", Qt::CaseInsensitive)) {		// db
				if ((str.at(3) == '"') && str.endsWith("\"")) {		// db "text"
					str = str.mid(4, str.size() - 5);
					idx = 0;
					cbyte = str.at(idx).cell();
					while ((idx < 250) && (idx < str.size()) && (cbyte > 31) && (cbyte < 128)) {
						buf[idx] = cbyte;
						*ptr &= 0x0f;
						*ptr |= DBG_VIEW_TEXT;
						ptr++;
						idx++;
						cbyte = str.at(idx).cell();
					}
					len = idx;
				} else {						// db n
					str = str.mid(3);
					idx = str.toInt(&flag, 0);
					if (flag) {
						len = 1;
						buf[0] = idx & 0xff;
						*ptr &= 0x0f;
						*ptr |= DBG_VIEW_BYTE;
					} else {
						len = 0;
					}
				}
			} else if (str.startsWith("dw ", Qt::CaseInsensitive)) {	// word/addr
				str = str.mid(3);
				if (labels.contains(str)) {				// check label
					idx = labels[str].adr;
					*ptr &= 0x0f;
					*ptr |= DBG_VIEW_ADDR;
					ptr++;
					*ptr &= 0x0f;
					*ptr |= DBG_VIEW_ADDR;
					len = 2;
				} else {
					idx = str.toInt(&flag, 0);
					if (flag) {
						len = 2;
						*ptr &= 0x0f;
						*ptr |= DBG_VIEW_WORD;
						ptr++;
						*ptr &= 0x0f;
						*ptr |= DBG_VIEW_WORD;
					} else {
						len = 0;
					}
				}
				if (len > 0) {
					buf[0] = idx & 0xff;
					buf[1] = (idx >> 8) & 0xff;
				}
			} else {			// byte
				len = cpuAsm((*cptr)->cpu, str.toLocal8Bit().data(), buf, adr);
				if (len > 0) {
					for(idx = 0; idx < len; idx++) {
						*ptr &= 0x0f;
						*ptr |= DBG_VIEW_CODE;
					}
				}
			}
			idx = 0;
			while (idx < len) {
				memWr((*cptr)->mem, (adr + idx) & 0xffff, buf[idx]);
				idx++;
			}
			break;
	}
	update();
	return true;
}

// TABLE

xDisasmTable::xDisasmTable(QWidget* p):QTableView(p) {
}

QVariant xDisasmTable::getData(int row, int col, int role) {
	return model()->data(model()->index(row, col), role);
}

void xDisasmTable::keyPressEvent(QKeyEvent* ev) {
	QModelIndex idx = currentIndex();
	switch (ev->key()) {
		case Qt::Key_Up:
			if ((ev->modifiers() & Qt::ControlModifier) || (idx.row() == 0)) {
				scrolUp(ev->modifiers());
			} else {
				QTableView::keyPressEvent(ev);
			}
			break;
		case Qt::Key_Down:
			if ((ev->modifiers() & Qt::ControlModifier) || (idx.row() == model()->rowCount() - 1)) {
				scrolDn(ev->modifiers());
			} else {
				QTableView::keyPressEvent(ev);
			}
			break;
		case Qt::Key_Home:
			disasmAdr = (*cptr)->cpu->pc;
			emit rqRefill();
			ev->ignore();
			break;
		case Qt::Key_End:
			(*cptr)->cpu->pc = getData(idx.row(), 0, Qt::UserRole).toInt();
			emit rqRefillAll();
			ev->ignore();
			break;
		default:
			QTableView::keyPressEvent(ev);
			break;
	}
}

void xDisasmTable::mousePressEvent(QMouseEvent* ev) {
	int row = rowAt(ev->pos().y());
	if ((row < 0) || (row >= model()->rowCount())) return;
	int adr = getData(row, 0, Qt::UserRole).toInt();
	switch (ev->button()) {
		case Qt::MiddleButton:
			blockStart = -1;
			blockEnd = -1;
			emit rqRefill();
			ev->ignore();
			break;
		case Qt::LeftButton:
			if (ev->modifiers() & Qt::ControlModifier) {
				blockStart = adr;
				if (blockEnd < blockStart) blockEnd = blockStart;
				emit rqRefill();
				ev->ignore();
			} else if (ev->modifiers() & Qt::ShiftModifier) {
				blockEnd = adr;
				if (blockStart > blockEnd) blockStart = blockEnd;
				if (blockStart < 0) blockStart = 0;
				emit rqRefill();
				ev->ignore();
			} else {
				markAdr = adr;
				QTableView::mousePressEvent(ev);
			}
			break;
		default:
			QTableView::mousePressEvent(ev);
			break;
	}
}

void xDisasmTable::mouseReleaseEvent(QMouseEvent* ev) {
	if (ev->button() == Qt::LeftButton)
		markAdr = -1;
}

void xDisasmTable::mouseMoveEvent(QMouseEvent* ev) {
	int row = rowAt(ev->pos().y());
	if ((row < 0) || (row >= model()->rowCount())) return;
	int adr = model()->data(model()->index(row, 0), Qt::UserRole).toInt();	// item(row,0)->data(Qt::UserRole).toInt();
	if ((ev->modifiers() == Qt::NoModifier) && (ev->buttons() & Qt::LeftButton) && (adr != blockStart) && (adr != blockEnd) && (markAdr >= 0)) {
		if (adr < blockStart) {
			blockStart = adr;
			blockEnd = markAdr;
		} else {
			blockStart = markAdr;
			blockEnd = adr;
		}
		emit rqRefill();
	}
	QTableView::mouseMoveEvent(ev);
}

void xDisasmTable::scrolDn(Qt::KeyboardModifiers mod) {
	if (mod & Qt::ControlModifier) {
		disasmAdr++;
	} else {
		disasmAdr = getData(1, 0, Qt::UserRole).toInt() & 0xffff;
	}
	emit rqRefill();
}

void xDisasmTable::scrolUp(Qt::KeyboardModifiers mod) {
	if (mod & Qt::ControlModifier) {
		disasmAdr--;
	} else {
		disasmAdr = getPrevAdr(*cptr, disasmAdr);
	}
	emit rqRefill();
}

void xDisasmTable::wheelEvent(QWheelEvent* ev) {
	if (ev->delta() < 0) {
		scrolDn(ev->modifiers());
	} else if (ev->delta() > 0) {
		scrolUp(ev->modifiers());
	}
}
