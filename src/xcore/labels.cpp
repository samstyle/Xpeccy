#include "xcore.h"

#include <QFile>
#include <QFileDialog>

#include <QDebug>

int loadLabels(const char* fn) {
	int res = 1;
	QString path(fn);
	QString line;
	QString name;
	QStringList arr;
	QFile file;
	xAdr xadr;
	if (path.isEmpty())
		path = QFileDialog::getOpenFileName(NULL, "Load SJASM labels",QString(),QString(),nullptr,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) {
		res = 0;			// no file specified
	} else {
		conf.labels.clear();
		file.setFileName(path);
		if (file.open(QFile::ReadOnly)) {
			while(!file.atEnd()) {
				line = file.readLine();
				if (line.startsWith(":"))
					line.prepend("FF");
				arr = line.split(QRegExp("[: \r\n]"),X_SkipEmptyParts);
				if (arr.size() > 2) {
					xadr.type = MEM_RAM;
					xadr.bank = arr.at(0).toInt(NULL,16);
					xadr.adr = arr.at(1).toInt(NULL,16);
					if (xadr.bank == 0xff) {
						switch (xadr.adr & 0xc000) {
							case 0x0000: break;
							case 0x4000: xadr.bank = 5; break;
							case 0x8000: xadr.bank = 2; break;
							case 0xc000: xadr.bank = 0; break;
						}
					}
					xadr.adr &= 0x3fff;
					xadr.abs = (xadr.bank << 14) | xadr.adr;
					name = arr.at(2);
					switch (xadr.bank) {
						case 0xff:
							xadr.type = MEM_ROM;
							xadr.bank = -1;
							break;
						case 0x05:
							xadr.adr |= 0x4000;
							break;
						case 0x02:
							xadr.adr |= 0x8000;
							break;
						default:
							xadr.adr |= 0xc000;
							break;
					}
					if (xadr.bank > 0)
						xadr.bank = xadr.abs >> 8;
					conf.labels[name] = xadr;
				}
			}
			conf.labpath = path;
		} else {
			res = 0;		// can't open file
		}
	}
	return res;
}

int saveLabels(const char* fn) {
	int res = 1;
	QStringList keys;
	QString key;
	xAdr xadr;
	QString line;
	QFile file;
	QString path(fn);
	if (path.isEmpty())
		path = QFileDialog::getSaveFileName(NULL, "save SJASM labels",QString(),QString(),nullptr,QFileDialog::DontUseNativeDialog);
	if (path.isEmpty()) {
		res = 0;
	} else {
		file.setFileName(path);
		if (file.open(QFile::WriteOnly)) {
			keys = conf.labels.keys();
			foreach(key, keys) {
				xadr = conf.labels[key];
				line = (xadr.type == MEM_RAM) ? gethexbyte(xadr.abs >> 14) : "FF";
				line.append(QString(":%0 %1\n").arg(gethexword(xadr.abs & 0x3fff), key));
				file.write(line.toUtf8());
			}
			file.close();
		} else {
			res = 0;
		}
	}
	return res;
}

QString findLabel(int adr, int type, int bank) {
	QString lab;
	if (!conf.dbg.labels)
		return lab;
	QString key;
	xAdr xadr;
	QStringList keys = conf.labels.keys();
	foreach(key, keys) {
		xadr = conf.labels[key];
		if ((xadr.adr == adr) \
				&& ((type < 0) || (xadr.type < 0) || (type == xadr.type))\
				&& ((bank < 0) || (xadr.bank < 0) || (bank == xadr.bank))) {
			lab = key;
			break;
		}
	}
	return lab;
}

xAdr getLabel(const char* n) {
	QString name(n);
	xAdr adr;
	if (conf.labels.contains(name)) {
		adr = conf.labels[name];
	} else {
		adr.abs = -1;		// not found
	}
	return adr;
}
