#include "bdi.h"
//#include "z80.h"

#include <fstream>

#include <QMessageBox>
#include <QChar>
#include <QString>
#include <QDebug>

#include "filer.h"

uint8_t trd_8e1[] = {
	0x00,0x00,0x01,0x16,0x00,0xf0,0x09,0x10,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00
};

Floppy::Floppy() {
	trk80 = true;
	dblsid = true;
}

void Floppy::wr(uint8_t val) {
	data[rtrk].byte[pos] = val; changed=true;
}

uint8_t Floppy::rd() {
	return data[rtrk].byte[pos];
}

uint8_t Floppy::getfield() {
	return data[rtrk].field[pos];
}

void Floppy::step(bool dir) {
	if (dir) {if (trk < (trk80?86:43)) trk++;}
	else {if (trk>0) trk--;}
}
/*
void Floppy::tick() {
	index=false;
	rtrk = trk<<1; if (dblsid) rtrk += bdi->vg93.side;
	if (insert) {
		if (pos==0 && spin==0) iback=250;
		if (iback>0) iback--;
		index = (iback>0);
		if (++spin > (bdi->turbo?(((bdi->vg93.cop & 0xf8)!=0x50)?3:95):112)) {
			spin=0;
			if (++pos>=TRACKLEN) pos=0;
			field = fnext;
			fnext = data[rtrk].field[pos+1];
		}
	}
}
*/
void Floppy::next() {
	rtrk = trk<<1; if (dblsid) rtrk += bdi->vg93.side?0:1;
	if (insert) {
		if (++pos >= TRACKLEN) {
			pos = 0;
			ti = bdi->t;		// tick of index begin
		}
		field = data[rtrk].field[pos];
	} else {
		field = 0;
	}
}

void Floppy::format() {
	int32_t i;
	uint8_t *buf = new uint8_t[0x1000];
	for (i=0;i<0x1000;i++) buf[i]=0x00;
	for (i=1;i<168;i++) formtrdtrack(i,buf);
	memcpy(buf + 0x8e0, trd_8e1, 0x20);
	formtrdtrack(0,buf);
}

void Floppy::nulltrack(uint8_t tr) {
	for (int32_t i=0; i<TRACKLEN; i++) data[tr].byte[i] = 0x00;
}

void Floppy::formtrdtrack(uint8_t tr, uint8_t *bpos) {
	std::vector<Sector> lst;
	Sector sct;
	uint8_t* ppos = bpos;
	sct.cyl = ((tr&0xfe)>>1);
	sct.side = (tr&0x01)?1:0;
	sct.len = 1;
	int32_t sc;
	for (sc=1; sc<17; sc++) {
		sct.sec = sc;
		sct.data = ppos;
		lst.push_back(sct);
		ppos += 256;
	}
	formtrack(tr,lst);
}

uint16_t Floppy::getcrc(uint8_t* ptr, int32_t len) {
	uint32_t crc = 0xcdb4;
	int32_t i;
	while (len--) {
		crc ^= *ptr << 8;
		for (i = 0; i<8 ; i++) {
			if ((crc *= 2) & 0x10000) crc ^= 0x1021;
		}
		ptr++;
	}
	return  (crc & 0xffff);
}

void Floppy::formtrack(uint8_t tr, std::vector<Sector> sdata) {
	uint8_t *ppos = data[tr].byte;
	uint8_t *cpos;
	uint16_t crc;
	int32_t i,ln;
	uint32_t sc;
	for (i=0; i<12; i++) *(ppos++) = 0x00;		// 12	space
	*(ppos++) = 0xc2; *(ppos++) = 0xc2;		// 	track mark
	*(ppos++) = 0xc2; *(ppos++) = 0xfc;
	for (sc=0; sc<sdata.size(); sc++) {
		for(i=0;i<10;i++) *(ppos++) = 0x4e;		// 10	sync
		for(i=0;i<12;i++) *(ppos++) = 0x00;		// 12	space
		*(ppos++) = 0xa1;				//	address mark
		*(ppos++) = 0xa1;
		*(ppos++) = 0xa1;
		cpos = ppos;
		*(ppos++) = 0xfe;
		*(ppos++) = sdata[sc].cyl;			// 	addr field
		*(ppos++) = sdata[sc].side;
		*(ppos++) = sdata[sc].sec;
		*(ppos++) = sdata[sc].len;
		crc = getcrc(cpos,ppos - cpos);
		*(ppos++) = 0xf7; *(ppos++) = 0xf7;
//		*(ppos++) = ((crc & 0xff00) >> 8); *(ppos++) = (crc & 0xff);
		for(i=0;i<22;i++) *(ppos++) = 0x4e;		// 22	sync
		for(i=0;i<12;i++) *(ppos++) = 0x00;		// 12	space
		*(ppos++) = 0xa1;				//	data mark
		*(ppos++) = 0xa1;
		*(ppos++) = 0xa1;
		cpos = ppos;
		*(ppos++) = sdata[sc].type;
		ln = (128 << sdata[sc].len);			//	data
		for (i=0; i<ln; i++) *(ppos++) = sdata[sc].data[i];
		*(ppos++) = 0xf7; *(ppos++) = 0xf7;
/*
		if (sdata[sc].crc < 0) {
			crc = getcrc(cpos, ppos - cpos);
			*(ppos++) = ((crc & 0xff00) >> 8);
			*(ppos++) = (crc & 0xff);
		} else {
			*(ppos++) = ((sdata[sc].crc & 0xff00) >> 8);
			*(ppos++) = sdata[sc].crc & 0xff;
		}
*/
		for(i=0;i<60;i++) *(ppos++) = 0x4e;		// 60	sync
	}
	while ((ppos - data[tr].byte) < TRACKLEN) *(ppos++) = 0x4e;		// ?	last sync
	fillfields(tr,true);
}

