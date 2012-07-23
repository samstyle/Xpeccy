#include "xcore.h"
#include <stdio.h>

XProfile* currentProfile = NULL;
std::vector<XProfile> profileList;

int findProfile(std::string nm) {
	int idx = 0;
	while (idx < (int)profileList.size()) {
		if (profileList[idx].name == nm) return idx;
		idx++;
	}
	return -1;
}

XProfile* getProfile(std::string nm) {
	int idx = findProfile(nm);
	if (idx < 0) return NULL;
	return &profileList[idx];
}

bool addProfile(std::string nm, std::string fp) {
	printf("add Profile: %s : %s\n",nm.c_str(),fp.c_str());
	int idx = findProfile(nm);
	if (idx > -1) return false;
	XProfile nprof;
	nprof.name = nm;
	nprof.file = fp;
	nprof.layName = std::string("default");
	nprof.zx = zxCreate();
	setHardware(nprof.zx,"ZX48K");
	if (currentProfile != NULL) {
		nm = currentProfile->name;
		profileList.push_back(nprof);		// PUSH_BACK reallocate profileList and breaks current profile pointer!
		setProfile(nm);				// then it must be setted again
	} else {
		profileList.push_back(nprof);
	}
	return true;
}

int delProfile(std::string nm) {
	if (nm == "default") return DELP_ERR;			// can't touch this
	int idx = findProfile(nm);
	if (idx < 0) return DELP_ERR;				// no such profile
	int res = DELP_OK;
	zxDestroy(profileList[idx].zx);
	if (currentProfile->name == nm) {
		setProfile("default");	// if current profile deleted, set default
		res = DELP_OK_CURR;
	}
	if (currentProfile != NULL) {
		nm = currentProfile->name;
		profileList.erase(profileList.begin() + idx);
		setProfile(nm);
	} else {
		profileList.erase(profileList.begin() + idx);
	}
	return res;
}

bool setProfile(std::string nm) {
	int idx = findProfile(nm);
	if (idx < 0) return false;
	currentProfile = &profileList[idx];
	zx = currentProfile->zx;
	vidUpdate(zx->vid);
	return true;
}

void clearProfiles() {
	XProfile defprof = profileList[0];
	profileList.clear();
	profileList.push_back(defprof);
	setProfile(profileList[0].name);
}

std::vector<XProfile> getProfileList() {
	return profileList;
}

XProfile* getCurrentProfile() {
	return currentProfile;
}
