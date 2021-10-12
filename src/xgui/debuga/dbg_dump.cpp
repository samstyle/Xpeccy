#include "dbg_dump.h"
#include "../../xcore/xcore.h"

#include <QDebug>

#include <QTextCodec>
#include <QColor>

extern QColor colBRK;
extern QColor colSEL;

extern unsigned int blockStart;
extern unsigned int blockEnd;

QString gethexbyte(unsigned char);
QString gethexword(int);

// MODEL

QString getDumpString(QByteArray bts, int cp) {
	QString res;
	QTextCodec* codec = NULL;
	switch(cp) {
		case XCP_1251: codec = QTextCodec::codecForName("CP1251"); break;
		case XCP_866: codec = QTextCodec::codecForName("IBM866"); break;
		case XCP_KOI8R: codec = QTextCodec::codecForName("KOI8R"); break;
	}
	unsigned char bte;
	for (int i = 0; i < 8; i++) {
		bte = (unsigned char)bts.at(i);
		if (bte > 31) {
			res.append(QChar(bte));
		} else {
			res.append(".");
		}
	}
	if (codec == NULL) return res;
	QByteArray arr = res.toLatin1();
	res = codec->toUnicode(arr);
	return res;
}

int xDumpModel::mrd(int adr) const {
	Computer* comp = *cptr;
	MemPage* pg;
	int fadr;
	int res = 0xff;
	if (comp->cpu->type == CPU_I80286) {
		res = comp->hw->mrd(comp, adr, 0);
	} else {
		switch(mode) {
			case XVIEW_CPU:
				pg = &comp->mem->map[(adr >> 8) & 0xff];
				fadr = (pg->num << 8) | (adr & 0xff);
				switch (pg->type) {
					case MEM_ROM: res = comp->mem->romData[fadr & comp->mem->romMask]; break;
					case MEM_RAM: res = comp->mem->ramData[fadr & comp->mem->ramMask]; break;
					case MEM_SLOT: res = memRd(comp->mem, adr % maxadr);
						break;
				}
				res |= getBrk(comp, adr % maxadr) << 8;
				break;
			case XVIEW_RAM:
				adr &= 0x3fff;
				adr |= (page << 14);
				res = comp->mem->ramData[adr & 0x3fffff];
				res |= comp->brkRamMap[adr & 0x3fffff] << 8;
				break;
			case XVIEW_ROM:
				adr &= 0x3fff;
				adr |= (page << 14);
				res = comp->mem->romData[adr & 0x7ffff];
				res |= comp->brkRomMap[adr & 0x7ffff] << 8;
				break;
		}
	}
	return res;
}

void xDumpModel::mwr(int adr, unsigned char bt) {
	Computer* comp = *cptr;
	MemPage* pg;
	int fadr;
	if (comp->cpu->type == CPU_I80286) {
		comp->hw->mwr(comp, adr, bt);
	} else {
		switch(mode) {
			case XVIEW_CPU:
				pg = &comp->mem->map[(adr >> 8) & 0xff];
				fadr = (pg->num << 8) | (adr & 0xff);
				switch (pg->type) {
					case MEM_ROM:
						if (conf.dbg.romwr)
							comp->mem->romData[fadr & comp->mem->romMask] = bt;
						break;
					case MEM_RAM:
						comp->mem->ramData[fadr & comp->mem->ramMask] = bt;
						break;
					case MEM_SLOT:
						break;
				}
				break;
			case XVIEW_RAM:
				adr &= 0x3fff;
				adr |= (page << 14);
				comp->mem->ramData[adr & comp->mem->ramMask] = bt;
				break;
			case XVIEW_ROM:
				if (conf.dbg.romwr)
					comp->mem->romData[((adr & 0x3fff) | (page << 14)) & comp->mem->romMask] = bt;
				break;
		}
	}
}

xDumpModel::xDumpModel(QObject* par):QAbstractTableModel(par) {
	codePage = XCP_1251;
	mode = XVIEW_CPU;
	page = 0;
	dmpadr = 0;
	maxadr = MEM_64K;
	row_count = 11;
}

void xDumpModel::setComp(Computer** ptr) {
	cptr = ptr;
}

void xDumpModel::setMode(int md, int pg) {
	mode = md;
	page = pg;
	update();
}

void xDumpModel::setView(int t) {
	view = t;
	update();
}

void xDumpModel::setRows(int r) {
	if (r < row_count) {
		emit beginRemoveRows(QModelIndex(), r, row_count);
		row_count = r;
		emit endRemoveRows();
	} else if (r > row_count) {
		emit beginInsertRows(QModelIndex(), row_count, r);
		row_count = r;
		emit endInsertRows();
	}
}