// эта фигня пересканирует дорожку и намечает номера полей данных. надо вызывать после записи(формирования) дорожки
void Floppy::fillfields(uint8_t tr, bool fcrc) {
	int i, bcnt = 0, sct = 1;
	uint8_t fld = 0;
	uint8_t* cpos = &data[tr].byte[0];
	uint8_t* bpos = cpos;
	ushort crc;
	for (i=0;i<TRACKLEN;i++) {
		data[tr].field[i] = fld;
		if (fcrc) {
			switch (fld) {
				case 0:
					if ((*bpos) == 0xf5) {*bpos = 0xa1;}
					if ((*bpos) == 0xf6) {*bpos = 0xc2;}
					break;
				case 4:
					if (*bpos == 0xf7) {
						crc = getcrc(cpos, bpos - cpos);
						*bpos = ((crc & 0xff00) >> 8);
						*(bpos+1) = (crc & 0xff);
					}
					break;
			}
		}
		if (bcnt > 0) {
			bcnt--;
			if (bcnt==0) {
				if (fld < 4) {
					fld = 4; bcnt = 2;
				} else {
					fld = 0;
				}
			}
		} else {
			if (data[tr].byte[i] == 0xfe) {cpos = bpos; fld = 1; bcnt = 4; sct = data[tr].byte[i+4];}
			if (data[tr].byte[i] == 0xfb) {cpos = bpos; fld = 2; bcnt = (128<<sct);}
			if (data[tr].byte[i] == 0xf8) {cpos = bpos; fld = 3; bcnt = (128<<sct);}
		}
		bpos++;
	}
}

bool Floppy::eject() {
	if (!savecha()) return false;
	path = "";
	insert = false;
	changed = false;
	return true;
}

bool Floppy::savecha() {
	bool res=true;
	if (changed) {
		QMessageBox mbox;
		mbox.setText(QString("<b>Disk ").append(QChar('A'+id)).append(": has been changed</b>"));
		mbox.setInformativeText("Do you want to save it?");
		mbox.setStandardButtons(QMessageBox::Yes|QMessageBox::Ignore|QMessageBox::Cancel);
		mbox.setIcon(QMessageBox::Warning);
		int32_t ret=mbox.exec();
		if (ret==QMessageBox::Yes) {res = filer->savedisk(path,id,true);}	// save
		if (ret==QMessageBox::Ignore) res=true;					// don't save
		if (ret==QMessageBox::Cancel) res=false;				// cancel
	}
	return res;
}

void Floppy::loaduditrack(std::ifstream* file, uint8_t tr, bool sd) {
	int32_t rt = (tr << 1) + (sd?1:0);
	uint8_t* buf = new uint8_t[4];
//printf("pos %.8X, trk %i\n",(int)file->tellg(),rt);
	file->read((char*)buf,1);
	int32_t len;
	if (*buf != 0x00) {
		printf("TRK %i: unknown format %.2X\n",rt,*buf);
		file->read((char*)buf,4);
		len = *buf + (*(buf+1) << 8) + (*(buf+2) << 16) + (*(buf+3) << 24);	// field len
		file->seekg(len,std::ios_base::cur);					// skip unknown field
	} else {
		file->read((char*)buf,2);
		len = *buf + (*(buf+1) << 8);	// track size
		if (len > TRACKLEN) {
			printf("TRK %i: too long (%i)\n",rt,len);
			file->seekg(len,std::ios_base::cur);		// skip track image
			len = (len >> 3) + (((len & 7) == 0)?0:1);	// and bit field
			file->seekg(len,std::ios_base::cur);
		} else {
			nulltrack(rt);
			file->read((char*)data[rt].byte,len);		// read track head 0
			fillfields(rt,false);
			len = (len >> 3) + (((len & 7) == 0)?0:1);	// skip bit field
			file->seekg(len,std::ios_base::cur);
		}
	}
}

