#include "xcore.h"

#include <QFile>
#include <QFileDialog>

#include <QDebug>

// labels

void clear_labels() {
	conf.prof.cur->labels.clear();
	conf.prof.cur->labmap.ram.clear();
	conf.prof.cur->labmap.rom.clear();
	conf.prof.cur->labmap.cpu.clear();
}

void del_label(QString name) {
	if (!conf.prof.cur->labels.contains(name)) return;
	xAdr xadr = conf.prof.cur->labels[name];
	conf.prof.cur->labels.remove(name);
	switch(xadr.type) {
		case MEM_RAM: conf.prof.cur->labmap.ram.remove(xadr.abs); break;
		// case MEM_ROM: conf.prof.cur->labmap.rom.remove(xadr.abs); break;
		default: conf.prof.cur->labmap.cpu.remove(xadr.adr); break;
	}
}

void add_label(xAdr xadr, QString name) {
	if (conf.prof.cur->labels.contains(name))
		del_label(name);
	conf.prof.cur->labels[name] = xadr;
	switch (xadr.type) {
		case MEM_RAM: conf.prof.cur->labmap.ram[xadr.abs] = name; break;
		//case MEM_ROM: conf.prof.cur->labmap.rom[xadr.abs] = name; break;
		default: conf.prof.cur->labmap.cpu[xadr.adr] = name; break;
	}
}

QString find_label(xAdr xadr) {
	QString lab;
	if (conf.dbg.labels) {
		switch(xadr.type) {
			case MEM_RAM: lab = conf.prof.cur->labmap.ram[xadr.abs]; break;
			// case MEM_ROM: lab = conf.labmap.rom[xadr.abs]; break;
			default: lab = conf.prof.cur->labmap.ram[xadr.abs];
				if (lab.isEmpty()) lab = conf.prof.cur->labmap.cpu[xadr.adr];
				break;
		}
	}
	return lab;
}

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
	if (!path.isEmpty()) {
		clear_labels();
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
					add_label(xadr, name);
					//
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
			keys = conf.prof.cur->labels.keys();
			foreach(key, keys) {
				xadr = conf.prof.cur->labels[key];
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

// comments

void add_comment(xAdr xadr, QString str) {
	switch(xadr.type) {
		case MEM_RAM: conf.prof.cur->comments.ram[xadr.abs] = str; break;
		case MEM_ROM: conf.prof.cur->comments.rom[xadr.abs] = str; break;
	}
}

void del_comment(xAdr xadr) {
	switch(xadr.type) {
		case MEM_RAM: conf.prof.cur->comments.ram.remove(xadr.abs); break;
		case MEM_ROM: conf.prof.cur->comments.rom.remove(xadr.abs); break;
	}
}

QString find_comment(xAdr xadr) {
	QString str;
	switch(xadr.type) {
		case MEM_RAM:
			if (conf.prof.cur->comments.ram.contains(xadr.abs)) {
				str = conf.prof.cur->comments.ram[xadr.abs];
			}
			break;
		case MEM_ROM:
			if (conf.prof.cur->comments.rom.contains(xadr.abs)) {
				str = conf.prof.cur->comments.rom[xadr.abs];
			}
			break;
	}
	return str;
}

void clear_comments() {
	conf.prof.cur->comments.ram.clear();
	conf.prof.cur->comments.rom.clear();
}
