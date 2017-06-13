#include <stdio.h>

#include "gamepad.h"

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
			if (buf[0] == 'A') jent.type = JOY_AXIS;
			else if (buf[0] == 'B') jent.type = JOY_BUTTON;
			else if (buf[0] == 'H') jent.type = JOY_HAT;
			else jent.type = JOY_NONE;
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
					switch(buf[idx]) {
						case 'U': jent.state = SDL_HAT_UP; break;
						case 'D': jent.state = SDL_HAT_DOWN; break;
						case 'L': jent.state = SDL_HAT_LEFT; break;
						case 'R': jent.state = SDL_HAT_RIGHT; break;
					}
					idx++;
					break;
			}
			if (buf[idx] == ':') {
				idx++;
				if (buf[idx] == 'K') jent.dev = JMAP_KEY;
				else if (buf[idx] == 'J') jent.dev = JMAP_JOY;
				else jent.dev = JMAP_NONE;
				idx++;
				switch(jent.dev) {
					case JMAP_KEY:
						jent.key = getKeyIdByName(&buf[idx]);
						if ((jent.type != JOY_NONE) && (jent.key != ENDKEY))
							conf.joy.map.push_back(jent);
						break;
					case JMAP_JOY:
						switch(buf[idx]) {
							case 'R': jent.dir = XJ_RIGHT; break;
							case 'L': jent.dir = XJ_LEFT; break;
							case 'U': jent.dir = XJ_UP; break;
							case 'D': jent.dir = XJ_DOWN; break;
							case 'F': jent.dir = XJ_FIRE; break;
							default: jent.dir = XJ_NONE; break;
						}
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
			if (jent.type == JOY_AXIS) fputc('A', file);
			else if (jent.type == JOY_HAT) fputc('H', file);
			else fputc('B', file);
			fprintf(file, "%i", jent.num);
			switch (jent.type) {
				case JOY_AXIS:
					fputc((jent.state < 0) ? '-' : '+', file);
					break;
				case JOY_HAT:
					switch(jent.state) {
						case SDL_HAT_UP: fputc('U', file); break;
						case SDL_HAT_DOWN: fputc('D', file); break;
						case SDL_HAT_LEFT: fputc('L', file); break;
						case SDL_HAT_RIGHT: fputc('R', file); break;
					}
					break;
			}
			fputc(':', file);
			switch(jent.dev) {
				case JMAP_KEY:
					fprintf(file, "K%s", getKeyNameById(jent.key));
					break;
				case JMAP_JOY:
					fputc('J', file);
					switch(jent.dir) {
						case XJ_RIGHT: fputc('R', file); break;
						case XJ_LEFT: fputc('L', file); break;
						case XJ_UP: fputc('U', file); break;
						case XJ_DOWN: fputc('D', file); break;
						case XJ_FIRE: fputc('F', file); break;
						default: fputc('?', file); break;
					}
					break;
				default:
					fprintf(file, "??");
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