void Floppy::load(std::string sfnam, uint8_t type) {
	if (!savecha()) return;
	std::ifstream file(sfnam.c_str(),std::ios::binary);
	QMessageBox mbx; mbx.setIcon(QMessageBox::Critical);
	if (!file.good()) {
		mbx.setText("Can't open file");
		mbx.setInformativeText(sfnam.c_str());
		mbx.exec();
		return;
	}
	int32_t i,j,scnt;
	uint8_t* buf = new uint8_t[0x1000];
	uint8_t* bptr;
	uint8_t fcnt,tmp;
	uint16_t tmpa,tmpb,slen;
	size_t tmpd,tmph,tmpt,tmps,cpos;
	bool err;
	uint64_t len;
	Sector sct;
	std::vector<Sector> trkimg;
	switch(type) {
		case TYPE_FDI:
			mbx.setText("Wrong FDI image");
			file.read((char*)buf,14);
			if (std::string((const char*)buf,3) != "FDI") {mbx.setInformativeText("Wrong signature"); mbx.exec(); break;}
			err = (buf[3] != 0);			// write protect
			tmpa = buf[4] + (buf[5] << 8);		// cylinders
			tmpb = buf[6] + (buf[7] << 8);		// heads
			if ((tmpb != 1) && (tmpb != 2)) {mbx.setInformativeText("Incorrect heads count"); mbx.exec(); break;}
			tmpd = buf[10] + (buf[11] << 8);	// sectors data pos
			tmph = buf[12] + (buf[13] << 8) + 14;	// track headers data pos
			file.seekg(tmph);			// read tracks data
			for (i=0; i<tmpa; i++) {
				for (j=0; j<tmpb; j++) {
//printf("TRK %i HD %i\n",i,j);
					trkimg.clear();
					file.read((char*)buf,4);
					tmpt = tmpd + buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);	// track data pos
					file.seekg(2,std::ios_base::cur);		// skip 2 bytes
					file.get((char&)tmp);			// sectors in disk;
					for (scnt=0; scnt < tmp; scnt++) {
//printf("SEC %i\n",scnt);
						file.get((char&)sct.cyl);
						file.get((char&)sct.side);
						file.get((char&)sct.sec);
						file.get((char&)sct.len);
						file.get((char&)fcnt);				// flag
						sct.type = (fcnt & 0x80) ? 0xf8 : 0xfb;
						file.read((char*)buf,2);
						tmps = tmpt + buf[0] + (buf[1] << 8);	// sector data pos
						cpos = file.tellg();			// remember current pos
						file.seekg(tmps);
						slen = (128 << sct.len);		// sector len
						sct.data = new uint8_t[slen];
						file.read((char*)sct.data,slen);	// read sector data
						file.seekg(cpos);
//printf("C H S L = %i %i %i %i; TYP %.2X; DATA:\n",sct.cyl,sct.side,sct.sec,sct.len,sct.type);
//for (int k=0; k<slen; k++) printf("%.2X  ",*(sct.data + k));
//printf("\n");
						trkimg.push_back(sct);
					}
					formtrack((i << 1) + j, trkimg);
//					throw(0);
				}
			}
			insert = true;
			path = sfnam;
			changed = false;
			break;
		case TYPE_UDI:
			mbx.setText("Wrong UDI image");
			file.read((char*)buf,16);
			if (std::string((const char*)buf,4) != "UDI!") {mbx.setInformativeText("Wrong signature"); mbx.exec(); break;}
			if (*(buf+8) != 0x00) {mbx.setInformativeText("Wrong version"); mbx.exec(); break;}
			tmp = *(buf+9);			// max trk
			err = (*(buf+10)==0x01);	// true if double side
			for (i=0; i<tmp+1; i++) {
				loaduditrack(&file,i,false);
				if (err) loaduditrack(&file,i,true);
			}
			insert = true;
			path = sfnam;
			changed = false;
			break;
		case TYPE_TRD:
			file.seekg(0x8e7);
			tmp = file.peek();
			file.seekg(0,std::ios::end);
			len = file.tellg();
			file.seekg(0,std::ios::beg);
			err = ((len&0xfff)!=0) || (len==0) || (len>0xa8000);
			if (err || tmp!=0x10) {
				mbx.setText("<b>Wrong TRD file</b>");
				if (err) {mbx.setInformativeText("Incorrect lenght");}
					else {mbx.setInformativeText("Not TRDos image");}
				mbx.exec();
			} else {
				format();
				i=0;
				do {
					file.read((char*)buf,0x1000);
					formtrdtrack(i,buf);
					i++;
				} while (!file.eof());
				insert = true;
				path = sfnam;
				changed = false;
			}
			break;
		case TYPE_SCL:
			file.read((char*)buf,9);
			mbx.setText("<b>Wrong SCL file</b>");
			if (std::string((const char*)buf,8) != "SINCLAIR") {mbx.setInformativeText("Wrong signature"); mbx.exec(); break;}
			if (buf[8]>0x80) {mbx.setInformativeText("More than 128 files inside"); mbx.exec(); break;}
			format();
			fcnt = buf[8];
			for (i=0;i<0x1000;i++) buf[i]=0x00;
			scnt = 0x10;
			bptr = buf;
			for (i=0;i<fcnt;i++) {
				file.read((char*)bptr,14);
				*(bptr+14) = scnt & 0x0f;
				*(bptr+15) = ((scnt & ~0x0f) >> 4);
				scnt += *(bptr+13);
//printf("%.8s.%.1s\t%.2X.%.2X/%.2X\n",bptr,bptr+8,*(bptr+13),*(bptr+14),*(bptr+15));
				bptr += 16;
			}
			*(bptr)=0;
			buf[0x800] = 0;
			buf[0x8e1] = scnt & 0x0f;
			buf[0x8e2] = ((scnt & 0xf0) >> 4);
			buf[0x8e3]=0x16;
			buf[0x8e4]=fcnt;
			tmpa = 0xa00 - scnt;
			buf[0x8e5] = (tmpa & 0xff);
			buf[0x8e6]=((tmpa & 0xff00) >> 8);
			buf[0x8e7]=0x10;
			formtrdtrack(0,buf);
			i=1;
			while (!file.eof()) {
				file.read((char*)buf,0x1000);
				formtrdtrack(i,buf);
				i++;
			}
			insert = true;
			path = sfnam;
			changed=false;
			break;
	}
}

