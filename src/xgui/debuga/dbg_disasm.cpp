#include "dbg_disasm.h"
#include "dbg_dump.h"
#include "../../xcore/xcore.h"

#include <QDebug>
#include <QFont>
#include <QPainter>
#include <QHeaderView>

extern int blockStart;
extern int blockEnd;

static int mode = XVIEW_CPU;
static int page = 0;

extern unsigned short adr_of_reg(CPU* cpu, bool* flag, QString nam);

// MODEL

xDisasmModel::xDisasmModel(QObject* p):QAbstractTableModel(p) {
//	cptr = NULL;
	disasmAdr = 0;
	row_count = 25;
}

int xDisasmModel::columnCount(const QModelIndex&) const {
	return 4;
}

int xDisasmModel::rowCount(const QModelIndex&) const {
	return row_count;
}

void xDisasmModel::setRows(int r) {
	if (r < row_count) {
		emit beginRemoveRows(QModelIndex(), r, row_count);
		row_count = r;
		emit endRemoveRows();
	} else if (r > row_count) {
		emit beginInsertRows(QModelIndex(), row_count, r);
		row_count = r;
		emit endInsertRows();
		fill();
	}
}

QVariant xDisasmModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
//	if (!cptr) return res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if (row < 0) return res;
	if (col < 0) return res;
	if (row >= rowCount()) return res;
	if (col >= columnCount()) return res;
	if (row >= dasm.size()) return res;
	QFont font;
	QColor clr;
	QPixmap pxm;
	QPixmap icn;
	QPainter pnt;
	switch(role) {
		case Qt::EditRole:
			switch(col) {
				case 0: res = dasm[row].aname; break;
				case 1: res = dasm[row].bytes; break;
				case 2: res = dasm[row].command; break;
			}
			break;
		case Qt::FontRole:
			if ((col == 0) && dasm[row].islab && !dasm[row].iscom) {
				font = QFont();
				font.setBold(true);
				res = font;
			}
			break;
		case Qt::ForegroundRole:
			if (dasm[row].isbrk) {
				clr = conf.pal["dbg.brk.txt"];
			} else if ((col == 0) && !dasm[row].islab && conf.dbg.hideadr) {
				clr = QColor(Qt::gray);
			} else if ((col == 0) && dasm[row].islab && dasm[row].iscom) {
				clr = QColor(Qt::gray);
			} else if (dasm[row].ispc && !dasm[row].islab) {
				clr = conf.pal["dbg.pc.txt"];
			} else if (dasm[row].issel) {
				clr = conf.pal["dbg.sel.txt"];
			} else {
				clr = conf.pal["dbg.table.txt"];
			}
			if (clr.isValid())
				res = clr;
			break;
		case Qt::BackgroundColorRole:
			if (dasm[row].ispc && !dasm[row].islab) {
				clr = conf.pal["dbg.pc.bg"];
			} else if (dasm[row].issel) {
				clr = conf.pal["dbg.sel.bg"];
			} else {
				clr = conf.pal["dbg.table.bg"];
			}
			if (clr.isValid())
				res = clr;
			break;
		case Qt::DecorationRole:
			if ((col == 3) && !dasm[row].icon.isEmpty()) {
				pxm = QPixmap(45,15);
				pxm.fill(Qt::transparent);
				icn.load(dasm[row].icon);
				pnt.begin(&pxm);
				pnt.drawPixmap(pxm.width() - icn.width(),0,icn.scaled(16,16));
				pnt.end();
				res = pxm;
			}
			break;
		case Qt::TextAlignmentRole:
			if (col == 3) res = Qt::AlignLeft;
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

int dasmrd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	unsigned char res = 0xff;
	int fadr;
	MemPage* pg;
	if (comp->cpu->type == CPU_I80286) {
		res = comp->hw->mrd(comp, comp->cpu->cs.base + (adr & 0xffff), 0) & 0xff;
	} else {
		switch (mode) {
			case XVIEW_CPU:
				pg = mem_get_page(comp->mem, adr);	// = &comp->mem->map[adr >> 8];
				fadr = mem_get_phys_adr(comp->mem, adr);
				switch (pg->type) {
					case MEM_ROM: res = comp->mem->romData[fadr & comp->mem->romMask]; break;
					case MEM_RAM: res = comp->mem->ramData[fadr & comp->mem->ramMask]; break;
					case MEM_SLOT: res = memRd(comp->mem, adr & 0xffff);
						break;
				}
				break;
			case XVIEW_RAM:
				res = comp->mem->ramData[((adr & 0x3fff) | (page << 14)) & comp->mem->ramMask];
				break;
			case XVIEW_ROM:
				res = comp->mem->romData[((adr & 0x3fff) | (page << 14)) & comp->mem->romMask];
				break;
		}
	}
	return res;
}