int xDumpModel::rowCount(const QModelIndex&) const {
	return row_count;
}

int xDumpModel::columnCount(const QModelIndex&) const {
	return 10;
}

void xDumpModel::update() {
	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void xDumpModel::updateCell(int row, int col) {
	emit dataChanged(index(row, col), index(row, col));
}

void xDumpModel::updateRow(int row) {
	emit dataChanged(index(row, 0), index(row, columnCount() - 1));
}

void xDumpModel::updateColumn(int col) {
	emit dataChanged(index(0, col), index(rowCount() - 1, col));
}

int check_seg(int adr, xSegPtr seg) {
	if (adr < seg.base) return 0;
	if (adr - seg.base > seg.limit) return 0;
	return 1;
}

QVariant xDumpModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	QString str;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if (row < 0) return res;
	if (row >= rowCount()) return res;
	if (col < 0) return res;
	if (col >= columnCount()) return res;
	unsigned int adr = (dmpadr + (row << 3)) % maxadr;
	unsigned int cadr = (adr + col - 1) % maxadr;
	unsigned short wrd;
	int flg = mrd(cadr) >> 8;
	QByteArray arr;
	QColor clr;
	switch(role) {
		case Qt::BackgroundColorRole:
			if ((col > 0) && (col < 9)) {
				if ((cadr >= blockStart) && (cadr <= blockEnd) && (mode == XVIEW_CPU)) {	// selection
					clr = conf.pal["dbg.sel.bg"];
				} else {
					clr = conf.pal["dbg.table.bg"];
				}
			} else {
				clr = conf.pal["dbg.table.bg"];
			}
			if (clr.isValid())
				res = clr;
			break;
		case Qt::ForegroundRole:
			if ((col > 0) && (col < 9)) {
				if (flg & 0x0f) {								// breakpoint
					clr = conf.pal["dbg.brk.txt"];
				} else if ((cadr >= blockStart) && (cadr <= blockEnd) && (mode == XVIEW_CPU)) {	// selection
					clr = conf.pal["dbg.sel.txt"];
				} else {
					clr = conf.pal["dbg.table.txt"];
				}
			} else {
				clr = conf.pal["dbg.table.txt"];
			}
			if (clr.isValid())
				res = clr;
			break;
		case Qt::TextAlignmentRole:
			switch (col) {
				case 0: break;
				case 9: res = Qt::AlignRight; break;
				default: res = Qt::AlignCenter; break;
			}
			break;
		case Qt::EditRole:
			switch(col) {
				case 0:
					if ((mode == XVIEW_RAM) || (mode == XVIEW_ROM))
						adr &= 0x3fff;
					switch(view) {
						case XVIEW_OCTWRD:
							res = QString::number(adr, 8).rightJustified(6, '0');
							break;
						default:
							res = QString::number(adr, 16).toUpper();
							break;
					}
					break;
				case 9: break;
				default:
					switch(view) {
						case XVIEW_OCTWRD:
							wrd = mrd(cadr) & 0xff;
							wrd += (mrd((cadr + 1) % maxadr) << 8) & 0xff00;
							res = QString::number(wrd, 8).rightJustified(6,'0');
							break;
						default:
							res = gethexbyte(mrd(cadr) & 0xff);
							break;
					}
					break;
			}
			break;
		case Qt::DisplayRole:
			switch(col) {
				case 0:
					if ((*cptr)->cpu->type == CPU_I80286) {
						adr %= maxadr;
						if (!conf.dbg.segment) {
							res = QString::number(adr, 16).toUpper().rightJustified(6 , '0');
						} else if (check_seg(adr, (*cptr)->cpu->cs)) {
							adr -= (*cptr)->cpu->cs.base;
							res = QString("CS:").append(gethexword(adr & 0xffff));
						} else if (check_seg(adr, (*cptr)->cpu->ss)) {
							adr -= (*cptr)->cpu->ss.base;
							res = QString("SS:").append(gethexword(adr & 0xffff));
						} else if (check_seg(adr, (*cptr)->cpu->ds)) {
							adr -= (*cptr)->cpu->ds.base;
							res = QString("DS:").append(gethexword(adr & 0xffff));
						} else if (check_seg(adr, (*cptr)->cpu->es)) {
							adr -= (*cptr)->cpu->es.base;
							res = QString("ES:").append(gethexword(adr & 0xffff));
						} else {
							res = QString::number(adr, 16).toUpper().rightJustified(6 , '0');
						}
						// res = QString("DS:%0").arg(gethexword(adr & 0xffff));
					} else {
						if ((mode == XVIEW_RAM) || (mode == XVIEW_ROM)) {
							adr &= 0x3fff;
							str = QString::number(page, (*cptr)->hw->base).toUpper().rightJustified(2, '0').append(":");
						}
						str.append(QString::number(adr, (*cptr)->hw->base).toUpper().rightJustified(((*cptr)->hw->base == 16) ? 4 : 6, '0'));
						res = str;
					}
					break;
				case 9:
					for(int i = 0; i < 8; i++)
						arr.append(mrd((adr + i) % maxadr) & 0xff);
					res = getDumpString(arr, codePage);
					break;
				default:
					switch (view) {
						case XVIEW_OCTWRD:
							wrd = mrd(cadr) & 0xff;
							wrd += (mrd((cadr + 1) % maxadr) << 8) & 0xff00;
							res = QString::number(wrd, 8).rightJustified(6,'0');
							break;
						default:
							res = gethexbyte(mrd(cadr) & 0xff);
							break;
					}
					break;
			}
			break;
	}
	return res;
}

