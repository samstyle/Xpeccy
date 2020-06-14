#include <stdio.h>

#include "gamepad.h"

typedef struct {
	char ch;
	int val;
} xCharDir;

const xCharDir kjoyChars[] = {
	{'U', XJ_UP},
	{'D', XJ_DOWN},
	{'L', XJ_LEFT},
	{'R', XJ_RIGHT},
	{'F', XJ_FIRE},
	{'2', XJ_BUT2},
	{'3', XJ_BUT3},
	{'4', XJ_BUT4},
	{'-', XJ_NONE}
};

const xCharDir kmouChars[] = {
	{'U', XM_UP},
	{'D', XM_DOWN},
	{'L', XM_LEFT},
	{'R', XM_RIGHT},
	{'[', XM_LMB},
	{'|', XM_MMB},
	{']', XM_RMB},
	{'^', XM_WHEELUP},
	{'v', XM_WHEELDN},
	{'-', XM_NONE}
};

const xCharDir hatChars[] = {
	{'U', SDL_HAT_UP},
	{'D', SDL_HAT_DOWN},
	{'L', SDL_HAT_LEFT},
	{'R', SDL_HAT_RIGHT},
	{'-', 0}
};

const xCharDir pabhChars[] = {
	{'A', JOY_AXIS},
	{'B', JOY_BUTTON},
	{'H', JOY_HAT},
	{'-', JOY_NONE}
};

const xCharDir devChars[] = {
	{'K', JMAP_KEY},
	{'J', JMAP_JOY},
	{'M', JMAP_MOUSE},
	{'-', JMAP_NONE}
};

char padGetChar(int val, const xCharDir* tab) {
	int idx = 0;
	while ((tab[idx].val > 0) && (tab[idx].val != val))
		idx++;
	return tab[idx].ch;
}

int padGetId(char ch, const xCharDir* tab) {
	int idx = 0;
	while ((tab[idx].val > 0) && (tab[idx].ch != ch))
		idx++;
	return tab[idx].val;
}

void padLoadConfig(std::string name) {
	if (name.empty()) return;
	xJoyMapEntry jent;
	// char path[FILENAME_MAX];
	FILE* file;
	int num;
	int idx;
	char buf[1024];
	char* ptr;

	std::string path = conf.path.confDir + SLASH + name;
	file = fopen(path.c_str(), "rb");
	if (file) {
		conf.joy.map.clear();
		while(!feof(file)) {
			memset(buf, 0x00, 1024);
			fgets(buf, 1023, file);
			ptr = strtok(buf, ":\n");
			if (ptr) {
				jent.type = padGetId(ptr[0], pabhChars);
				jent.num = atoi(&ptr[1]);
				idx = 1;
				num = 0;
				while ((ptr[idx] >= '0') && (ptr[idx] <= '9')) {
					num = num * 10 + ptr[idx] - '0';
					idx++;
				}
				jent.num = num;
				// get direction for axis & hat
				switch(jent.type) {
					case JOY_AXIS:		// A0+ A0-
						jent.state = (ptr[idx] == '-') ? -1 : +1;
						idx++;
						break;
					case JOY_BUTTON:
						break;
					case JOY_HAT:		// HU HD HR HL
						jent.state = padGetId(ptr[idx], hatChars);
						idx++;
						break;
				}
				ptr = strtok(NULL, ":\n");
				if (ptr) {		// there was 1st :
					jent.dev = padGetId(ptr[0], devChars);
					idx = 1;
					switch (jent.dev) {
						case JMAP_KEY:		// KUP, KLEFT, KQ, KA
							jent.key = getKeyIdByName(&ptr[1]);
							if (jent.key == ENDKEY)
								jent.dev = JMAP_NONE;
							break;
						case JMAP_JOY:		// JU, JD, JF, J2, J4
							jent.dir = padGetId(ptr[1], kjoyChars);
							break;
						case JMAP_MOUSE:	// MD, ML, M[ M| M] M^ Mv
							jent.dir = padGetId(ptr[1], kmouChars);
							break;
						default:
							jent.dev = JMAP_NONE;	// ignore it
							break;
					}
					jent.rpt = 0;
					jent.cnt = 0;
					ptr = strtok(NULL, ":\n");
					if (ptr)
						jent.rpt = atoi(ptr);
					if (jent.dev != JMAP_NONE)
						conf.joy.map.push_back(jent);
				}
			}
		}
		fclose(file);
	}
}

void padSaveConfig(std::string name) {
	if (name.size() == 0) return;
	std::string path = conf.path.confDir + SLASH + name;
	FILE* file;
	file = fopen(path.c_str(), "wb");
	if (file) {
		foreach(xJoyMapEntry jent, conf.joy.map) {
			fprintf(file, "%c%i", padGetChar(jent.type, pabhChars), jent.num);
			switch (jent.type) {
				case JOY_AXIS:
					fputc((jent.state < 0) ? '-' : '+', file);
					break;
				case JOY_HAT:
					fputc(padGetChar(jent.state, hatChars), file);
					break;
			}
			fprintf(file, ":%c", padGetChar(jent.dev, devChars));
			switch(jent.dev) {
				case JMAP_KEY:
					fprintf(file, "%s", getKeyNameById(jent.key));
					break;
				case JMAP_JOY:
					fputc(padGetChar(jent.dir, kjoyChars), file);
					break;
				case JMAP_MOUSE:
					fputc(padGetChar(jent.dir, kmouChars), file);
					break;
				default:
					fprintf(file, "?");
			}
			if (jent.rpt > 0)
				fprintf(file, ":%i", jent.rpt);
			fputc('\n', file);
		}
		fclose(file);
	}
}

int padExists(std::string name) {
	std::string path = conf.path.confDir + SLASH + name;
	FILE* file = fopen(path.c_str(), "rb");
	if (!file) return 0;
	fclose(file);
	return 1;
}

int padCreate(std::string name) {
	if (padExists(name)) return 0;
	std::string path = conf.path.confDir + SLASH + name;
	FILE* file = fopen(path.c_str(), "wb");
	if (!file) return 0;
	fclose(file);
	return 1;
}

void padDelete(std::string name) {
	std::string path = conf.path.confDir + SLASH + name;
	remove(path.c_str());
}
