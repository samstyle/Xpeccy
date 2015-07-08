#include "xcore.h"

void addBookmark(std::string nm, std::string fp) {
	xBookmark nbm;
	nbm.name = nm;
	nbm.path = fp;
	conf.bookmarkList.push_back(nbm);
}

void swapBookmarks(int p1, int p2) {
	xBookmark bm = conf.bookmarkList[p1];
	conf.bookmarkList[p1] = conf.bookmarkList[p2];
	conf.bookmarkList[p2] = bm;
}

void setBookmark(int idx,std::string nm, std::string fp) {
	conf.bookmarkList[idx].name = nm;
	conf.bookmarkList[idx].path = fp;
}

void delBookmark(int idx) {
	conf.bookmarkList.erase(conf.bookmarkList.begin() + idx);
}

void clearBookmarks() {
	conf.bookmarkList.clear();
}
