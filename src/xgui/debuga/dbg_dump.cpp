#include "dbg_dump.h"
#include "../../xcore/xcore.h"

#include <QDebug>

#include <QTextCodec>
#include <QColor>

extern int blockStart;
extern int blockEnd;

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
	Computer* comp = conf.prof.cur->zx;
	MemPage* pg;
	int fadr;
	int res = 0xff;
	if (comp->cpu->type == CPU_I80286) {
		res = comp->hw->mrd(comp, adr, 0);
	} else {
		switch(mode) {
			case XVIEW_CPU:
				pg = mem_get_page(comp->mem, adr);	// = &comp->mem->map[(adr >> 8) & 0xff];
				fadr = mem_get_phys_adr(comp->mem, adr);	// = pg->num << 8) | (adr & 0xff);
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
	Computer* comp = conf.prof.cur->zx;
	MemPage* pg;
	int fadr;
	if (comp->cpu->type == CPU_I80286) {
		comp->hw->mwr(comp, adr, bt);
	} else {
		switch(mode) {
			case XVIEW_CPU:
				pg = mem_get_page(comp->mem, adr);	// = &comp->mem->map[(adr >> 8) & 0xff];
				fadr = mem_get_phys_adr(comp->mem, adr);	// = pg->num << 8) | (adr & 0xff);
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

xDumpModel::xDumpModel(QObject* par):xTableModel(par) {
	codePage = XCP_1251;
	mode = XVIEW_CPU;
	page = 0;
	dmpadr = 0;
	maxadr = MEM_64K;
	row_count = 11;
	pgbase = 0;
	pgsize = MEM_64K;
}

void xDumpModel::setMode(int md, int pgn, int pgb, int pgs) {
	mode = md;
	if (pgn >= 0) page = pgn;
	pgbase = (pgb >= 0) ? pgb : 0xc000;
	pgsize = (pgs >= 0) ? pgs : MEM_16K;
	maxadr = pgsize;
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
	int adr = (dmpadr + (row << 3)) % maxadr;
	int cadr = (adr + col - 1) % maxadr;
	unsigned short wrd;
	int flg = mrd(cadr) >> 8;
	QByteArray arr;
	QColor clr;
	switch(role) {
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
		case Qt::BackgroundColorRole:
#else
		case Qt::BackgroundRole:
#endif
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
					if (conf.prof.cur->zx->cpu->type == CPU_I80286) {
						adr %= maxadr;
						if (!conf.dbg.segment) {
							res = QString::number(adr, 16).toUpper().rightJustified(6 , '0');
						} else if (check_seg(adr, conf.prof.cur->zx->cpu->cs)) {
							adr -= conf.prof.cur->zx->cpu->cs.base;
							res = QString("CS:").append(gethexword(adr & 0xffff));
						} else if (check_seg(adr, conf.prof.cur->zx->cpu->ss)) {
							adr -= conf.prof.cur->zx->cpu->ss.base;
							res = QString("SS:").append(gethexword(adr & 0xffff));
						} else if (check_seg(adr, conf.prof.cur->zx->cpu->ds)) {
							adr -= conf.prof.cur->zx->cpu->ds.base;
							res = QString("DS:").append(gethexword(adr & 0xffff));
						} else if (check_seg(adr, conf.prof.cur->zx->cpu->es)) {
							adr -= conf.prof.cur->zx->cpu->es.base;
							res = QString("ES:").append(gethexword(adr & 0xffff));
						} else {
							res = QString::number(adr, 16).toUpper().rightJustified(6 , '0');
						}
						// res = QString("DS:%0").arg(gethexword(adr & 0xffff));
					} else {
						if ((mode == XVIEW_RAM) || (mode == XVIEW_ROM)) {
							adr %= pgsize;
							adr += pgbase;
							str = QString::number(page, conf.prof.cur->zx->hw->base).toUpper().rightJustified(2, '0').append(":");
						}
						str.append(QString::number(adr, conf.prof.cur->zx->hw->base).toUpper().rightJustified((conf.prof.cur->zx->hw->base == 16) ? 4 : 6, '0'));
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
		fadr = str_to_adr(conf.prof.cur->zx, str);
		if (fadr >= 0) {
			dmpadr = (fadr - (row << 3)) % maxadr;
			update();
			emit s_adrch(dmpadr);
			// emit rqRefill();
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
		emit s_datach();
	}
	return true;
}

// TABLE

xDumpTable::xDumpTable(QWidget* p):QTableView(p) {
	markAdr = -1;
	mode = XVIEW_CPU;
	model = new xDumpModel();
	setModel(model);

	connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(curAdrChanged()));

	connect(model, SIGNAL(s_datach()), this, SIGNAL(s_datach()));
	connect(model, SIGNAL(s_adrch(int)), this, SIGNAL(s_adrch(int)));
	connect(this, SIGNAL(s_adrch(int)), this, SLOT(curAdrChanged()));
}

void xDumpTable::setMode(int md, int pgn, int pgb, int pgs) {
	mode = md;
	pagenum = pgn;
	pagesize = pgs;
	model->setMode(md, pgn, pgb, pgs);
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

void xDumpTable::curAdrChanged() {
	int adr = getCurrentAdr();
	emit s_curadrch(adr);
}

int xDumpTable::getCurrentAdr() {
	int adr;
	int col;
	QModelIndex idx;
	idx = currentIndex();
	col = idx.column();
	adr = model->dmpadr + (idx.row() << 3);
	if ((col > 0) && (col < 9)) {
		adr += idx.column() - 1;
	}
	adr %= model->maxadr;
	return adr;
}

void xDumpTable::resizeEvent(QResizeEvent* ev) {
	int h = ev->size().height();
	if (h < 1) return;
	int rh = verticalHeader()->defaultSectionSize();
	int rc = h / rh;
	model->setRows(rc);
	setView(view);
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
			// emit s_adrch(model->dmpadr);
			break;
		case Qt::Key_PageUp:
			setAdr(model->dmpadr - (rows() * 8));
			break;
		case Qt::Key_PageDown:
			setAdr(model->dmpadr + (rows() * 8));
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
				emit s_blockch();
			} else if (ev->modifiers() & Qt::ShiftModifier) {
				blockEnd = adr;
				if (blockStart > blockEnd) blockStart = blockEnd;
				if (blockStart < 0) blockStart = 0;
				emit s_blockch();
			} else {
				markAdr = adr;
			}
			update();
			break;
		case X_MidButton:
			blockStart = -1;
			blockEnd = -1;
			markAdr = -1;
			emit s_blockch();
			update();
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
	if (mode != XVIEW_CPU) return;
	QModelIndex idx = indexAt(ev->pos());
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= model->rowCount())) return;
	if ((col < 0) || (col >= model->columnCount())) return;
	int adr;
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
		emit s_blockch();
		update();
	}
	ev->accept();
}

void xDumpTable::wheelEvent(QWheelEvent* ev) {
	if (ev->yDelta < 0) {
		setAdr(model->dmpadr + 8);
	} else if (ev->yDelta > 0) {
		setAdr(model->dmpadr - 8);
	}
}

// WIDGET

xDumpWidget::xDumpWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("MEMDUMPWIDGET");

	ramBase = 0xc000;
	romBase = 0x0000;

	ui.cbCodePage->addItem("WIN1251", XCP_1251);
	ui.cbCodePage->addItem("CP866", XCP_866);
	ui.cbCodePage->addItem("KOI8R", XCP_KOI8R);

	ui.cbDumpView->addItem("CPU", XVIEW_CPU);
	ui.cbDumpView->addItem("RAM", XVIEW_RAM);
	ui.cbDumpView->addItem("ROM", XVIEW_ROM);

	ui.leDumpPageBase->setMin(0);
	ui.leDumpPageBase->setMax(0xffff);
	ui.cbDumpPageSize->addItem("256", MEM_256);
	ui.cbDumpPageSize->addItem("512", MEM_512);
	ui.cbDumpPageSize->addItem("1KB", MEM_1K);
	ui.cbDumpPageSize->addItem("2KB", MEM_2K);
	ui.cbDumpPageSize->addItem("4KB", MEM_4K);
	ui.cbDumpPageSize->addItem("8KB", MEM_8K);
	ui.cbDumpPageSize->addItem("16KB", MEM_16K);
	ui.cbDumpPageSize->addItem("32KB", MEM_32K);
	ui.cbDumpPageSize->addItem("64KB", MEM_64K);

	cellMenu = new QMenu;
	cellMenu->addAction(ui.actFetch);
	cellMenu->addAction(ui.actRead);
	cellMenu->addAction(ui.actWrite);

	ui.dumpTable->setColumnWidth(0,70);
	ui.dumpTable->setItemDelegate(new xItemDelegate(XTYPE_BYTE)); // xid_byte);
	ui.dumpTable->setItemDelegateForColumn(0, new xItemDelegate(XTYPE_LABEL)); // ) xid_labl);
	ui.dumpTable->setItemDelegateForColumn(9, new xItemDelegate(XTYPE_NONE)); // xid_none);
	ui.cbDumpPageSize->setCurrentIndex(6);	// 16K

	connect(ui.dumpTable, &xDumpTable::customContextMenuRequested, this, &xDumpWidget::customMenu);
	connect(cellMenu, &QMenu::triggered, this, &xDumpWidget::customMenuAction);

	connect(ui.dumpTable, &xDumpTable::s_adrch, ui.dumpScroll, &QScrollBar::setValue);
	connect(ui.dumpScroll, &QScrollBar::valueChanged, ui.dumpTable, &xDumpTable::setAdr);

	connect(ui.dumpTable, &xDumpTable::s_blockch, this, &xDumpWidget::s_blockch);
	connect(ui.dumpTable, &xDumpTable::s_datach, this, &xDumpWidget::s_datach);
	connect(ui.dumpTable, &xDumpTable::s_adrch, this, &xDumpWidget::s_adrch);
	connect(ui.dumpTable, &xDumpTable::s_curadrch, this, &xDumpWidget::adr_changed);

	connect(ui.cbCodePage, SIGNAL(currentIndexChanged(int)), this, SLOT(cp_changed()));
	connect(ui.cbDumpView, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged()));
	connect(ui.sbDumpPage, SIGNAL(valueChanged(int)), this, SLOT(refill()));
	connect(ui.leDumpPageBase, &xHexSpin::valueChanged, this, &xDumpWidget::refill);
	connect(ui.cbDumpPageSize, SIGNAL(currentIndexChanged(int)), this, SLOT(refill()));

	ui.cbDumpView->setCurrentIndex(0);

	cellMenu = new QMenu;
	cellMenu->addAction(ui.actFetch);
	cellMenu->addAction(ui.actRead);
	cellMenu->addAction(ui.actWrite);

	connect(this, &QDockWidget::visibilityChanged, this, &xDumpWidget::draw);
}

