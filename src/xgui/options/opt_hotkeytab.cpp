#include "opt_hotkeytab.h"
#include "../../xcore/xcore.h"

#include <QVBoxLayout>

// model

xHotkeyModel::xHotkeyModel(QObject* p):xTableModel(p) {
	rows = 0;
	xShortcut* tab = shortcut_tab();
	while (tab[rows].text != NULL) {
		rows++;
	}
}

int xHotkeyModel::columnCount(const QModelIndex&) const {
	return 2;
}

int xHotkeyModel::rowCount(const QModelIndex&) const {
	return rows;
}

QVariant xHotkeyModel::data(const QModelIndex& idx, int role) const {
	QVariant var;
	if (!idx.isValid()) return var;
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= rows)) return var;
	xShortcut* tab = shortcut_tab();
	switch (role) {
		case Qt::DisplayRole:
			if (col == 0) {
				var = tab[row].text;
			} else {
				var = tab[row].seq.toString();
			}
			break;
	}
	return var;
}

/*
void xHotkeyModel::updateCell(int row, int col) {
	emit dataChanged(index(row, col), index(row, col));
}
*/

// table

xHotkeyTable::xHotkeyTable(QWidget* p):QTableView(p) {
	model = new xHotkeyModel();
	edt = new xKeyEditor();
	setModel(model);

	connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(dbl_click(QModelIndex)));
	connect(edt, SIGNAL(s_done(int, QKeySequence)), this, SLOT(set_seq(int, QKeySequence)));
}

void xHotkeyTable::dbl_click(QModelIndex idx) {
	if (!idx.isValid()) return;
	int row = idx.row();
	if (row < 0) return;
	if (row >= model->rowCount()) return;
	xShortcut* tab = shortcut_tab();
	edt->edit(tab[row].id);
}

void xHotkeyTable::set_seq(int id, QKeySequence seq) {
	set_shortcut_id(id, seq);
	xShortcut* tab = shortcut_tab();
	int i = 0;
	while ((tab[i].id >= 0) && (tab[i].id != id))
		i++;
	if (tab[i].id > 0)
		model->updateCell(i, 1);
}

// editor

xKeyEditor::xKeyEditor(QWidget* p):QDialog(p) {
	QVBoxLayout* lay = new QVBoxLayout;
	QHBoxLayout* hbx = new QHBoxLayout;
	lab.clear();
	but.setIcon(QIcon(":/images/ok-apply.png"));
	but.setText("Confirm");
	clr.setIcon(QIcon(":/images/cancel.png"));
	clr.setText("Clear");
	lab.setAlignment(Qt::AlignCenter);
	lay->addWidget(&lab);
	hbx->addWidget(&clr);
	hbx->addWidget(&but);
	lay->addLayout(hbx);
	setLayout(lay);
	setModal(true);
	connect(&clr, SIGNAL(released()), this, SLOT(clear()));
	connect(&but, SIGNAL(released()), this, SLOT(okay()));
}

void xKeyEditor::keyPressEvent(QKeyEvent* ev) {
	QString str;
	if (ev->modifiers() & Qt::AltModifier) str += "Alt + ";
	if (ev->modifiers() & Qt::ControlModifier) str += "Ctrl + ";
	if (ev->modifiers() & Qt::ShiftModifier) str += "Shift + ";
	if (ev->modifiers() & Qt::MetaModifier) str += "Meta + ";
	switch (ev->key()) {
		case Qt::Key_Alt:
		case Qt::Key_Multi_key:		// TODO: this is right-alt?
		case Qt::Key_Shift:
		case Qt::Key_Control:
		case Qt::Key_Meta:
			kseq = QKeySequence();
			break;
		default:
			str += QKeySequence(ev->key()).toString();
			kseq = QKeySequence(ev->key() | ev->modifiers());
			break;
	}
	if (str.isEmpty()) str = QString("<press now>");
	lab.setText(str);
}

void xKeyEditor::keyReleaseEvent(QKeyEvent* ev) {
	if (kseq.isEmpty())
		lab.setText("<press now>");
}

void xKeyEditor::edit(int f) {
	foo = f;
	xShortcut* cut = find_shortcut_id(f);
	kseq = cut->seq;
	lab.setText(kseq.isEmpty() ? "<no key>" : kseq.toString());
	grabKeyboard();
	show();
}

void xKeyEditor::clear() {
	kseq = QKeySequence();
	lab.setText("<no key>");
}

void xKeyEditor::okay() {
	emit s_done(foo, kseq);
	reject();
}

void xKeyEditor::reject() {
	releaseKeyboard();
	hide();
}
