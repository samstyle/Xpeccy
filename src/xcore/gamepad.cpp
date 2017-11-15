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
	std::string path;
	FILE* file;
	int num;
	int idx;
	char buf[1024];

	path = conf.path.confDir + SLASH + name;
	file = fopen(path.c_str(), "rb");
	if (file) {
		conf.joy.map.clear();
		while(!feof(file)) {
			memset(buf, 0x00, 1024);
			fgets(buf, 1023, file);
			strtok(buf, "\n\a");
			jent.type = padGetId(buf[0], pabhChars);
			num = 0;
			idx = 1;
			while ((buf[idx] >= '0') && (buf[idx] <= '9')) {
				num = num * 10 + buf[idx] - '0';
				idx++;
			}
			jent.num = num;

			switch(jent.type) {
				case JOY_AXIS:
					jent.state = (buf[idx] == '-') ? -1 : +1;
					idx++;
					break;
				case JOY_BUTTON:
					break;
				case JOY_HAT:
					jent.state = padGetId(buf[idx], hatChars);
					idx++;
					break;
			}
			if (buf[idx] == ':') {
				idx++;
				jent.dev = padGetId(buf[idx], devChars);
				idx++;
				switch(jent.dev) {
					case JMAP_KEY:
						jent.key = getKeyIdByName(&buf[idx]);
						if ((jent.type != JOY_NONE) && (jent.key != ENDKEY))
							conf.joy.map.push_back(jent);
						break;
					case JMAP_JOY:
						jent.dir = padGetId(buf[idx], kjoyChars);
						conf.joy.map.push_back(jent);
						break;
					case JMAP_MOUSE:
						jent.dir = padGetId(buf[idx], kmouChars);
						conf.joy.map.push_back(jent);
						break;
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
