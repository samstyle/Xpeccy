#include "dbg_widgets.h"

#include <QPainter>

//QImage dbgNesScreenImg(Video*, unsigned short, unsigned short);
//QImage dbgNesSpriteImg(Video*, unsigned short);
//QImage dbgNesTilesImg(Video*, unsigned short);

enum {
	NES_SCR_OFF = 0x00,
	NES_SCR_0,
	NES_SCR_1,
	NES_SCR_2,
	NES_SCR_3,
	NES_SCR_ALL,
	NES_SCR_TILES
};

enum {
	NES_TILE_0000 = 0x0000,
	NES_TILE_1000 = 0x1000
};

int getRFIData(QComboBox*);

xPPUWidget::xPPUWidget(QString i, QString t, QWidget* p):xDockWidget(i,t,p) {
	QWidget* wid = new QWidget;
	setWidget(wid);
	ui.setupUi(wid);
	setObjectName("NESPPUWIDGET");

	hwList << HWG_NES;

	ui.nesScrType->addItem("BG off", NES_SCR_OFF);
	ui.nesScrType->addItem("BG scr 0", NES_SCR_0);
	ui.nesScrType->addItem("BG scr 1", NES_SCR_1);
	ui.nesScrType->addItem("BG scr 2", NES_SCR_2);
	ui.nesScrType->addItem("BG scr 3", NES_SCR_3);
	ui.nesScrType->addItem("All in 1", NES_SCR_ALL);
	ui.nesScrType->addItem("Tiles", NES_SCR_TILES);

	ui.nesBGTileset->addItem("Tiles #0000", NES_TILE_0000);
	ui.nesBGTileset->addItem("Tiles #1000", NES_TILE_1000);
	ui.nesSPTileset->addItem("No sprites", -1);
	ui.nesSPTileset->addItem("Sprites #0000", NES_TILE_0000);
	ui.nesSPTileset->addItem("Sprites #1000", NES_TILE_1000);

	connect(ui.nesScrType, &QComboBox::currentIndexChanged, this, &xPPUWidget::draw);
	connect(ui.nesBGTileset, &QComboBox::currentIndexChanged, this, &xPPUWidget::draw);
	connect(ui.nesSPTileset, &QComboBox::currentIndexChanged, this, &xPPUWidget::draw);
}

extern xColor nesPal[64];

void dbgNesConvertColors(Video* vid, unsigned char* buf, QImage& img, int trn) {
	int x, y;
	unsigned char col, colidx;
	xColor xcol;
	int adr = 0;
	for (y = 0; y < img.height(); y++) {
		for (x = 0; x < img.width(); x++) {
			colidx = buf[adr];
			if (!(colidx & 3)) colidx = 0;
			col = vid->ram[0x3f00 | (colidx & 0x3f)];
			xcol = nesPal[col & 0x3f];
			if (trn && !(colidx & 3)) {
				img.setPixel(x, y, qRgba(255, 0, 0, 0));
			} else {
				img.setPixel(x, y, qRgba(xcol.r, xcol.g, xcol.b, 0xff));
			}
			adr++;
		}
	}
}

QImage dbgNesScreenImg(Video* vid, unsigned short adr, unsigned short tadr) {
	QImage img(256, 240, QImage::Format_RGB888);
	img.fill(Qt::black);
	unsigned char scrmap[256 * 240];
	memset(scrmap, 0x00, 256 * 240);
	if (adr != 0) {
		for(int y = 0; y < 240; y++) {
			ppuRenderBGLine(vid, scrmap + (y << 8), adr, 0, tadr);
			adr = ppuYinc(adr);
		}
	}
	dbgNesConvertColors(vid, scrmap, img, 0);
	return img;
}

QImage dbgNesSpriteImg(Video* vid, unsigned short tadr) {
	QImage img(256, 240, QImage::Format_ARGB32);
	img.fill(Qt::transparent);
	unsigned char scrmap[256 * 240];
	memset(scrmap, 0x00, 256 * 240);
	for (int y = 0; y < 240; y++) {
		ppuRenderSpriteLine(vid, y + 2, scrmap + (y << 8), NULL, tadr, 8);
	}
	dbgNesConvertColors(vid, scrmap, img, 1);
	return img;
}

