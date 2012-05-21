#include "xcore.h"

std::vector<XProfile> profileList;
XProfile* currentProfile = NULL;
extern ZXComp* zx;

void addProfile(std::string nm, std::string fp) {
	XProfile nprof;
	nprof.name = nm;
	nprof.file = fp;
	nprof.zx = zxCreate();
	profileList.push_back(nprof);
}

bool setProfile(std::string nm) {
	for (unsigned int i=0; i<profileList.size(); i++) {
		if (profileList[i].name == nm) {
			currentProfile = &profileList[i];
			zx = currentProfile->zx;
			return true;
		}
	}
	return false;
}

void clearProfiles() {
	XProfile defprof = profileList[0];
	profileList.clear();
	profileList.push_back(defprof);
}

std::vector<XProfile> getProfileList() {
	return profileList;
}

XProfile* getCurrentProfile() {
	return currentProfile;
}