void dasmwr(Computer* comp, int adr, int bt) {
	if (comp->cpu->type == CPU_I80286) return;
	int fadr;
	MemPage* pg;
	if (comp->cpu->type == CPU_I80286) {
		comp->hw->mwr(comp, comp->cpu->cs.base + (adr & 0xffff), bt);
	} else {
		switch(mode) {
			case XVIEW_CPU:
				pg = mem_get_page(comp->mem, adr);		// = &comp->mem->map[(adr >> 8) & 0xff];
				fadr = mem_get_phys_adr(comp->mem, adr);	// pg->num << 8) | (adr & 0xff);
				switch (pg->type) {
					// no writings to slot
					case MEM_ROM:
						if (conf.dbg.romwr)
							comp->mem->romData[fadr & comp->mem->romMask] = bt & 0xff;
						break;
					case MEM_RAM:
						comp->mem->ramData[fadr & comp->mem->ramMask] = bt & 0xff;
						break;
				}
				break;
			case XVIEW_RAM:
				comp->mem->ramData[((adr & 0x3fff) | (page << 14)) & comp->mem->ramMask] = bt & 0xff;
				break;
			case XVIEW_ROM:
				if (conf.dbg.romwr)
					comp->mem->romData[((adr & 0x3fff) | (page << 14)) & comp->mem->romMask] = bt & 0xff;
				break;
		}
	}
}

void placeLabel(Computer* comp, dasmData& drow) {
	int shift = 0;
	int work = 1;
	QString lab;
	QString num;
	xMnem mn;
	xAdr xadr;
	while (work && (shift < 8)) {
		if (comp->cpu->type == CPU_I80286) {
			xadr = mem_get_xadr(comp->mem, drow.oadr - shift + comp->cpu->cs.base);
		} else {
			xadr = mem_get_xadr(comp->mem, drow.oadr - shift);
		}
		lab = find_label(xadr);
		if (lab.isEmpty()) {
			if (drow.oflag & OF_MEMADR) {
				shift++;
			} else {
				work = 0;
			}
		} else {
			mn.len = 8;					// default seek range
			if ((drow.flag & 0xf0) == DBG_VIEW_CODE) {	// for code - size of opcode only
				mn = cpuDisasm(comp->cpu, (drow.oadr - shift) & 0xffff, NULL, dasmrd, comp);
			}
			if (shift < mn.len) {
				num = gethexword(drow.oadr).prepend("#").toUpper();
				if (shift == 0) {
					drow.command.replace(num, QString("%0").arg(lab));
				} else {
					drow.command.replace(num, QString("%0 + %1").arg(lab).arg(shift));
				}
			}
			work = 0;
		}
	}
}

int dasmByte(Computer* comp, int adr, dasmData& drow) {
	int len = 1;
	drow.command = QString("DB #%0").arg(gethexbyte(dasmrd(adr & 0xffff, comp)));
	adr++;
	unsigned char fl = getBrk(comp, adr);
	xAdr xadr = mem_get_xadr(comp->mem, adr);
	while ((len < conf.dbg.dbsize) && (fl == DBG_VIEW_BYTE) /*&& findLabel(adr,-1,-1).isEmpty()*/ && find_label(xadr).isEmpty()) {
		drow.command.append(QString(",#%0").arg(gethexbyte(dasmrd(adr & 0xffff, comp))));
		len++;
		adr++;
		fl = getBrk(comp, adr);
	}
	return len;
}

