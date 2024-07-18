#pragma once

#include "defines.h"

typedef int(*xurdcb)(void*);
typedef void(*xuwrcb)(unsigned char, void*);

typedef struct {
	unsigned drqr:1;	// byte from device is ready to be read
	unsigned drqw:1;	// need byte to write to device
	unsigned ready:1;	// device is ready to send byte (set on device signal, reset when all data is readed)

	int nsrate;	// data rate for device (ns for 1 byte)
	int nscnt;
	xurdcb devrd;	// read from device callback
	xuwrcb devwr;	// write to device callback
	void* devptr;	// device pointer

	int datar;	// dev->cpu
	int dataw;	// cpu->dev
	int ier;	// int.enable register
	int iir;	// int.ident. register
	int lcr;	// line control register
	int mcr;
	int lsr;
	int msr;
	int scr;
	int div;	// div * clock / 16: freq.of bits transmitting

	int irqn;	// int id
	cbirq xirq;	// INTR callback
	void* xptr;
} UART;

UART* uart_create(int, cbirq, void*);
void uart_destroy(UART*);
void uart_set_dev(UART*, xurdcb, xuwrcb, void*);
void uart_ready(UART*);

void uart_sync(UART*, int);
int uart_rd(UART*, int);
void uart_wr(UART*, int, int);