void Floppy::getudibitfield(uint8_t tr, uint8_t* buf) {
	int i,msk=0x01;
	for (i=0; i<TRACKLEN; i++) {
		if (msk == 0x100) {msk = 0x01; *(++buf)=0x00;}
		if ((data[tr].field[i] == 0) && (data[tr].byte[i] == 0xa1)) *buf |= msk;
		msk <<= 1;
	}
	buf++;
}

std::vector<Sector> Floppy::getsectors(uint8_t tr) {
	std::vector<Sector> res;
	Sector sec;
	int32_t len,p = 0;
	do {
		if (data[tr].field[p] == 1) {
			sec.cyl = data[tr].field[p++];
			sec.side = data[tr].field[p++];
			sec.sec = data[tr].field[p++];
			sec.len = data[tr].field[p];
			do {
				p++;
			} while ((p < TRACKLEN) && (data[tr].field[p] != 2) && (data[tr].field[p] != 3));
			if (p < TRACKLEN) {
				sec.type = data[tr].field[p-1];
				len = (128 << sec.len);
				sec.data = new uint8_t[len];
				memcpy(sec.data, &data[tr].byte[p],len);
				p += len;
				sec.crc = (data[tr].byte[p] << 8) + data[tr].byte[p+1]; p += 2;
				res.push_back(sec);
			}
		}
	} while (p < TRACKLEN);
	return res;
}

bool Floppy::getsector(uint8_t tr,uint8_t sc,uint8_t* buf) {
	int32_t tpos = 0;
	bool fnd;
	while (1) {
		while (data[tr].field[tpos] != 1) {
			if (++tpos >= TRACKLEN) return false;
		}
		fnd = (data[tr].byte[tpos+2] == sc);
		tpos += 6;
		while ((data[tr].field[tpos] != 2) && (data[tr].field[tpos] != 3)) {
			if (++tpos >= TRACKLEN) return false;
		}
		if (fnd) {
			for (uint32_t i=0;i<256;i++) *(buf++) = data[tr].byte[tpos++];
			return true;
		}
		tpos += 0x102;
	}
}

struct FilePos {
	uint8_t trk;
	uint8_t sec;
	uint8_t slen;
};