int dasmWord(Computer* comp, int adr, dasmData& drow) {
	int len = 2;
	int word = dasmrd(adr, comp);
	word |= (dasmrd(adr + 1, comp) << 8);
	drow.command = QString("DW #%0").arg(gethexword(word));
	adr += 2;
	unsigned char fl = getBrk(comp, adr);
	while ((len < conf.dbg.dwsize * 2) && (fl == DBG_VIEW_WORD)) {
		word = dasmrd(adr, comp);
		word |= (dasmrd(adr + 1, comp) << 8);
		drow.command.append(QString(",#%0").arg(gethexword(word)));
		adr += 2;
		len += 2;
		fl = getBrk(comp, adr);
	}
	return len;
}

int dasmAddr(Computer* comp, int adr, dasmData& drow) {
	int len = 2;
	int word = dasmrd(adr, comp);
	word |= (dasmrd(adr + 1, comp) << 8);
/*
	xAdr xadr;
	if (comp->cpu->type == CPU_I80286) {
		xadr = mem_get_xadr(comp->mem, word + comp->cpu->cs.base);
	} else {
		xadr = mem_get_xadr(comp->mem, word);
	}
	QString lab = find_label(xadr);
	if (lab.isEmpty()) {
		lab = gethexword(word).prepend("#");
	}
*/
	drow.oadr = word;
	drow.command = QString("DW #%0").arg(gethexword(word));
	placeLabel(comp, drow);
	return len;
}

int dasmText(Computer* comp, int adr, dasmData& drow) {
	int clen = 0;
	drow.command = QString("DB \"");
	unsigned char fl = getBrk(comp, adr);
	unsigned char bt = dasmrd(adr, comp);
	while (((fl & 0xf0) == DBG_VIEW_TEXT) && (bt > 31) && (bt < 128) && (clen < conf.dbg.dmsize)) {
		drow.command.append(QChar(bt));
		clen++;
		bt = dasmrd(adr + clen, comp);
		fl = getBrk(comp, adr + clen);
	}
	if (clen == 0) {		// TODO: non-ascii characters: single db, multiple db untill ascii or wut?
		drow.flag = (getBrk(comp, adr) & 0x0f) | DBG_VIEW_BYTE;
		setBrk(comp, adr, drow.flag);
		clen = 1;
		drow.command = QString("DB #%0").arg(dasmrd(adr, comp));
	} else {
		drow.command.append("\"");
	}
	return clen;
}

int dasmCode(Computer* comp, int adr, dasmData& drow) {
	char buf[1024];
	xMnem mnm = cpuDisasm(comp->cpu, adr, buf, dasmrd, comp);
	drow.command = QString(buf).toUpper();
	drow.oadr = mnm.oadr;
	drow.oflag = mnm.flag;
	placeLabel(comp, drow);
	if (drow.ispc) {
		if (mnm.mem) {
			if (mnm.flag & OF_MWORD) {
				drow.info = QString::number(mnm.mop, 16).toUpper().rightJustified(4, '0');
			} else {
				drow.info = QString::number(mnm.mop & 0xff, 16).toUpper().rightJustified(2, '0');
			}
		} else if (mnm.cond && mnm.met && (drow.oadr >= 0)) {
			if (drow.adr < drow.oadr) {
				drow.icon = QString(":/images/arrdn.png");
			} else if (drow.adr > drow.oadr) {
				drow.icon = QString(":/images/arrup.png");
			} else {
				//drow.icon = QString(":/images/redCircle.png");
				drow.icon = QString(":/images/arrleft.png");
			}
		}
	}
	return mnm.len;
}

int dasmSome(Computer* comp, int adr, dasmData& drow) {
	int clen = 0;
	drow.adr = adr;
	drow.flag = getBrk(comp, adr);
	drow.oflag = 0;
	drow.oadr = -1;
	switch(drow.flag & 0xf0) {
		case DBG_VIEW_WORD: clen = dasmWord(comp, adr, drow); break;
		case DBG_VIEW_ADDR: clen = dasmAddr(comp, adr, drow); break;
		case DBG_VIEW_TEXT: clen = dasmText(comp, adr, drow); break;
		case DBG_VIEW_CODE:
		case DBG_VIEW_EXEC: clen = dasmCode(comp, adr, drow); break;
		case DBG_VIEW_BYTE: clen = dasmByte(comp, adr, drow); break;
	}
	return clen;
}

