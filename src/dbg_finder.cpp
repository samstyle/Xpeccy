#include "dbg_finder.h"
#include "xcore/xcore.h"

xMemFinder::xMemFinder(QWidget* p):QDialog(p) {
	ui.setupUi(this);

	connect(ui.leBytes, SIGNAL(textEdited(QString)),this,SLOT(onBytesEdit()));
	connect(ui.leText, SIGNAL(textEdited(QString)),this,SLOT(onTextEdit()));
	connect(ui.pbFind, SIGNAL(clicked(bool)),this,SLOT(doFind()));
}

void xMemFinder::onTextEdit() {
	QString str;
	QString txt = ui.leText->text();
	QChar ch;
	int i;
	int len = txt.size();
	if (len < 1) {
		str = "00";
	} else {
		if (len > 8) len = 8;
		for (i = 0; i < len; i++) {
			ch = txt.at(i);
			if (i != 0)
				str.append(":");
			str.append(gethexbyte(ch.toAscii()));
		}
	}
	ui.leBytes->setText(str);
}

void xMemFinder::onBytesEdit() {
	QString str;
	int ch;
	QStringList tlst = ui.leBytes->text().split(":",QString::SkipEmptyParts);
	QString sbt;
	while (!tlst.isEmpty()) {
		sbt = tlst.takeFirst();
		ch = sbt.toInt(NULL, 16) & 0xff;
		if ((ch < 32) || (ch > 127)) {
			str.append(".");
		} else {
			str.append(QChar(ch));
		}
	}
	ui.leText->setText(str);
}

void xMemFinder::doFind() {
	ui.labResult->setText("");
	QStringList strl = ui.leBytes->text().split(":",QString::SkipEmptyParts);
	QStringList strm = ui.leMask->text().split(":",QString::SkipEmptyParts);
	int psiz = strl.size();
	if (psiz == 0) return;
	unsigned char pat[8];
	unsigned char msk[8];
	int idx = 0;
	for (idx = 0; idx < 8; idx++) {	// fill arrays
		if (idx < psiz) {
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
	int shift = 0;
	idx = 0;
	int found = 0;
	unsigned char bt;
	while (!found && (shift < 0x10000)) {
		bt = memRd(mem, (adr + shift) & 0xffff);
		if ((bt & msk[idx]) == (pat[idx] & msk[idx])) {
			idx++;
			if (idx >= psiz)
				found = 1;
		} else {
			idx = 0;
		}
        shift++;
	}
	if (found) {
		adr = adr + shift - psiz;
		ui.labResult->setText(QString("Found @ %0").arg(gethexword(adr)));
		emit patFound(adr);
	} else {
		ui.labResult->setText("Not found");
	}
}
