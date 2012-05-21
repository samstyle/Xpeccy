#include "xcore.h"

std::vector<XBookmark> bookmarkList;

void addBookmark(std::string nm, std::string fp) {
	XBookmark nbm;
	nbm.name = nm;
	nbm.path = fp;
	bookmarkList.push_back(nbm);
}

void swapBookmarks(int p1, int p2) {
	XBookmark bm = bookmarkList[p1];
	bookmarkList[p1] = bookmarkList[p2];
	bookmarkList[p2] = bm;
}

void setBookmark(int idx,std::string nm, std::string fp) {
	bookmarkList[idx].name = nm;
	bookmarkList[idx].path = fp;
}

void delBookmark(int idx) {
	bookmarkList.erase(bookmarkList.begin() + idx);
}

void clearBookmarks() {
	bookmarkList.clear();
}

int getBookmarksCount() {
	return bookmarkList.size();
}

std::vector<XBookmark> getBookmarkList() {
	return bookmarkList;
}
