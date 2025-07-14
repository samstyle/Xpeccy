#include "dbg_widgets.h"

#include <QPainter>

xGameboyWidget::xGameboyWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("GAMEBOYWIDGET");
	hwList << HWG_GB;
	connect(ui.gbModeGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(draw()));
	connect(ui.sbTileset, SIGNAL(valueChanged(int)), this, SLOT(draw()));
	connect(ui.sbTilemap, SIGNAL(valueChanged(int)), this, SLOT(draw()));
}

extern xColor iniCol[4];
void drawGBTile(QImage& img, Video* vid, int x, int y, int adr) {
	int row, bit;
	int data;
	unsigned char col;
	xColor xcol;
	for (row = 0; row < 8; row++) {
		data = vid->ram[adr & 0x3fff] & 0xff;
		adr++;
		data |= (vid->ram[adr & 0x3fff] & 0xff) << 8;
		adr++;
		for (bit = 0; bit < 8; bit++) {
			col = ((data & 0x80) ? 1 : 0) | ((data & 0x8000) ? 2 : 0);
			xcol = iniCol[col];
			img.setPixel(x + bit, y + row, qRgb(xcol.r, xcol.g, xcol.b));
			data <<= 1;
		}
	}
}

QImage getGBTiles(Video* vid, int tset) {
	int tadr = (tset & 1) ? 0x800 : 0;
	if (tset & 2) tadr |= 0x2000;
	int x,y;
	QImage img(128, 128, QImage::Format_RGB888);
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			drawGBTile(img, vid, x << 3, y << 3, tadr);
			tadr += 16;
		}
	}
	return img.scaled(256,256);
}

QImage getGBMap(Video* vid, int tmap, int tset) {
	QImage img(256, 256, QImage::Format_RGB888);
	img.fill(qRgb(0,0,0));
	int adr = tmap ? 0x1c00 : 0x1800;
	int badr = (tset & 1) ? 0x800 : 0;
	if (tset & 2) badr |= 0x2000;
	int tadr;
	unsigned char tile;
	int x,y;
	for (y = 0; y < 32; y++) {
		for (x = 0; x < 32; x++) {
			tile = vid->ram[adr & 0x1fff];
			adr++;
			tadr = badr;
			if (tset & 1) {
				tadr += (tile ^ 0x80) << 4;
			} else {
				tadr += tile << 4;
			}
			drawGBTile(img, vid, x << 3, y << 3, tadr);
		}
	}
	return img;
}

QImage getGBPal(Video* gbv) {
	QImage img(256,256,QImage::Format_RGB888);
	img.fill(Qt::black);
	int x,y;
	int idx = 0;
	xColor col;
	QPainter pnt;
	pnt.begin(&img);
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 4; x++) {
			col = vid_get_col(gbv, idx);
			idx++;
			pnt.fillRect((x << 6) + 1, (y << 4) + 1, 62, 14, QColor(col.r, col.g, col.b));
			if (idx == 32) idx += 32;
		}
	}
	pnt.end();
	return img;
}

void xGameboyWidget::draw() {
	Computer* comp = conf.prof.cur->zx;
	QImage img;
	int tset = ui.sbTileset->value();
	int tmap = ui.sbTilemap->value();
	if (ui.rbTilesetView->isChecked()) {
		img = getGBTiles(comp->vid, tset);
	} else if (ui.rbTilemapView->isChecked()) {
		img = getGBMap(comp->vid, tmap, tset);
	} else {
		img = getGBPal(comp->vid);
	}
	ui.gbImage->setPixmap(QPixmap::fromImage(img));
}

// gb video registers

typedef struct {
	QString name;
	int port;
} xGBPortDsc;

QList<xGBPortDsc> gbvPortList = {{"LCDC",0xff40},{"STAT",0xff41},{"SCY",0xff42}, {"SCX",0xff43},{"LY",0xff44},{"LYC",0xff45},\
				 {"BGP",0xff47},{"OBP0",0xff48},{"OBP1",0xff49},{"WX",0xff4a},{"WY",0xff4b}};

xGBVideoModel::xGBVideoModel(QObject* p):xTableModel(p) {}
int xGBVideoModel::rowCount(const QModelIndex &) const {return gbvPortList.size();}
int xGBVideoModel::columnCount(const QModelIndex &) const {return 3;}
QVariant xGBVideoModel::data(const QModelIndex& idx, int role) const {
	QVariant res;
	Computer* comp = conf.prof.cur->zx;
	int row = idx.row();
	int col = idx.column();
	switch(role) {
		case Qt::DisplayRole:
			switch (col) {
				case 0: res = QString("%0 (%1)").arg(gbvPortList[row].name).arg(gethexword(gbvPortList[row].port)); break;
				case 1:	res = gethexbyte(comp->hw->mrd(comp, gbvPortList.at(row).port, 0)); break;		// hex
				case 2: res = getbinbyte(comp->hw->mrd(comp, gbvPortList.at(row).port, 0)); break;		// bin
			}
			break;
	}
	return res;
}

void xGBVideoWidget::draw() {
	((xTableModel*)(ui.table->model()))->update();
}

xGBVideoWidget::xGBVideoWidget(QString n, QString i, QWidget* p):xDockWidget(n, i, p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	ui.table->setModel(new xGBVideoModel);
	ui.table->horizontalHeader()->setVisible(false);
	ui.table->verticalHeader()->setVisible(false);
	ui.table->horizontalHeader()->setStretchLastSection(true);
	ui.table->setColumnWidth(0, 150);
	setObjectName("GBVWIDGET");
	hwList << HWG_GB;
}