QImage dbgNesTilesImg(Video* vid, unsigned short tadr) {
	QImage img(256, 256, QImage::Format_RGB888);
	unsigned char scrmap[256 * 256];
	int x,y,lin,bit;
	int adr = tadr;
	int oadr = 0;
	unsigned char col;
	unsigned short bt;
	img.fill(Qt::black);
	for (y = 0; y < 256; y += 8) {
		for (x = 0; x < 256; x += 8) {
			for (lin = 0; lin < 8; lin++) {
				bt = nes_ppu_ext_rd(adr + lin, conf.prof.cur->zx) & 0xff;
				bt |= (nes_ppu_ext_rd(adr + lin + 8, conf.prof.cur->zx) << 8) & 0xff00;
				for (bit = 0; bit < 8; bit++) {
					col = ((bt >> 7) & 1) | ((bt >> 14) & 2);
					scrmap[oadr + (lin << 8) + bit] = col;
					bt <<= 1;
				}
			}
			oadr += 8;
			adr += 16;		// next sprite
		}
		oadr += 0x700;			// next line (8 lines / sprite)
	}
	dbgNesConvertColors(vid, scrmap, img, 0);
	return img;
}

void xPPUWidget::draw() {
	Computer* comp = conf.prof.cur->zx;
	unsigned short adr = 0;
	unsigned short tadr = 0;
	QPixmap pic;

	unsigned short vidvadr = comp->vid->vadr;
	unsigned short vidtadr = comp->vid->tadr;

	// screen
	int type = getRFIData(ui.nesScrType);
	int sprt = getRFIData(ui.nesSPTileset);
	switch(type) {
		case NES_SCR_OFF: adr = 0; break;
		case NES_SCR_0:	adr = 0x2000; break;
		case NES_SCR_1: adr = 0x2400; break;
		case NES_SCR_2: adr = 0x2800; break;
		case NES_SCR_3: adr = 0x2c00; break;
	}
	tadr = getRFIData(ui.nesBGTileset) & 0xffff;
	pic = QPixmap(256, 240);
	pic.fill(Qt::black);
	QPainter pnt(&pic);
	switch (type) {
		case NES_SCR_ALL:
			pnt.drawImage(0, 0, dbgNesScreenImg(comp->vid, 0x2000, tadr).scaled(128, 120));
			pnt.drawImage(128, 0, dbgNesScreenImg(comp->vid, 0x2400, tadr).scaled(128, 120));
			pnt.drawImage(0, 120, dbgNesScreenImg(comp->vid, 0x2800, tadr).scaled(128, 120));
			pnt.drawImage(128, 120, dbgNesScreenImg(comp->vid, 0x2c00, tadr).scaled(128, 120));
			break;
		case NES_SCR_TILES:
			pnt.drawImage(0, 0, dbgNesTilesImg(comp->vid, tadr));
			break;
		case NES_SCR_OFF:
			if (sprt > -1)
				pnt.drawImage(0, 0, dbgNesSpriteImg(comp->vid, sprt & 0xffff));
			break;
		case NES_SCR_0:
		case NES_SCR_1:
		case NES_SCR_2:
		case NES_SCR_3:
			pnt.drawImage(0, 0, dbgNesScreenImg(comp->vid, (comp->vid->tadr & ~0x2c00) | adr, tadr));
			if (sprt > -1)
				pnt.drawImage(0, 0, dbgNesSpriteImg(comp->vid, sprt & 0xffff));
			break;
	}
	pnt.end();
	ui.nesScreen->setPixmap(pic);

	comp->vid->vadr = vidvadr;
	comp->vid->tadr = vidtadr;

	// registers
	ui.lab_ppu_r0->setText(gethexbyte(comp->vid->reg[0]));
	ui.lab_ppu_r1->setText(gethexbyte(comp->vid->reg[1]));
	ui.lab_ppu_r2->setText(gethexbyte(comp->vid->reg[2]));
	ui.lab_ppu_r3->setText(gethexbyte(comp->vid->reg[3]));
	ui.lab_ppu_r4->setText(gethexbyte(comp->vid->reg[4]));
	ui.lab_ppu_r5->setText(gethexbyte(comp->vid->reg[5]));
	ui.lab_ppu_r6->setText(gethexbyte(comp->vid->reg[6]));
	ui.lab_ppu_r7->setText(gethexbyte(comp->vid->reg[7]));
	// vadr
	ui.labVAdr->setText(gethexword(comp->vid->vadr & 0x7fff));
	ui.labTAdr->setText(gethexword(comp->vid->tadr & 0x7fff));
}
