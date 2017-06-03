#include "dbg_memfill.h"
#include "../../xcore/xcore.h"

#include <QDebug>

xMemFiller::xMemFiller(QWidget* p):QDialog(p) {
	ui.setupUi(this);

	connect(ui.sbStart, SIGNAL(valueChanged(int)),this,SLOT(adrChange()));
	connect(ui.sbEnd, SIGNAL(valueChanged(int)),this,SLOT(adrChange()));
	connect(ui.leStartHex, SIGNAL(textChanged(QString)),this,SLOT(hexChange()));
	connect(ui.leEndHex, SIGNAL(textChanged(QString)),this,SLOT(hexChange()));
	connect(ui.cbMethod, SIGNAL(currentIndexChanged(int)),this,SLOT(metChange()));

	connect(ui.pbFill,SIGNAL(clicked(bool)),this,SLOT(fill()));
}

void xMemFiller::start(Memory* me, int bgn, int end) {
	mem = me;
	if ((bgn > -1) && (end > -1)) {
		ui.sbStart->setValue(bgn);
		ui.sbEnd->setValue(end);
	}
	show();
}

void xMemFiller::adrChange() {
	QString str = gethexword(ui.sbStart->value()).toUpper();
	if (ui.leStartHex->text().toUpper() != str)
		ui.leStartHex->setText(str);
	str = gethexword(ui.sbEnd->value()).toUpper();
	if (ui.leEndHex->text().toUpper() != str)
		ui.leEndHex->setText(str);
	ui.pbFill->setEnabled(ui.sbStart->value() <= ui.sbEnd->value());
}

void xMemFiller::hexChange() {
	ui.sbStart->setValue(ui.leStartHex->text().toInt(NULL, 16));
	ui.sbEnd->setValue(ui.leEndHex->text().toInt(NULL, 16));
	ui.pbFill->setEnabled(ui.sbStart->value() <= ui.sbEnd->value());
}

void xMemFiller::metChange() {
	ui.leMask->setEnabled(ui.cbMethod->currentText() == "Mask");
}

void xMemFiller::fill() {
	QStringList strl = ui.leBytes->text().split(":",QString::SkipEmptyParts);
	QStringList strm = ui.leMask->text().split(":",QString::SkipEmptyParts);
	int psiz = strl.size();		// pattern size
	if (psiz == 0) return;
	unsigned char pat[8];		// pattern bytes
	unsigned char msk[8];		// mask bytes
	int idx = 0;
	for (idx = 0; idx < 8; idx++) {	// fill arrays
		if (idx < strl.size()) {
			pat[idx] = strl[idx].toInt(NULL, 16);
		} else {
			pat[idx] = 0x00;
		}
		if (idx < strm.size()) {
			msk[idx] = strm[idx].toInt(NULL, 16);
		} else {
			msk[idx] = 0xff;
		}
	}
	int adr = ui.sbStart->value();	// start addr
	int end = ui.sbEnd->value();	// end addr
	unsigned char byt;
	idx = 0;
	do {				// fill by pattern and mask
		byt = memRd(mem, adr & 0xffff);
		switch(ui.cbMethod->currentIndex()) {
			case 0:				// mask
				byt = (byt & ~msk[idx]) | (pat[idx] & msk[idx]);
				break;
			case 1:				// put
				byt = pat[idx];
				break;
			case 2:				// or
				byt |= pat[idx];
				break;
			case 3:				// and
				byt &= pat[idx];
				break;
			case 4:				// xor
				byt ^= pat[idx];
				break;
		}
		memWr(mem, adr & 0xffff, byt);
		idx++;
		if (idx >= psiz)
			idx = 0;
		adr++;
	} while (adr <= end);
	emit rqRefill();
	close();
}
