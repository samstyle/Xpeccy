#include "opt_romset.h"

#include <QDir>
#include <QDebug>

// ROMSET EDITOR

void fillRFBox(QComboBox*, QStringList);
std::string getRFText(QComboBox*);

xRomsetEditor::xRomsetEditor(QWidget* par):QDialog(par) {
	ui.setupUi(this);
	connect(ui.rse_apply, SIGNAL(clicked()), this, SLOT(store()));
	connect(ui.rse_cancel, SIGNAL(clicked()), this, SLOT(hide()));
}

void xRomsetEditor::edit(xRomFile f) {
	xrf = f;
	QDir rdir(QString(conf.path.romDir));
	QStringList rlst = rdir.entryList(QStringList() << "*.rom", QDir::Files, QDir::Name);
	QString str;
	ui.cbFile->clear();
	foreach(str, rlst) {
		ui.cbFile->addItem(str, str);
	}
	ui.cbFile->setCurrentIndex(rlst.indexOf(f.name.c_str()));
	ui.cbFoffset->setValue(f.foffset);
	ui.cbFsize->setValue(f.fsize);
	ui.cbRoffset->setValue(f.roffset);
	show();
}

void xRomsetEditor::store() {
	xrf.name = getRFText(ui.cbFile);
	if (xrf.name.empty()) return;
	xrf.foffset = ui.cbFoffset->value();
	xrf.fsize = ui.cbFsize->value();
	xrf.roffset = ui.cbRoffset->value();
	emit complete(xrf);
	hide();
}

// MODEL

xRomsetModel::xRomsetModel(QObject* p):QAbstractTableModel(p) {

}

int xRomsetModel::columnCount(const QModelIndex& idx) const {
	if (idx.isValid()) return 0;
	return 5;
}

int xRomsetModel::rowCount(const QModelIndex& idx) const {
	if (idx.isValid()) return 0;
	return rset.roms.size() + 2;
}

const QString xrsNamX[] = {"Page0","Page1","Page2","Page3","File","GS","Font"};

QVariant xRomsetModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= rowCount())) return res;
	if ((col < 0) || (col >= columnCount())) return res;
	int rlsz = (int)rset.roms.size();
	switch (role) {
		case Qt::DisplayRole:
			switch(col) {
				case 0:
					if (row < rlsz) {
						res = trUtf8("ROM");
					} else if (row == rlsz) {
						res = trUtf8("GS");
					} else {
						res = trUtf8("Font");
					}
					break;
				case 1:
					if (row < rlsz) {
						res = trUtf8(rset.roms[row].name.c_str());
					} else if (row == rlsz) {
						res = trUtf8(rset.gsFile.c_str());
					} else {
						res = trUtf8(rset.fntFile.c_str());
					}
					break;
				case 2:
					if (row >= rlsz) break;
					res = rset.roms[row].foffset;
					break;
				case 3:
					if (row >= rlsz) break;
					res = rset.roms[row].fsize;
					break;
				case 4:
					if (row >= rlsz) break;
					res = rset.roms[row].roffset;
					break;
			}
			break;
	}
	return res;
}

void xRomsetModel::update(xRomset rs) {
	rset = rs;
	endResetModel();
}
