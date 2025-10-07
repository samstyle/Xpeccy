#include "xcore.h"
#include "../xgui/xgui.h"

#include <QFile>
#include <QBuffer>

int qfgeti(QFile& f) {
	char c;
	int v;
	f.getChar(&c);
	v = c & 0xff;
	f.getChar(&c);
	v |= (c & 0xff) << 8;
	f.getChar(&c);
	v |= (c & 0xff) << 16;
	f.getChar(&c);
	v |= (c & 0xff) << 24;
	return v;
}

int xadr_type_by_name(QString str, int t = -1) {
	if (str == "RAM") {
		t = MEM_RAM;
	} else if (str == "ROM") {
		t = MEM_ROM;
	} else if (str == "SLT") {
		t = MEM_SLOT;
	} else if (str == "IO") {
		t = MEM_IO;
	}
	return t;
}

void load_xmap(QString path) {
	QFile file;
	char buf[8];
	int len;
	QByteArray arr;
	QBuffer qb;
	char line[1024];
	QString str;
	QStringList lst;
	xAdr xadr;
	Computer* comp = conf.prof.cur->zx;
	file.setFileName(path);
	if (file.open(QFile::ReadOnly)) {
		file.read(buf, 8);
		if (memcmp(buf, "XMEMMAP ", 8)) {
			shitHappens("File signature mismatching");
		} else {
			while(!file.atEnd()) {
				file.read(buf, 8);
				len = qfgeti(file);
				if (!memcmp(buf, "ramflags", 8)) {
					if (len <= MEM_4M) {
						file.read((char*)comp->brkRamMap, len);
					} else {
						file.read((char*)comp->brkRamMap, MEM_4M);
						file.seek(file.pos() + len - MEM_4M);
					}
				} else if (!memcmp(buf, "romflags", 8)) {
					if (len <= MEM_512K) {
						file.read((char*)comp->brkRomMap, len);
					} else {
						file.read((char*)comp->brkRomMap, MEM_512K);
						file.seek(file.pos() + len - MEM_512K);
					}
				} else if (!memcmp(buf, "sltflags", 8)) {
					if (comp->slot->brkMap) {
						if (len <= comp->slot->memMask) {
							file.read((char*)comp->slot->brkMap, len);
						} else {
							file.read((char*)comp->slot->brkMap, comp->slot->memMask + 1);
							file.seek(file.pos() + len - comp->slot->memMask - 1);
						}
					}
				} else if (!memcmp(buf, "labels  ", 8)) {
					clear_labels();
					arr = file.read(len);
					qb.setData(arr);
					qb.open(QIODevice::ReadOnly);
					while(!qb.atEnd()) {
						memset(line, 0, 1024);
						qb.readLine(line, 1024);
						str = QString::fromUtf8(line).trimmed();
						lst = str.split(":", X_SkipEmptyParts);
						if (lst.size() > 2) {
							str = lst.at(0);
							xadr.type = xadr_type_by_name(str, -1);
							if (xadr.type >= 0) {
								xadr.abs = lst.at(1).toInt(nullptr, 16);
								xadr.adr = xadr.abs & 0xffff;
								xadr.bank = xadr.abs >> 8;
								add_label(xadr, lst.at(2));
							}
						}
					}
					qb.close();
				} else if (!memcmp(buf, "comments", 8)) {
					clear_comments();
					arr = file.read(len);
					qb.setData(arr);
					qb.open(QIODevice::ReadOnly);
					while(!qb.atEnd()) {
						memset(line, 0, 1024);
						qb.readLine(line, 1024);
						str = QString::fromUtf8(line).trimmed();
						lst = str.split(":", X_SkipEmptyParts);
						if (lst.size() > 2) {
							str = lst.at(0);
							xadr.type = xadr_type_by_name(str, MEM_EXT);
							xadr.abs = lst.at(1).toInt(nullptr, 16);
							str = lst.at(2);
							if (!str.isEmpty()) {
								add_comment(xadr, str);
								// conf.dbg.comments[xadr.abs] = str;
							}
						}
					}
					qb.close();
				} else {
					file.seek(file.pos() + len);
					//file.skip(len);
				}
			}
			brkInstallAll();
		}
	} else {
		shitHappens("Can't open this file for reading");
	}
}

void qfputi(QFile& f, int v) {
	f.putChar(v & 0xff);
	f.putChar((v >> 8) & 0xff);
	f.putChar((v >> 16) & 0xff);
	f.putChar((v >> 24) & 0xff);
}

void xputcomments(QByteArray& arr, QMap<int, QString>* map, QString prefix) {
	int adr;
	QString str;
	foreach(adr, map->keys()) {
		str = prefix;
		str.append(":");
		str.append(QString::number(adr, 16).rightJustified(8, '0'));
		str.append(":");
		str.append(map->value(adr));
		arr.append(str.toUtf8());
		arr.append((char)0x0a);
	}
}

void save_xmap(QString path) {
	QFile file;
	QByteArray arr;
	int adr;
	xAdr xadr;
	QString str;
	QString lab;
	Computer* comp = conf.prof.cur->zx;
	if (!path.isEmpty()) {
		file.setFileName(path);
		if (file.open(QFile::WriteOnly)) {
			// signature
			file.write("XMEMMAP ", 8);
			// ram map
			file.write("ramflags", 8);
			qfputi(file, comp->mem->ramMask + 1);				// to aviod zx48k with 64K ram and 128K mask
			file.write((char*)comp->brkRamMap, comp->mem->ramMask + 1);
			// rom map
			file.write("romflags", 8);
			qfputi(file, comp->mem->romSize);
			file.write((char*)comp->brkRomMap, comp->mem->romSize);
			// cartrige map
			file.write("sltflags", 8);
			if (comp->slot->brkMap) {
				qfputi(file, comp->slot->memMask + 1);
				file.write((char*)comp->slot->brkMap, comp->slot->memMask + 1);
			} else {
				qfputi(file, 0);
			}
			// labels
			file.write("labels  ", 8);
			foreach(lab, conf.prof.cur->labels.keys()) {
				xadr = conf.prof.cur->labels[lab];
				switch(xadr.type) {
					case MEM_RAM: str = "RAM:"; adr = xadr.abs; break;
					case MEM_ROM: str = "ROM:"; adr = xadr.abs; break;
					case MEM_SLOT: str = "SLT:"; adr = xadr.abs; break;
					case MEM_IO: str = "IO:"; adr = xadr.abs; break;
					default: str.clear(); adr = 0; break;
				}
				if (!str.isEmpty()) {
					str.append(QString::number(adr, 16).rightJustified(8, '0'));
					str.append(":").append(lab);
					arr.append(str.toUtf8());
					arr.append((char)0x0a);
				}
			}
			qfputi(file, arr.size());
			file.write(arr.data(), arr.size());
			arr.clear();
			// comments
			file.write("comments", 8);
			xputcomments(arr, &conf.prof.cur->commap[MEM_RAM], "RAM");
			xputcomments(arr, &conf.prof.cur->commap[MEM_ROM], "ROM");
			xputcomments(arr, &conf.prof.cur->commap[MEM_SLOT], "SLT");
			xputcomments(arr, &conf.prof.cur->commap[MEM_IO], "IO");
			xputcomments(arr, &conf.prof.cur->commap[MEM_EXT], "EXT");
			qfputi(file, arr.size());
			file.write(arr.data(), arr.size());
		} else {
			shitHappens("Can't open this file for writing");
		}
	}
}