void xDumpWidget::setAdr(int a) {
	ui.dumpTable->setAdr(a);
}

void xDumpWidget::setLimit(int lim) {
	ui.dumpTable->setLimit(lim);
	ui.dumpScroll->setMaximum(lim - 1);
}

void xDumpWidget::customMenu() {
	int adr = ui.dumpTable->getCurrentAdr();
	unsigned char flag = getBrk(conf.prof.cur->zx, adr);
	ui.actFetch->setChecked(flag & MEM_BRK_FETCH);
	ui.actRead->setChecked(flag & MEM_BRK_RD);
	ui.actWrite->setChecked(flag & MEM_BRK_WR);
	cellMenu->move(cursor().pos());
	cellMenu->show();
}

void xDumpWidget::customMenuAction(QAction* act) {
	int bt = 0;
	int adr = ui.dumpTable->getCurrentAdr();
	if (ui.actFetch->isChecked()) bt |= MEM_BRK_FETCH;
	if (ui.actRead->isChecked()) bt |= MEM_BRK_RD;
	if (ui.actWrite->isChecked()) bt |= MEM_BRK_WR;
	switch (getRFIData(ui.cbDumpView)) {
		case XVIEW_CPU: emit s_brkrq(BRK_CPUADR, bt, adr); break;
		case XVIEW_ROM: emit s_brkrq(BRK_MEMROM, bt, adr); break;
		case XVIEW_RAM: emit s_brkrq(BRK_MEMRAM, bt, adr); break;
	}
}

