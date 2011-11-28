#include <fstream>
#include <stdint.h>

uint16_t getLEWord(std::ifstream* file) {
	uint16_t res = file->get();
	res += (file->get() << 8);
	return res;
}

uint16_t getBEWord(std::ifstream* file) {
	uint16_t res = file->get();
	res = (res << 8) + file->get();
	return res;
}