// crc32 for UDI, taken from Unreal 0.32.7
void crc32(int &crc, uint8_t *buf, unsigned len) {
	while (len--) {
		crc ^= -1 ^ *buf++;
		for(int k = 8; k--; ) {
			int temp = -(crc & 1); crc >>= 1, crc ^= 0xEDB88320 & temp;
		}
	crc ^= -1;
	}
}

void putint(uint8_t* ptr, uint32_t val) {
	*ptr++ = val & 0xff;
	*ptr++ = (val & 0xff00) >> 8;
	*ptr++ = (val & 0xff0000) >> 16;
	*ptr++ = (val & 0xff000000) >> 24;
}

void Floppy::save(std::string fnam, uint8_t type) {
	if (!insert) return;
	QMessageBox mbx;
	mbx.setIcon(QMessageBox::Critical);
	const char* sign = "SINCLAIR";
	const char* sudi = "UDI!";
	uint8_t* img = new uint8_t[0x200000];		// 0x112cf4 for 160 tracks in UDI
	FilePos newfp;
	std::vector<FilePos> fplist;
	uint8_t* buf = new uint8_t[256];
	uint8_t* bptr;
	uint8_t* dptr = img;
	int32_t i,j;
	uint8_t tr,sc;
	std::ofstream file;
	switch (type) {
		case TYPE_UDI:
			memcpy(dptr,sudi,4);
			bptr = img+4;				// bptr - pointer to "file len" field
			dptr += 8;
			*(dptr++) = 0x00;			// version
			*(dptr++) = trk80?79:39;		// maximun track number
			*(dptr++) = dblsid?0x01:0x00;		// double side (due to floppy property)
			*(dptr++) = 0x00;
			*(dptr++) = 0x00; *(dptr++) = 0x00; *(dptr++) = 0x00; *(dptr++) = 0x00;
			for (i=0; i < (trk80?160:80); i++) {
				*(dptr++) = 0x00;		// MFM
				*(dptr++) = (TRACKLEN & 0xff);	// track len
				*(dptr++) = ((TRACKLEN & 0xff00) >> 8);
				memcpy((char*)dptr,(char*)data[i].byte,TRACKLEN);	// track image
				dptr += TRACKLEN;
				getudibitfield(i,dptr);
				dptr += 782;			// 6250 / 8 + 1
				if (!dblsid) i++;		// if single-side skip 
			}
			i = dptr - img;
			putint(bptr,i);
			j=-1; crc32(j,img,i);
		printf("crc = %X\n",j);
			putint(dptr,j); dptr += 4;
			file.open(fnam.c_str(),std::ios::binary);
			file.write((char*)img,dptr-img);
			file.close();
			break;
		case TYPE_TRD:
			for (i=0; i<160; i++) {
				for (j=1; j<17; j++) {
					if (!getsector(i,j,dptr)) {
						mbx.setText(QString("<b>Error parsing disk</b><br>Sector %1:%2 not found").arg(i).arg(j));
						mbx.exec();
						i=j=200;
						break;
					}
					dptr+=256;
				}
			}
			if (i<200) {file.open(fnam.c_str(),std::ios::binary); file.write((const char*)img,0xa0000);}
			break;
		case TYPE_SCL:
			memcpy(img,sign,8);
			img[8]=0;
			dptr = img+9;
			for (i=1; i<9; i++) {
				getsector(0,i,buf);
				bptr = buf;
				for (j=0; j<16; j++) {
					if (*bptr == 0) {i=j=20;}
					else {
						if (*bptr != 1) {
							memcpy(dptr,bptr,14);
							newfp.trk = *(bptr+15);
							newfp.sec = *(bptr+14);
							newfp.slen = *(bptr+13);
							fplist.push_back(newfp);
							dptr+=14; img[8]++;
						}
						bptr+=16;
					}
				}
			}
			for (i=0;i<(int)fplist.size();i++) {
				tr = fplist[i].trk;
				sc = fplist[i].sec;
				for(j=0;j<fplist[i].slen;j++) {
					getsector(tr,sc+1,dptr);
					dptr+=256;
					sc++; if (sc>15) {tr++; sc=0;}
				}
			}
			j=0;
			for(i = 0; i < dptr - img; i++) {
				j += *(img + i);
			}
			putint(dptr, j);
			file.open(fnam.c_str(),std::ios::binary);
			file.write((char*)img,dptr-img);
			file.close();
			break;
	}
}

Sector::Sector() {
	type = 0xfb;
	crc = -1;
}

Sector::Sector(uint8_t p1,uint8_t p2,uint8_t p3,uint8_t p4,uint8_t* p5) {
	cyl = p1; side = p2; sec = p3; len = p4; data = p5;
	type = 0xfb;
	crc = -1;
}