// adr must be cpu adr bus (16/24 bits)
QList<dasmData> getDisasm(Computer* comp, int& adr) {
	QList<dasmData> list;
	dasmData drow;
	xAdr xadr;
	int oadr = adr;
	drow.adr = adr;
	drow.oflag = 0;
	drow.ispc = 0;
	drow.issel = 0;
	drow.islab = 0;
	drow.iscom = 0;
	drow.isequ = 0;
	drow.info.clear();
	drow.icon.clear();
	int clen = 0;
//	int wid;
	// 0:adr
	QString lab;
	xadr = mem_get_xadr(comp->mem, comp->cpu->pc);
	int abs = xadr.abs;		// remember pc cell
	int pct = xadr.type;
	switch (mode) {
		case XVIEW_RAM:
			xadr.type = MEM_RAM;
			xadr.bank = page << 6;
			xadr.adr = adr & 0x3fff;
			xadr.abs = xadr.adr | (page << 14);
			drow.flag = comp->brkRamMap[xadr.abs];
			drow.ispc = ((xadr.type == pct) && (abs == xadr.abs)) ? 1 : 0;
			break;
		case XVIEW_ROM:
			xadr.type = MEM_ROM;
			xadr.bank = page << 6;
			xadr.adr = adr & 0x3fff;
			xadr.abs = xadr.adr | (page << 14);
			drow.flag = comp->brkRomMap[xadr.abs];
			drow.ispc = ((xadr.type == pct) && (abs == xadr.abs)) ? 1 : 0;
			break;
		default:
			xadr = mem_get_xadr(comp->mem, adr);
			drow.flag = getBrk(comp, xadr.adr);
			drow.ispc = (xadr.adr == comp->cpu->pc + comp->cpu->cs.base) ? 1 : 0;
			drow.issel = ((adr >= blockStart) && (adr <= blockEnd)) ? 1 : 0;
			break;
	}
	drow.isbrk = (drow.flag & MEM_BRK_ANY) ? 1 : 0;

	// comments
	lab = find_comment(xadr);
	if (!lab.isEmpty()) {
		drow.islab = 1;
		drow.iscom = 1;
		drow.aname = lab;
		drow.aname.prepend("; ");
		list.append(drow);
		drow.iscom = 0;
		lab.clear();
	}
	// add label line if any
	if (conf.dbg.labels) {		// if show labels
		lab = find_label(xadr);
	}
	if (!lab.isEmpty()) {		// place label line if it exists
		drow.islab = 1;
		drow.aname = lab;
		list.append(drow);
	}
	drow.islab = 0;			// next line is addr
	if (comp->cpu->type == CPU_I80286) {
		drow.aname = QString("CS:%0").arg(gethexword(adr));
	} else if (conf.dbg.segment || (mode != XVIEW_CPU)) {
		switch(xadr.type) {
			case MEM_RAM: drow.aname = "RAM:"; break;
			case MEM_ROM: drow.aname = "ROM:"; break;
			case MEM_SLOT: drow.aname = "SLT:"; break;
			case MEM_EXT: drow.aname = "EXT:"; break;
			default: drow.aname = "???"; break;
		}
		drow.aname.append(QString("%1:%2").arg(gethexbyte((xadr.bank >> 6) & 0xff)).arg(gethexword(xadr.adr & 0x3fff)));
	} else {
		drow.aname = QString::number(xadr.adr, comp->hw->base).toUpper().rightJustified((comp->hw->base == 8) ? 6 : 4, '0');
	}
	// 2:command / 3:info
	clen = dasmSome(comp, adr, drow);
	// 1:bytes
	drow.bytes.clear();
	while(clen > 0) {
		drow.bytes.append(gethexbyte(dasmrd(adr, comp)));
		adr++;
		clen--;
	}
	list.append(drow);
// add equ's
	clen = adr - oadr;
	while (clen > 1) {
		oadr++;
		clen--;
		xadr = mem_get_xadr(comp->mem, oadr);
		lab = find_label(xadr);
		//lab = findLabel(oadr, -1, -1);
		if (!lab.isEmpty()) {
			drow.islab = 1;
			drow.isequ = 1;
			drow.adr = oadr;
			drow.aname = lab;
			drow.command = QString("EQU $-%0").arg(clen);
			list.append(drow);
		}
	}

	return list;
}