Qt::ItemFlags xDumpModel::flags(const QModelIndex& idx) const {
	Qt::ItemFlags res = QAbstractItemModel::flags(idx);
	if (!idx.isValid()) return res;
	if (idx.column() < columnCount())
		res |= Qt::ItemIsEditable;
	return res;
}

extern int str_to_adr(Computer* comp, QString str);

bool xDumpModel::setData(const QModelIndex& idx, const QVariant& val, int role) {
	if (!idx.isValid()) return false;
	if (role != Qt::EditRole) return false;
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= rowCount())) return false;
	if ((col < 0) || (col >= columnCount())) return false;
	int fadr;
	int adr = (dmpadr + (row << 3)) % maxadr;
	int nadr;
	unsigned char bt;
	QString str = val.toString();
	if (col == 0) {
		fadr = str_to_adr(*cptr, str);
		if (fadr >= 0) {
			dmpadr = (fadr - (row << 3)) % maxadr;
			update();
			emit s_adrch(dmpadr);
			emit rqRefill();
		}
	} else if (col < 9) {
		nadr = (adr + col - 1) % maxadr;
		switch(view) {
			case XVIEW_OCTWRD:
				fadr = str.toInt(NULL, 8) & 0xffff;
				mwr(nadr++, fadr & 0xff);
				mwr(nadr, (fadr >> 8) & 0xff);
				break;
			default:
				bt = str.toInt(NULL, 16) & 0xff;
				mwr(nadr, bt);
				break;
		}
		updateRow(row);
		emit rqRefill();
	}
	return true;
}

// TABLE

xDumpTable::xDumpTable(QWidget* p):QTableView(p) {
	markAdr = -1;
	model = new xDumpModel();
	setModel(model);

	connect(model, SIGNAL(rqRefill()), this, SIGNAL(rqRefill()));
	connect(model, SIGNAL(s_adrch(int)), this, SIGNAL(s_adrch(int)));
}

void xDumpTable::setComp(Computer** ptr) {
	cptr = ptr;
	model->setComp(ptr);
}

void xDumpTable::setMode(int md, int pg) {
	mode = md;
	model->setMode(md, pg);

	emit rqRefill();
}

void xDumpTable::setView(int t) {
	view = t;
	switch(t) {
		case XVIEW_OCTWRD:
			for (int i = 0; i < model->rowCount(); i++) {
				setSpan(i, 1, 1, 2);
				setSpan(i, 3, 1, 2);
				setSpan(i, 5, 1, 2);
				setSpan(i, 7, 1, 2);
			}
			break;
		default:
			clearSpans();
			break;
	}
	model->setView(t);
}

int xDumpTable::rows() {
	return model->rowCount();
}

void xDumpTable::setCodePage(int cp) {
	model->codePage = cp;
	model->updateColumn(9);
}

void xDumpTable::update() {
	model->update();
	QTableView::update();
}

void xDumpTable::setLimit(unsigned int lim) {
	if (model->maxadr != lim) {
		model->dmpadr %= lim;
		model->maxadr = lim;
		update();
	}
}

unsigned int xDumpTable::limit() {
	return model->maxadr;
}

void xDumpTable::setAdr(int adr) {
	if (model->dmpadr != (unsigned int)adr) {
		model->dmpadr = adr % model->maxadr;
		emit s_adrch(model->dmpadr);
		update();
	}
}

