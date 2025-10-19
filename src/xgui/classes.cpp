#include "xgui.h"
#include "../xcore/xcore.h"

#include <QPalette>
#include <QPainter>
#include <QDebug>

QString gethexword(int);
QString gethexbyte(uchar);

void setRegExp(QRegExpValidator& v, QString s) {
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
	v.setRegExp(QRegExp(s));
#else
	v.setRegularExpression(QRegularExpression(s));
#endif
}

xHexSpin::xHexSpin(QWidget* p):QLineEdit(p) {
	setMinimumWidth(60);
	setAutoFillBackground(true);
	setUpdatesEnabled(true);
	//vldtr.setRegExp(QRegExp(""));
	setRegExp(vldtr, "");
	min = 0x0000;
	max = 0xffff;
	value = 0x0000;
	hsflag = XHS_DEC;
	len = 6;
	vtxt = "0000";
	// setValidator(&vldtr);
	setBase(16);
	setText(vtxt);
	connect(this, SIGNAL(textChanged(QString)), SLOT(onTextChange(QString)));
}

int xHexSpin::getValue() {
	return value;
}

void xHexSpin::updateMask() {
	int mx;
	QString rxp;
	switch(base) {
		case 8:
			rxp = "[0-7]";
			//setStyleSheet("border:1px solid red;");
			break;
		case 10:
			rxp = "[0-9]";
			//setStyleSheet("border:1px solid black;");
			break;
		default:
			base = 16;
			rxp = "[A-Fa-f0-9]";
			//setStyleSheet("border:1px solid green;");
			break;
	}
	len = 1;
	mx = base;
	while (mx <= max) {
		mx *= base;
		len++;
	}
	rxp.append(QString("{%0}").arg(len));	// 'len' times this char
	setInputMask(QString(len, 'h'));	// to enter overwrite cursor mode. TODO:is there some legit method?
	setRegExp(vldtr, rxp);
}

void xHexSpin::setBase(int b) {
	int tmp = value;
	base = b;
	updateMask();
	hsflag |= XHS_UPD;			// update even if value doesn't changed
	setValue(tmp);
}

void xHexSpin::setXFlag(int xf) {
	hsflag = xf;
}

