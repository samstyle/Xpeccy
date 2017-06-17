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

	connect(ui.rsName, SIGNAL(textChanged(QString)), this, SLOT(check()));
	connect(ui.rse_grp_single, SIGNAL(toggled(bool)), this, SLOT(grpSingle(bool)));
	connect(ui.rse_grp_separate, SIGNAL(toggled(bool)), this, SLOT(grpSeparate(bool)));
}

void xRomsetEditor::grpSingle(bool st) {
	ui.rse_grp_separate->setChecked(!st);
}

void xRomsetEditor::grpSeparate(bool st) {
	ui.rse_grp_single->setChecked(!st);
}

void xRomsetEditor::edit(int ix) {
	if (ix >= (signed)conf.rsList.size())
		ix = -1;
	idx = ix;
	int i;
	if (idx >= 0) {
		nrs = conf.rsList[idx];
	} else {
		nrs.name.clear();
		nrs.file.clear();
		nrs.gsFile.clear();
		nrs.fntFile.clear();
		for (i = 0; i < 4; i++) {
			nrs.roms[i].path.clear();
			nrs.roms[i].part = 0;
		}
	}

	QDir rdir(QString(conf.path.romDir.c_str()));
	QStringList rlst = rdir.entryList(QStringList() << "*.rom", QDir::Files, QDir::Name);
	fillRFBox(ui.rse_singlefile,rlst);
	fillRFBox(ui.rse_file0,rlst);
	fillRFBox(ui.rse_file1,rlst);
	fillRFBox(ui.rse_file2,rlst);
	fillRFBox(ui.rse_file3,rlst);
	fillRFBox(ui.rse_gsfile,rlst);
	fillRFBox(ui.rse_fntfile,rlst);

	ui.rsName->setText(nrs.name.c_str());
	ui.rse_singlefile->setCurrentIndex(rlst.indexOf(QString(nrs.file.c_str())) + 1);
	ui.rse_file0->setCurrentIndex(rlst.indexOf(QString(nrs.roms[0].path.c_str())) + 1);
	ui.rse_file1->setCurrentIndex(rlst.indexOf(QString(nrs.roms[1].path.c_str())) + 1);
	ui.rse_file2->setCurrentIndex(rlst.indexOf(QString(nrs.roms[2].path.c_str())) + 1);
	ui.rse_file3->setCurrentIndex(rlst.indexOf(QString(nrs.roms[3].path.c_str())) + 1);
	ui.rse_part0->setValue(nrs.roms[0].part);
	ui.rse_part1->setValue(nrs.roms[1].part);
	ui.rse_part2->setValue(nrs.roms[2].part);
	ui.rse_part3->setValue(nrs.roms[3].part);
	ui.rse_gsfile->setCurrentIndex(rlst.indexOf(QString(nrs.gsFile.c_str())) + 1);
	ui.rse_fntfile->setCurrentIndex(rlst.indexOf(QString(nrs.fntFile.c_str())) + 1);
	ui.rse_grp_separate->setChecked(nrs.file.empty());
	ui.rse_grp_single->setChecked(!nrs.file.empty());

	show();
}

void xRomsetEditor::check() {
	std::string nam = ui.rsName->text().toStdString();
	bool fl = true;
	if (nam.empty()) {
		fl = false;
	} else {
		for(unsigned int i = 0; i < conf.rsList.size(); i++) {
			if ((conf.rsList[i].name == nam) && (idx != (int)i)) {
				fl = false;
			}
		}
	}
	ui.rse_apply->setEnabled(fl);
}

void xRomsetEditor::store() {
	if (ui.rse_grp_single->isChecked()) {
		nrs.file = getRFText(ui.rse_singlefile);
	} else {
		nrs.file.clear();
	}
	nrs.roms[0].path = getRFText(ui.rse_file0);
	nrs.roms[0].part = ui.rse_part0->value();
	nrs.roms[1].path = getRFText(ui.rse_file1);
	nrs.roms[1].part = ui.rse_part1->value();
	nrs.roms[2].path = getRFText(ui.rse_file2);
	nrs.roms[2].part = ui.rse_part2->value();
	nrs.roms[3].path = getRFText(ui.rse_file3);
	nrs.roms[3].part = ui.rse_part3->value();
	nrs.gsFile = getRFText(ui.rse_gsfile);
	nrs.fntFile = getRFText(ui.rse_fntfile);
	nrs.name = ui.rsName->text().toStdString();

	if (idx < 0) {
		addRomset(nrs);
		emit complete(0, trUtf8(nrs.name.c_str()));
	} else {
		prfChangeRsName(conf.rsList[idx].name, nrs.name);
		conf.rsList[idx] = nrs;
		emit complete(1, QString());
	}
	hide();
}

// MODEL

xRomsetModel::xRomsetModel(QObject* p):QAbstractTableModel(p) {

}

int xRomsetModel::columnCount(const QModelIndex& idx) const {
	if (idx.isValid()) return 0;
	return 3;
}

int xRomsetModel::rowCount(const QModelIndex& idx) const {
	if (idx.isValid()) return 0;
	return 7;
}

const QString xrsNamX[] = {"Page0","Page1","Page2","Page3","File","GS","Font"};

QVariant xRomsetModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	if (!idx.isValid()) return res;
	int row = idx.row();
	int col = idx.column();
	if ((row < 0) || (row >= rowCount())) return res;
	if ((col < 0) || (col >= columnCount())) return res;
	switch (role) {
		case Qt::DisplayRole:
			switch(col) {
				case 0:
					res = xrsNamX[row];
					break;
				case 1:
					switch(row) {
						case 0:
						case 1:
						case 2:
						case 3: res = trUtf8(rset.roms[row].path.c_str());
							break;
						case 4: res = trUtf8(rset.file.c_str()); break;
						case 5: res = trUtf8(rset.gsFile.c_str()); break;
						case 6: res = trUtf8(rset.fntFile.c_str()); break;
					}
					break;
				case 2:
					if (row > 3) break;
					res = QString::number(rset.roms[row].part);
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
