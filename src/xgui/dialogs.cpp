#include "xgui.h"
#include "../xcore/xcore.h"
#include <QMessageBox>

void shitHappens(const char* msg) {
	if (!conf.running) return;
	QMessageBox mbx(QMessageBox::Critical,"Shit happens",QDialog::trUtf8(msg),QMessageBox::Ok);
	mbx.exec();
}

bool areSure(const char* msg) {
	QMessageBox mbx(QMessageBox::Question,"Yes? No? What?",QDialog::trUtf8(msg),QMessageBox::Yes | QMessageBox::No);
	int res = mbx.exec();
	return (res == QMessageBox::Yes);
}

void showInfo(const char* msg) {
	QMessageBox mbx(QMessageBox::Information,"Message", QDialog::trUtf8(msg),QMessageBox::Ok);
	mbx.exec();
}

int askYNC(const char* msg) {
	QMessageBox mbx(QMessageBox::Question,"That is the question", QDialog::trUtf8(msg), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	return mbx.exec();
}