int xDisasmModel::fill() {
	dasmData drow;
	QList<dasmData> list;
	int row;
	int adr = (disasmAdr & 0xffff) + conf.prof.cur->zx->cpu->cs.base;
	int res = 0;
//	QString lab;
	dasm.clear();
	for(row = 0; row < rowCount(); row++) {
		list = getDisasm(conf.prof.cur->zx, adr);
		foreach (drow, list) {
			if (dasm.size() < rowCount()) {
				dasm.append(drow);
				res |= drow.ispc;
			} else {
				row = rowCount();		// it will prevent next iteration
			}
		}
	}
	return res;
}

void xDisasmModel::update_data() {
	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

int xDisasmModel::update() {
//	if (cptr == NULL) return 0;
//	if (*cptr == NULL) return 0;
	int res = fill();
	int i;
	xMnem mnm = cpuDisasm(conf.prof.cur->zx->cpu, conf.prof.cur->zx->cpu->pc, NULL, dasmrd, conf.prof.cur->zx);
	if (mnm.cond && mnm.met) {
		for (i = 0; i < dasm.size(); i++) {
			if ((dasm[i].adr == mnm.oadr) && (mnm.oadr != conf.prof.cur->zx->cpu->pc)) {
				dasm[i].icon = QString(":/images/arrleft.png");
			}
		}
	}
	update_data();
	return res;
}

Qt::ItemFlags xDisasmModel::flags(const QModelIndex& idx) const {
	Qt::ItemFlags res = QAbstractItemModel::flags(idx);
	if ((idx.column() < 3) && !((idx.column() == 2) && dasm[idx.row()].isequ)) {
		res |= Qt::ItemIsEditable;
	}
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

unsigned short adr_of_reg(CPU* cpu, bool* flag, QString nam) {
	xRegBunch bch = cpuGetRegs(cpu);
	int i = 0;
	nam = nam.toUpper();
	while ((bch.regs[i].id != REG_NONE) && (QString(bch.regs[i].name).toUpper() != nam))
		i++;
	if (bch.regs[i].id == REG_NONE) {
		*flag = false;
		return 0;
	} else {
		*flag = true;
		return bch.regs[i].value;
	}
}

int str_to_adr(Computer* comp, QString str) {
	bool flag = true;
	int adr = -1;
	if (str.startsWith(".")) {			// .REG : name of CPU register
		adr = adr_of_reg(comp->cpu, &flag, str.mid(1));
	} else if (str.startsWith("#")) {			// #nnnn : hex adr
		str.replace(0, 1, "0x");
		adr = str.toInt(&flag, 16);
	} else if (str.startsWith("0x")) {			// 0xnnnn : hex adr
		adr = str.toInt(&flag, 16);
	} else if (conf.prof.cur->labels.contains(str)) {	// NAME : label name
		adr = conf.prof.cur->labels[str].adr;
		flag = true;
	} else {						// other : adr in base of comp hardware (16 | 8)
		adr = str.toInt(&flag, comp->hw->base);
	}
	if (!flag) adr = -1;
	return adr;
}

// convert string val to address (.reg, #HEXA 0xHEXA HEXA(base of computer) label)
int asmAddr(Computer* comp, QVariant val, xAdr xadr) {
	QString lab;
	QString str = val.toString();
	int adr;
	if (str.startsWith('@')) {			// @label will not create new label, just jump if it exists
		adr = str_to_adr(comp, str.mid(1));
	} else {
		adr = str_to_adr(comp, str);
	}
	if (adr < 0) {
		if ((str.at(0).isLetter() || (str.at(0) == '_')) && !str.contains('$') && !str.contains(' ')) {
			lab = find_label(xadr);
			// lab = findLabel(xadr.adr, xadr.type, xadr.bank);		// current address label
			if (!lab.isEmpty()) {						// remove current label
				del_label(lab);
			}
			add_label(xadr, str);						// add new label
		}
	}
	return adr;
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
	int idx = -1;
	int zadr;
	int oadr = disasmAdr;
	int len;
	QString str;
	int adr = dasm[row].adr;
	Computer* comp = conf.prof.cur->zx;
	xAdr xadr = mem_get_xadr(comp->mem, adr);
	switch(col) {
		case 0:
			str = val.toString();
			if (str.isEmpty()) {				// empty string = delete label/comment
				if (dasm[row].islab) {			// comment or label
					if (dasm[row].iscom) {		// comment
						del_comment(xadr);
					} else {			// label
						str = find_label(xadr);
						if (!str.isEmpty()) {
							del_label(str);
						}
					}
				}
				//emit s_adrch(oadr, disasmAdr);
			} else if (str.startsWith(";")) {		// ; comment
				str.remove(";");
				str = str.trimmed();
				if (str.isEmpty()) {
					del_comment(xadr);
				} else {
					add_comment(xadr, str);
				}
				//emit s_adrch(oadr, disasmAdr);
			} else if (str.startsWith(".")) {		// .REG
				idx = adr_of_reg(comp->cpu, &flag, str.mid(1));
				if (!flag)
					idx = -1;
			} else {					// hex/label
				idx = asmAddr(comp, val, xadr);
				//emit s_adrch(oadr, disasmAdr);
			}
			if (idx >= 0) {
				while (row > 0) {
					idx = getPrevAdr(comp, idx & 0xffff);
					zadr = idx & 0xffff;
					row -= getDisasm(comp, zadr).size();
				}
				disasmAdr = idx & 0xffff;
				//emit s_adrch(oadr, disasmAdr);
			}
			emit s_adrch(oadr, disasmAdr);
			break;
		case 1:	// bytes
			str = val.toString();
			while(!str.isEmpty()) {
				cbyte = str.left(2).toInt(&flag,16) & 0xff;
				if (flag)
					dasmwr(comp, adr, cbyte);
				adr++;
				str.remove(0, 2);
			}
			emit rqRefill();
			break;
		case 2:	// command
			ptr = getBrkPtr(comp, adr);
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
				if (conf.prof.cur->labels.contains(str)) {				// check label
					idx = conf.prof.cur->labels[str].adr;
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
			} else {			// code
				// TODO: replace label name
				len = cpuAsm(comp->cpu, str.toLocal8Bit().data(), buf, adr);
				if (len > 0) {
					for(idx = 0; idx < len; idx++) {
						*ptr &= 0x0f;
						*ptr |= DBG_VIEW_EXEC;
					}
				}
			}
			idx = 0;
			while (idx < len) {
				dasmwr(comp, (adr + idx) & 0xffff, buf[idx]);
				idx++;
			}
			emit rqRefill();
			break;
	}
	update();
	return true;
}

// TABLE

xDisasmTable::xDisasmTable(QWidget* p):QTableView(p) {
	//cptr = NULL;
	model = new xDisasmModel();
	setModel(model);
	connect(model, SIGNAL(s_adrch(int, int)), this, SLOT(t_update(int, int)));
	connect(model, SIGNAL(rqRefill()), this, SIGNAL(rqRefill()));
}

void xDisasmTable::resizeEvent(QResizeEvent* ev) {
	int wid = ev->size().width();
	QFontMetrics qfm(font());
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
	int adw = qfm.horizontalAdvance("000:00:0000") + 10;
	int btw = qfm.horizontalAdvance("0000000000") + 10;
#else
	int adw = qfm.width("000:00:0000") + 10;
	int btw = qfm.width("0000000000") + 10;
#endif
	setColumnWidth(0, adw);
	setColumnWidth(1, btw);
	setColumnWidth(2, wid - adw - btw - 50);
	setColumnWidth(3, 50);

	int h = ev->size().height();
	int rh = verticalHeader()->defaultSectionSize();
	int rc = h / rh;
	// if (h % rh != 0) rc++;
	model->setRows(rc);
	updContent();

}

QVariant xDisasmTable::getData(int row, int col, int role) {
	return model->data(model->index(row, col), role);
}

int xDisasmTable::rows() {
	return model->rowCount();
}

/*
void xDisasmTable::setComp(Computer** comp) {
	cptr = comp;
	model->cptr = comp;
}
*/

void xDisasmTable::setMode(int md, int pg) {
	mode = md;
	page = pg;
	updContent();
}

unsigned short xDisasmTable::getAdr() {
	return model->disasmAdr;
}

void xDisasmTable::setAdr(int adr, int hist) {
	if (hist)
		history.append(model->disasmAdr);
	int oadr = model->disasmAdr & 0xffff;
	model->disasmAdr = adr & 0xffff;
	updContent();
	if (oadr != model->disasmAdr)
		emit s_adrch(model->disasmAdr);
}

int xDisasmTable::updContent() {
	int res = model->update();
	clearSpans();
	for (int i = 0; i < model->dasm.size(); i++) {
		if (model->dasm[i].isequ) {
			setSpan(i, 0, 1, 2);
			setSpan(i, 2, 1, 2);
		} else if (model->dasm[i].islab) {
			setSpan(i, 0, 1, model->columnCount());
		} else if (model->dasm[i].icon.isEmpty() && model->dasm[i].info.isEmpty()) {
			setSpan(i, 2, 1, model->columnCount() - 2);
		}
	}
	// prevent case if current cell is invisible (eated by span)
	QModelIndex idx = currentIndex();
	int r = idx.row();
	if (idx.isValid()) {
		if (model->dasm[r].islab) {
			// if ((r == 0) || (r == rows() - 1)) {
				setCurrentIndex(model->index(r, 0));
			//}
		}
	}
	return res;
}

void xDisasmTable::update() {
	model->update_data();
}

void xDisasmTable::t_update(int oadr, int nadr) {
	if (oadr >= 0) {
		history.append(oadr);
	}
	updContent();
/*
	for(int r = 0; r < rows(); r++) {
		if (model->dasm[r].adr == (nadr & 0xffff)) {
			setCurrentIndex(model->index(r, 0));
			r = rows();
		}
	}
*/
	if (oadr != nadr)
		emit s_adrch(nadr);
}

void xDisasmTable::keyPressEvent(QKeyEvent* ev) {
	QModelIndex idx = currentIndex();
	int i;
	int bpt;
	int bpr;
	int adr;
	xAdr xadr;
	int key = shortcut_check(SCG_DISASM, QKeySequence(ev->key() | ev->modifiers()));
	if (key < 0)
		key = shortcut_check(SCG_DISASM, QKeySequence(ev->key()));
	if (key < 0)
		key = ev->key();
	switch (key) {
		case Qt::Key_Up:
			if ((ev->modifiers() & Qt::ControlModifier) || (idx.row() == 0)) {
				scrolUp(ev->modifiers());
			} else {
				QTableView::keyPressEvent(ev);
			}
			break;
		case Qt::Key_Down:
			if ((ev->modifiers() & Qt::ControlModifier) || (idx.row() == model->rowCount() - 1)) {
				scrolDn(ev->modifiers());
			} else {
				QTableView::keyPressEvent(ev);
			}
			break;
		case Qt::Key_PageUp:
			for (i = 0; i < rows() - 1; i++) {
				model->disasmAdr = getPrevAdr(conf.prof.cur->zx, model->disasmAdr);
			}
			updContent();
			emit s_adrch(model->disasmAdr);
			break;
		case Qt::Key_PageDown:
			model->disasmAdr = getData(rows() - 1, 0, Qt::UserRole).toInt() & 0xffff;
			updContent();
			emit s_adrch(model->disasmAdr);
			break;
		case XCUT_TOPC:
			if (mode != XVIEW_CPU) break;
			if (!conf.prof.cur->zx) break;
			model->disasmAdr = conf.prof.cur->zx->cpu->pc;
			updContent();
			emit s_adrch(model->disasmAdr);
			break;
		case XCUT_SETPC:
			if (mode != XVIEW_CPU) break;
			if (!conf.prof.cur->zx) break;
			conf.prof.cur->zx->cpu->pc = getData(idx.row(), 0, Qt::UserRole).toInt() & 0xffff;
			emit rqRefillAll();
			break;
		case XCUT_SAVE:
			ev->ignore();
			break;
		case XCUT_SETBRK:
			adr = getData(idx.row(), 0, Qt::UserRole).toInt();	// bus addr
			if (ev->modifiers() & Qt::ShiftModifier) {
				bpr = BRK_CPUADR;
				bpt = 0;
			} else {
				bpr = BRK_MEMCELL;
				xadr = mem_get_xadr(conf.prof.cur->zx->mem, adr);
				switch(xadr.type) {
					case MEM_RAM: bpt = MEM_BRK_RAM; break;
					case MEM_ROM: bpt = MEM_BRK_ROM; break;
					default: bpt = MEM_BRK_SLT; break;
				}
				adr = xadr.abs;
			}
			// modifiers doesn't work with new hotkeys (hotkey not detected)
			if (ev->modifiers() & Qt::AltModifier) {
				bpt |= MEM_BRK_RD;
			} else if (ev->modifiers() & Qt::ControlModifier) {
				bpt |= MEM_BRK_WR;
			} else {
				bpt |= MEM_BRK_FETCH;
			}
			brkXor(bpr, bpt, adr, -1, 1);
			emit rqRefill();
			break;
		case XCUT_JUMPTO:
			if (!idx.isValid()) break;
			adr = model->dasm[idx.row()].oadr;
			if (adr < 0) break;
			history.append(model->disasmAdr);
			model->setData(model->index(idx.row(), 0), gethexword(adr & 0xffff).prepend("0x"), Qt::EditRole);
			updContent();
			setCurrentIndex(idx);
			emit s_adrch(model->disasmAdr);
			break;
		case XCUT_RETFROM:
			if (history.size() < 1) break;
			model->disasmAdr = history.takeLast();
			updContent();
			emit s_adrch(model->disasmAdr);
			break;
		case Qt::Key_Return:
			if (state() == QAbstractItemView::EditingState) break;
			edit(currentIndex());
			break;
		default:
			QTableView::keyPressEvent(ev);
			break;
	}
}

void xDisasmTable::mousePressEvent(QMouseEvent* ev) {
	int row = rowAt(ev->pos().y());
	if ((row < 0) || (row >= model->rowCount())) return;
	int adr = getData(row, 0, Qt::UserRole).toInt();
	switch (ev->button()) {
		case Qt::MiddleButton:
			blockStart = -1;
			blockEnd = -1;
			emit rqRefill();
			ev->ignore();
			break;
		case Qt::LeftButton:
			if (mode != XVIEW_CPU) {
				QTableView::mousePressEvent(ev);
			} else if (ev->modifiers() & Qt::ControlModifier) {
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
	if (mode != XVIEW_CPU) return;
	int row = rowAt(ev->pos().y());
	if ((row < 0) || (row >= model->rowCount())) return;
	int adr = model->data(model->index(row, 0), Qt::UserRole).toInt();	// item(row,0)->data(Qt::UserRole).toInt();
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
	int i = 1;
	if (mod & Qt::ControlModifier) {
		model->disasmAdr++;
	} else {
		// TODO: comparing disasmAdr (16 bit) with dasm[i].adr (24 bit)
		while (((model->disasmAdr + conf.prof.cur->zx->cpu->cs.base) == model->dasm[i].adr) && (i < model->dasm.size()))
			i++;
		if (i < model->dasm.size()) {
			model->disasmAdr = model->dasm[i].adr - conf.prof.cur->zx->cpu->cs.base;
		} else {
			model->disasmAdr++;
		}
	}
	updContent();
	emit s_adrch(model->disasmAdr);
}

void xDisasmTable::scrolUp(Qt::KeyboardModifiers mod) {
	if (mod & Qt::ControlModifier) {
		model->disasmAdr--;
	} else {
		model->disasmAdr = getPrevAdr(conf.prof.cur->zx, model->disasmAdr);
	}
	updContent();
	emit s_adrch(model->disasmAdr);
}

void xDisasmTable::wheelEvent(QWheelEvent* ev) {
	if (ev->yDelta < 0) {
		scrolDn(ev->modifiers());
	} else if (ev->yDelta > 0) {
		scrolUp(ev->modifiers());
	}
}