int minMaxCorrect(int val, int min, int max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

void xHexSpin::setMin(int v) {
	min = v;
//	updateMask();		// no need if max is not updated
	if (value < min) setValue(min);
}

void xHexSpin::setMax(int v) {
	max = v;
	updateMask();
	if (value > max) setValue(max);
}

int xHexSpin::getMax() {
	return max;
}

extern QString getStyleString(QString, QString, int = 0, int = 100);

void xHexSpin::updatePal() {
#if 1
	QString str;
	if (changed) {
		str = getStyleString("dbg.changed.bg", "dbg.changed.txt");
	} else {
		str = ""; // getStyleString("dbg.input.bg", "dbg.input.txt");
	}
	setStyleSheet(str);
#else
	QPalette pal;
	if (changed) {
		pal.setColor(QPalette::Base, conf.pal["dbg.changed.bg"].isValid() ? conf.pal["dbg.changed.bg"] : pal.toolTipBase().color());
		pal.setColor(QPalette::Text, conf.pal["dbg.changed.txt"].isValid() ? conf.pal["dbg.changed.txt"] : pal.toolTipText().color());
	} else {
		pal.setColor(QPalette::Base, conf.pal["dbg.input.bg"].isValid() ? conf.pal["dbg.input.bg"] : pal.base().color());
		pal.setColor(QPalette::Text, conf.pal["dbg.input.txt"].isValid() ? conf.pal["dbg.input.txt"] : pal.text().color());
	}
	setPalette(pal);
#endif
}

void xHexSpin::setValue(int nval) {
	nval = minMaxCorrect(nval, min, max);
	if ((value == nval) && !(hsflag & XHS_UPD)) {
		changed = 0;
	} else {
		value = nval;
		changed = (hsflag & XHS_BGR) ? 1 : 0;
		emit valueChanged(nval);
		onChange(value);
	}
	updatePal();
}

void xHexSpin::onChange(int val) {
	int pos = cursorPosition();
	QString res = QString::number(val, base).toUpper();
	res = res.rightJustified(len, '0');
	if ((text() != res) || (hsflag & XHS_UPD)) {
		hsflag &= ~XHS_UPD;
		setText(res);
		setCursorPosition(pos);
	}
}

void xHexSpin::onTextChange(QString txt) {
	if (txt.size() < len) {
		txt = txt.leftJustified(len, '0');
	} else {
		txt = txt.left(len);
	}
	int pos = 0;
	if (vldtr.validate(txt, pos) == QValidator::Acceptable) {
		vtxt = txt;
		int nval = txt.toInt(NULL, base);
		int xval = minMaxCorrect(nval, min, max);
		if (value != xval)
			setValue(xval);
		else
			onChange(value);
	} else {
		pos = cursorPosition();		// Qt moves cursor at end of field after setText
		setText(vtxt);
		setCursorPosition(pos-1);
	}
}

void xHexSpin::keyPressEvent(QKeyEvent* ev) {
	QString txt;
	int pos;
	if (isReadOnly()) {
		QLineEdit::keyPressEvent(ev);
	} else {
		switch(ev->key()) {
			case Qt::Key_Up:
				setValue(minMaxCorrect(value + 1, min, max));
				break;
			case Qt::Key_Down:
				setValue(minMaxCorrect(value - 1, min, max));
				break;
			case Qt::Key_PageUp:
				setValue(minMaxCorrect(value + 0x100, min, max));
				break;
			case Qt::Key_PageDown:
				setValue(minMaxCorrect(value - 0x100, min, max));
				break;
			case Qt::Key_Insert:
				pos = cursorPosition();
				txt = vtxt;
				if (inputMask().isEmpty()) {
					setInputMask(QString(len,'H'));
				} else {
					setInputMask(QString());
				}
				setText(txt);
				setCursorPosition(pos);
				break;
			case Qt::Key_X:
				if (hsflag & XHS_DEC) {
					if (base == 8) {
						setBase(10);
					} else if (base == 10) {
						setBase(16);
					} else {
						setBase(8);
					}
				}
				break;
			default:
				QLineEdit::keyPressEvent(ev);
				break;
		}
	}
}

void xHexSpin::wheelEvent(QWheelEvent* ev) {
	if (isReadOnly()) return;
	if (ev->yDelta > 0) {
		setValue(minMaxCorrect(value + 1, min, max));
	} else if (ev->yDelta < 0) {
		setValue(minMaxCorrect(value - 1, min, max));
	}
	ev->accept();
}

// xLabel

xLabel::xLabel(QWidget* p):QLabel(p) {}

void xLabel::mousePressEvent(QMouseEvent* ev) {
	emit clicked(ev);
}

// xTreeBox

xTreeBox::xTreeBox(QWidget *p):QComboBox(p) {
	tree = new QTreeView;
	mod = new QFileSystemModel;
	setModel(mod);
	setView(tree);
	mod->setNameFilters(QStringList() << "*.rom" << "*.bin");
	mod->setReadOnly(true);
	mod->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
	mod->setNameFilterDisables(false);
	tree->setEditTriggers(QTableView::NoEditTriggers);
	tree->setSelectionBehavior(QAbstractItemView::SelectRows);
	tree->setSelectionMode(QAbstractItemView::SingleSelection);
	tree->header()->setVisible(false);
}

void xTreeBox::setDir(QString dir) {
	QModelIndex idx = mod->setRootPath(dir);
	setRootModelIndex(idx);
}

void xTreeBox::showPopup() {
	for (int i = 1; i < mod->columnCount(); i++) {
		tree->hideColumn(i);
	}
	QComboBox::showPopup();
}

void xTreeBox::hidePopup() {
	QModelIndex idx = tree->selectionModel()->currentIndex();
	QFileInfo inf = mod->fileInfo(idx);
	if (inf.isDir()) return;
	QComboBox::hidePopup();
}

void xTreeBox::setCurrentFile(QString path) {
	path.prepend(SLSH);
	path.prepend(mod->rootPath());
	QModelIndex idx = mod->index(path, 0);
	if (!idx.isValid()) return;
	QModelIndex x = rootModelIndex();
	setRootModelIndex(idx.parent());
	setModelColumn(0);
	setCurrentIndex(idx.row());
	setRootModelIndex(x);
	tree->setCurrentIndex(idx);
}

QString xTreeBox::currentFile() {
	QModelIndex idx = tree->selectionModel()->currentIndex();
	QFileInfo inf = mod->fileInfo(idx);
	return mod->rootDirectory().relativeFilePath(inf.filePath());
}

// base table model

xTableModel::xTableModel(QObject* p):QAbstractTableModel(p) {}

QModelIndex xTableModel::index(int row, int col, const QModelIndex& p) const {
	return createIndex(row, col);
}

void xTableModel::update() {
	emit dataChanged(index(0,0),index(rowCount() - 1, columnCount() - 1));
}

void xTableModel::updateRow(int row) {
	emit dataChanged(index(row, 0), index(row, columnCount() - 1));
}

void xTableModel::updateColumn(int col) {
	emit dataChanged(index(0, col), index(rowCount() - 1, col));
}

void xTableModel::updateCell(int row, int col) {
	emit dataChanged(index(row, col), index(row, col));
}

void xTableModel::setRows(int r) {
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

void xTableModel::setCols(int c) {
	if (c < col_count) {
		emit beginRemoveColumns(QModelIndex(), c, col_count);
		col_count = c;
		emit endRemoveColumns();
	} else if (c > col_count) {
		emit beginInsertColumns(QModelIndex(), col_count, c);
		col_count = c;
		emit endInsertColumns();
	}
}

int xTableModel::columnCount(const QModelIndex&) const {
	return col_count;
}

int xTableModel::rowCount(const QModelIndex&) const {
	return row_count;
}

// item delegate

xItemDelegate::xItemDelegate(int t) {
	type = t;
}

QWidget* xItemDelegate::createEditor(QWidget* par, const QStyleOptionViewItem&, const QModelIndex&) const {
	QLineEdit* edt = new QLineEdit(par);
	QString pat("[0-9A-Fa-f\\s]");
	int rpt = 0;
	switch (type) {
		case XTYPE_NONE: delete(edt); edt = NULL; break;
		case XTYPE_ADR: rpt = 4; break;
		case XTYPE_LABEL: break;
		case XTYPE_DUMP: rpt = 12; break;		// 6 bytes max
		case XTYPE_BYTE: rpt = 2; break;
		case XTYPE_OCTWRD: pat = "[0-7\\s]"; rpt = 6; break;
	}
	if (edt && (rpt > 0)) {
		edt->setInputMask(QString(rpt,'h'));
		edt->setMaxLength(rpt);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
		edt->setValidator(new QRegExpValidator(QRegExp(QString("%0+").arg(pat))));
#else
		edt->setValidator(new QRegularExpressionValidator(QRegularExpression(QString("%0+").arg(pat))));
#endif
	}
	return edt;
}

// dock widget

xDockWidget::xDockWidget(QString icopath, QString txt, QWidget* p):QDockWidget(p) {
	if (icopath.isEmpty()) {
		title = txt;
	} else {
		icon = QIcon(icopath);
	}
	setContextMenuPolicy(Qt::PreventContextMenu);
	titleWidget = new QLabel;
	titleWidget->setAlignment(Qt::AlignCenter);
	titleWidget->setText(txt);
	setTitleBarWidget(titleWidget);

	setFeatures(QDockWidget::DockWidgetMovable/* | QDockWidget::DockWidgetFloatable*/);
	connect(this, &xDockWidget::visibilityChanged, this, &xDockWidget::draw);
	connect(this, &xDockWidget::dockLocationChanged, this, &xDockWidget::moved);
}

// redraw all tabs icon/title
void xDockWidget::moved() {
	QList<QTabBar*> tabbars = parentWidget()->findChildren<QTabBar*>();
	QList<QTabBar*>::iterator it;
	xDockWidget* dw;
	int i;
	for(it = tabbars.begin(); it != tabbars.end(); it++) {
		for(i = 0; i < (*it)->count(); i++) {
			dw = reinterpret_cast<xDockWidget*>(qvariant_cast<quintptr>((*it)->tabData(i)));
			(*it)->setTabIcon(i, dw->icon);
			dw->setWindowTitle(dw->title);
		}
	}
}