void xDumpWidget::setBase(int b, int t) {
	xItemDelegate* xid_octw = new xItemDelegate(XTYPE_OCTWRD);
	xItemDelegate* xid_byte = new xItemDelegate(XTYPE_BYTE);
	switch(b) {
		case 8:
			ui.dumpTable->setItemDelegateForColumn(1, xid_octw);
			ui.dumpTable->setItemDelegateForColumn(3, xid_octw);
			ui.dumpTable->setItemDelegateForColumn(5, xid_octw);
			ui.dumpTable->setItemDelegateForColumn(7, xid_octw);
			ui.dumpTable->setView(XVIEW_OCTWRD);
			break;
		default:
			ui.dumpTable->setItemDelegateForColumn(1, xid_byte);
			ui.dumpTable->setItemDelegateForColumn(3, xid_byte);
			ui.dumpTable->setItemDelegateForColumn(5, xid_byte);
			ui.dumpTable->setItemDelegateForColumn(7, xid_byte);
			ui.dumpTable->setView(XVIEW_DEF);
			break;
	}
	if (t == HW_IBM_PC) {
		ui.cbDumpView->setCurrentIndex(0);		// cpu only
		ui.cbDumpView->setEnabled(false);
	} else {
		ui.cbDumpView->setEnabled(true);
	}

}

