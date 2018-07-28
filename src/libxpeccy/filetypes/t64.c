#include "filetypes.h"

#pragma pack (push, 1)

typedef struct {
	char type_c64;
	char type_1541;
	unsigned short start;
	unsigned short end;
	unsigned short nu01;
	unsigned short offset;
	unsigned short nu02;
	char name[16];
} t64file;

#pragma pack (pop)

int loadT64(Computer* comp, const char* fname) {
	char buf[32];
	int err = ERR_OK;
	unsigned short ver;
	unsigned short maxent;
	unsigned short totent;
	unsigned short start = 0;
	int i;
	t64file desc;
	long offset;
	FILE* file = fopen(fname, "rb");
	if (!file) {
		err = ERR_CANT_OPEN;
	} else {
		fread(buf, 32, 1, file);
		if (memcmp(buf, "C64", 3)) {
			err = ERR_T64_SIGN;
		} else {
			ver = fgetw(file);
			maxent = fgetw(file);
			totent = fgetw(file);
			fgetw(file);			// not used
			fread(buf, 24, 1, file);	// container name
			buf[24] = 0x00;
			printf("ver: %.4X\n",ver);
			printf("max ent: %i\n",maxent);
			printf("tot ent: %i\n",totent);
			printf("container: %s\n",buf);
			for(i = 0; i < totent; i++) {
				fread((char*)(&desc), sizeof(t64file), 1, file);
				offset = ftell(file);
				fseek(file, desc.offset, SEEK_SET);
				fread(comp->mem->ramData + desc.start, desc.end - desc.start + 1, 1, file);
				fseek(file, offset, SEEK_SET);
				if (i == 0)
					start = desc.start;
			}
		}
//		if (start)
//			comp->cpu->pc = start;
		fclose(file);
	}
	return err;
}
