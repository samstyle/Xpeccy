#pragma once

enum {
	PPI_OFF = 0,
	PPI_IN,
	PPI_OUT
};

typedef int(*cbppird)(void*);
typedef void(*cbppiwr)(int, void*);

typedef struct {
	int dir;
	int mode;
	int val;
	cbppird rd;
	cbppiwr wr;
} ppiChan;

typedef struct {
	ppiChan a;
	ppiChan b;
	ppiChan ch;
	ppiChan cl;
	int ctrl;
	void* ptr;
}  PPI;

PPI* ppi_create();
void ppi_destroy(PPI*);

void ppi_reset(PPI*);
int ppi_rd(PPI*, int adr);
void ppi_wr(PPI*, int adr, int val);