void xDumpWidget::draw() {
	ui.dumpTable->update();
	refill();
}

void xDumpWidget::adr_changed(int adr) {
	titleWidget->setText(QString("DUMP | %0").arg(QString::number(adr, 16).right(6).toUpper().rightJustified(6,'0')));
//	setWindowTitle(QString::number(adr, 16).right(6).toUpper().rightJustified(6,'0'));
}

void xDumpWidget::cp_changed() {
	int cp = getRFIData(ui.cbCodePage);
	ui.dumpTable->setCodePage(cp);
	ui.dumpTable->update();
}

void xDumpWidget::modeChanged() {
	int mode = getRFIData(ui.cbDumpView);
	switch(mode) {
		case XVIEW_CPU: refill(); break;
		case XVIEW_ROM:
		case XVIEW_RAM:
			ui.leDumpPageBase->setValue((mode == XVIEW_ROM) ? romBase : ramBase);
			refill();
			break;
	}
}

void xDumpWidget::refill() {
	int mode = getRFIData(ui.cbDumpView);
	int page = ui.sbDumpPage->value();
	int pbase = ui.leDumpPageBase->getValue();
	int psize;
	Computer* comp = conf.prof.cur->zx;
	if (mode == XVIEW_CPU) {
		psize = (comp->hw->id == HW_IBM_PC) ? MEM_4M : MEM_64K;
	} else {
		psize = getRFIData(ui.cbDumpPageSize);
	}
	ui.widDumpPage->setDisabled(mode == XVIEW_CPU);
	ui.dumpTable->setMode(mode, page, pbase, psize);
	ui.dumpScroll->setMaximum(psize-1);
	ui.dumpTable->setLimit(psize);
}
