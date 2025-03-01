#include "xcore.h"

#include <QFile>

// load preset colors for zx palette

QList<QColor> loadColors(std::string fname) {
	QList<QColor> list;
	QColor col;
	std::string path = conf.path.palDir + SLASH + fname;
	QFile file(path.c_str());
	int i = 0;
	QString line;
	QString hexPart;
	uint rgb;
	bool ok;
	int pos;
	if (file.open(QFile::ReadOnly)) {
		while (!file.atEnd() && (i < 16)) {
			line = file.readLine();
			pos = line.indexOf('#');
			if ((pos >= 0) && ((pos + 6) < line.size())) {
				hexPart = line.mid(pos + 1, 6);
				rgb = hexPart.toUInt(&ok, 16);
				if (ok) {
					col.setRed((rgb >> 16) & 0xff);
					col.setGreen((rgb >> 8) & 0xff);
					col.setBlue(rgb & 0xff);
					list.append(col);
					i++;
				}
			}
		}
		file.close();
	}
	return list;
}

int saveColors(std::string fname, QList<QColor> pal) {
	if (pal.size() < 16) return ERR_SIZE;
	std::string path = conf.path.palDir + SLASH + fname;
	QFile file(path.c_str());
	int err = ERR_OK;
	QColor col;
	QString str;
	if (file.open(QFile::WriteOnly)) {
		foreach(col, pal) {
			str = "#";
			str.append(gethexbyte(col.red()));
			str.append(gethexbyte(col.green()));
			str.append(gethexbyte(col.blue()));
			file.write(str.toLocal8Bit());
			file.write("\r\n");
		}
	} else {
		err = ERR_CANT_OPEN;
	}
	return err;
}

void loadPalette(xProfile* prf) {
//	printf("Loading palette: %s\n", prf->palette.c_str());
	Computer* comp = prf->zx;
	bool updateCurrentPallete = comp->vid->vmode == VID_NORMAL ? true : false;		// not necessary (VID_ALCO, VID_HWMC)
	updateCurrentPallete |= !!(comp->vid->vmode == VID_ALCO);
	updateCurrentPallete |= !!(comp->vid->vmode == VID_HWMC);
	QList<QColor> pal = loadColors(prf->palette);
	xColor xcol;
	int i;
	if (pal.size() == 16) {		// correct palette
		for (i = 0; i < 16; i++) {
			xcol.r = pal[i].red();
			xcol.g = pal[i].green();
			xcol.b = pal[i].blue();
			vid_set_bcol(comp->vid, i, xcol);
			if (updateCurrentPallete)
				vid_set_col(comp->vid, i, xcol);
		}
	} else {			// wrong palette, reset to default
		for (i = 0; i < 16; i++) {
			// TODO: review default color component value (0xaa), consider globaly defined value instead
			xcol.b = (i & 1) ? ((i & 8) ? 0xff : 0xaa) : 0x00;
			xcol.r = (i & 2) ? ((i & 8) ? 0xff : 0xaa) : 0x00;
			xcol.g = (i & 4) ? ((i & 8) ? 0xff : 0xaa) : 0x00;
			vid_set_bcol(comp->vid, i, xcol);
			if (updateCurrentPallete)
				vid_set_col(comp->vid, i, xcol);
		}
	}
}
