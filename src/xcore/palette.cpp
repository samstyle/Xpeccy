#include "xcore.h"

#include <QFile>

// load preset colors for zx palette

QList<QColor> loadColors(std::string fname) {
	QFile file(toQString(conf.path.palette.find(fname)));
	if (!file.open(QFile::ReadOnly)) return {};
	QList<QColor> colors;
	while (!file.atEnd() && colors.size() < 16) {
		const QString line = file.readLine();
		const int pos = line.indexOf('#');
		if (pos < 0 || pos + 6 >= line.size()) continue;
		bool ok;
		const uint rgb = line.mid(pos + 1, 6).toUInt(&ok, 16);
		if (ok) colors.append(QColor::fromRgb((rgb >> 16) & 0xff,
		                                      (rgb >> 8)  & 0xff,
		                                      rgb         & 0xff));
	}
	return colors;
}

int saveColors(std::string fname, QList<QColor> pal) {
	if (pal.size() < 16) return ERR_SIZE;
	QFile file(toQString(conf.path.palette.writable / fname));
	if (!file.open(QFile::WriteOnly)) return ERR_CANT_OPEN;
	for (const QColor &col : pal) {
		file.write(QString("#%1%2%3\r\n")
		               .arg(gethexbyte(col.red()),
		                    gethexbyte(col.green()),
		                    gethexbyte(col.blue()))
		               .toLocal8Bit());
	}
	return ERR_OK;
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
