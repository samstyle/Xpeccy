#pragma once

#include "defines.h"

enum {
	UART_8250 = 1,
	UPD_8251,
};

typedef int(*xurdcb)(void*);
typedef void(*xuwrcb)(int, void*);

typedef struct UART UART;

typedef struct {
	int id;
	void(*reset)(UART*);
	int(*rd)(UART*,int);
	void(*wr)(UART*,int,int);
	void(*sync)(UART*,int);
} UARTCore;

struct UART {
	unsigned drqr:1;	// byte from device is ready to be read
	unsigned drqw:1;	// need byte to write to device
	unsigned ready:1;	// device is ready to send byte (set on device signal, reset when all data is readed)

	int nsrate;	// data rate for device (ns for 1 byte)
	int nscnt;
	xurdcb devrd;	// read from device callback
	xuwrcb devwr;	// write to device callback
	void* devptr;	// device pointer

	int irqn;	// int id
	cbirq xirq;	// INTR callback
	void* xptr;

	const UARTCore* core;	// type depended callbacks

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
};

UART* uart_create(int, cbirq, void*);
void uart_destroy(UART*);
void uart_set_irq(UART*, int);
void uart_set_dev(UART*, xurdcb, xuwrcb, void*);
void uart_ready(UART*);

void uart_set_type(UART*, int);
void uart_set_rate(UART*, int);
void uart_sync(UART*, int);
int uart_rd(UART*, int);
void uart_wr(UART*, int, int);