int xDumpTable::getAdr() {
	return model->dmpadr;
}

void xDumpTable::resizeEvent(QResizeEvent* ev) {
	int h = ev->size().height();
	if (h < 1) return;
	int rh = verticalHeader()->defaultSectionSize();
	int rc = h / rh;
	model->setRows(rc);
	update();
}

void xDumpTable::keyPressEvent(QKeyEvent* ev) {
	QModelIndex idx = currentIndex();
	switch(ev->key()) {
		case Qt::Key_Up:
			if (idx.row() > 0) {
				QTableView::keyPressEvent(ev);
				emit s_adrch(model->dmpadr);
			} else {
				setAdr(model->dmpadr - 8);
			}
			break;
		case Qt::Key_Down:
			if (idx.row() < model->rowCount() - 1) {
				QTableView::keyPressEvent(ev);
				emit s_adrch(model->dmpadr);
			} else {
				setAdr(model->dmpadr + 8);
			}
			break;
		case Qt::Key_Left:
		case Qt::Key_Right:
			QTableView::keyPressEvent(ev);
			emit s_adrch(model->dmpadr);
			break;
		case Qt::Key_PageUp:
			setAdr(model->dmpadr - (rows() * 8));
			// update();
			break;
		case Qt::Key_PageDown:
			setAdr(model->dmpadr + (rows() * 8));
			// update();
			break;
		case Qt::Key_Return:
			if (state() == QAbstractItemView::EditingState) break;
			edit(currentIndex());
			ev->ignore();
			break;
		case Qt::Key_F2:
			ev->ignore();
			break;
		default:
			QTableView::keyPressEvent(ev);
			break;
	}
}

void xDumpTable::mousePressEvent(QMouseEvent* ev) {
	QTableView::mousePressEvent(ev);
	QModelIndex idx = indexAt(ev->pos());
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= model->rowCount())) return;
	if ((col < 0) || (col >= model->columnCount())) return;
	if (col > 8) return;
	int adr;
	if (col == 0) {
		adr = model->dmpadr + (row << 3);
	} else {
		adr = model->dmpadr + (row << 3) + col - 1;
	}
	adr %= model->maxadr;
	switch(ev->button()) {
		case Qt::LeftButton:
			if (ev->modifiers() & Qt::ControlModifier) {
				blockStart = adr;
				if (blockEnd < blockStart) blockEnd = blockStart;
				emit rqRefill();
			} else if (ev->modifiers() & Qt::ShiftModifier) {
				blockEnd = adr;
				if (blockStart > blockEnd) blockStart = blockEnd;
				if (blockStart < 0) blockStart = 0;
				emit rqRefill();
			} else {
				markAdr = adr;
			}
			emit rqRefill();
			break;
		case Qt::MidButton:
			blockStart = -1;
			blockEnd = -1;
			markAdr = -1;
			emit rqRefill();
			break;
		default:
			break;
	}
}

void xDumpTable::mouseReleaseEvent(QMouseEvent* ev) {
	QTableView::mouseReleaseEvent(ev);
	if (ev->button() == Qt::LeftButton)
		markAdr = -1;
}

void xDumpTable::mouseMoveEvent(QMouseEvent* ev) {
	QTableView::mouseMoveEvent(ev);
	QModelIndex idx = indexAt(ev->pos());
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= model->rowCount())) return;
	if ((col < 0) || (col >= model->columnCount())) return;
	unsigned int adr;
	if ((col == 0) || (col > 8)) {
		adr = model->dmpadr + (row << 3);
	} else {
		adr = model->dmpadr + (row << 3) + col - 1;
	}
	adr %= model->maxadr;
	if ((ev->modifiers() == Qt::NoModifier) && (ev->buttons() & Qt::LeftButton) && (adr != blockStart) && (adr != blockEnd) && (adr != markAdr)) {
		if ((col == 0) || (col > 8))
			adr += 7;
		if (adr < blockStart) {
			blockStart = adr;
			blockEnd = markAdr;
		} else {
			blockStart = markAdr;
			blockEnd = adr;
		}
		emit rqRefill();
	}
	ev->accept();
}

void xDumpTable::wheelEvent(QWheelEvent* ev) {
	if (ev->yDelta < 0) {
		setAdr(model->dmpadr + 8);
		emit rqRefill();
	} else if (ev->yDelta > 0) {
		setAdr(model->dmpadr - 8);
		emit rqRefill();
	}
}
