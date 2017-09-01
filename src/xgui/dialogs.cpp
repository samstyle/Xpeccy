#include "xgui.h"
#include <QMessageBox>

void shitHappens(const char* msg) {
	QMessageBox mbx(QMessageBox::Critical,"Shit happens",QDialog::trUtf8(msg),QMessageBox::Ok);
	mbx.exec();
}

bool areSure(const char* msg) {
	QMessageBox mbx(QMessageBox::Question,"R U Sure?",QDialog::trUtf8(msg),QMessageBox::Yes | QMessageBox::No);
	int res = mbx.exec();
	return (res == QMessageBox::Yes);
}

void showInfo(const char* msg) {
	QMessageBox mbx(QMessageBox::Information,QDialog::trUtf8("Message"), QDialog::trUtf8(msg),QMessageBox::Ok);
	mbx.exec();
}
