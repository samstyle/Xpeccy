#include "uart.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

UART* uart_create(int n, cbirq cb, void* ptr) {
	UART* uart = (UART*)malloc(sizeof(UART));
	if (uart) {
		memset(uart, 0, sizeof(UART));
		uart->irqn = n;
		uart->xirq = cb;
		uart->xptr = ptr;
		uart_set_rate(uart, 100);		// 100 bytes/sec
		uart_set_type(uart, UART_8250);
	}
	return uart;
}

void uart_destroy(UART* uart) {
	free(uart);
}

// set irq 'data ready' id
void uart_set_irq(UART* uart, int n) {
	uart->irqn = n;
}

// set callbacks for rd/wr from/to device and device ptr
void uart_set_dev(UART* uart, xurdcb cbr, xuwrcb cbw, void* p) {
	uart->devrd = cbr;
	uart->devwr = cbw;
	uart->devptr = p;
}

// call it when device is ready to transmit data
void uart_ready(UART* uart) {
	uart->ready = 1;
	uart->lsr = 0;
//	printf("uart ready to get data\n");
}

// clk: 1.8432MHz		542ns
// counter: each 16 ticks	8672ns
// 8bits			69376ns @ maximum speed, *uart->div

#define UART_MIN_BYTE_NS	69376

// set device rd rate, bytes/sec
void uart_set_rate(UART* uart, int bps) {
	uart->nsrate = 1e9/bps;
}

void uart_set_div(UART* uart, unsigned short div) {
	uart->div = div;
	uart->nsrate = UART_MIN_BYTE_NS * uart->div;
}

// wrapper to core

void uart_reset(UART *uart) {
	if (!uart->core) return;
	if (!uart->core->reset) return;
	uart->core->reset(uart);
}

void uart_sync(UART* uart, int ns) {
	if (!uart->core) return;
	if (!uart->core->sync) return;
	uart->core->sync(uart, ns);
}

int uart_rd(UART* uart, int adr) {
	if (!uart->core) return -1;
	if (!uart->core->rd) return -1;
	return uart->core->rd(uart, adr);
}

void uart_wr(UART* uart, int adr, int data) {
	if (!uart->core) return;
	if (!uart->core->wr) return;
	uart->core->wr(uart, adr, data);
}

// ns8250

void u8250_reset(UART* uart) {
	uart->ier = 0;
	uart->iir = 1;
	uart->lcr = 0;
	uart->mcr = 0;
	uart->lsr = 0x60;
	uart->msr = 0;
}

void u8250_sync(UART* uart, int ns) {
	if (uart->ready) {
		uart->nscnt -= ns;
		if (uart->nscnt < 0) {
			uart->nscnt += uart->nsrate;
			uart->datar = uart->devrd ? uart->devrd(uart->xptr) : -1;
//			printf("uart get byte from device: %X\n", uart->datar);
			if (uart->datar < 0) {		// no data from device
				uart->ready = 0;
				uart->nscnt = 0;
//				printf("transmit end\n");
			} else if (uart->drqr) {	// previous data didn't readed
				// data lost
				uart->lsr |= 2;
				uart->iir = 0x60;	// error
				if (uart->ier & 1) {
					uart->xirq(uart->irqn, uart->xptr);
				}
//				printf("data lost\n");
			} else {			// set new data
				uart->drqr = 1;
				uart->iir = 0x40;	// recieved data available
				if (uart->ier & 1) {
					uart->xirq(uart->irqn, uart->xptr);
				}
//				printf("uart sends INTR\n");
			}
		}
	}
}

int u8250_rd(UART* uart, int port) {
	int res = -1;
	switch (port & 7) {
		case 0:
			if (uart->lcr & 0x80) {
				res = uart->div & 0xff;
			} else if (uart->drqr) {
				res = uart->datar & 0xff;
				uart->drqr = 0;
				uart->iir = 1;
			}
			break;
		case 1: if (uart->lcr & 0x80) {
				res = (uart->div >> 8) & 0xff;
			} else  {
				res = uart->ier & 0xff;
			}
			break;
		case 2: res = uart->iir & 0x07;			// b0:0 on interrupt; b1,2:interrupt type; b3..7 = 0
			break;
		case 3: res = uart->lcr & 0xff; break;
		case 4: res = uart->mcr & 0xff; break;
		case 5: res = uart->lsr & ~0x21;		// b0: drqr, b1:overrun err, b2:parity err, b3:framing err (stop bit err), b4:data recieve err, b5:drqw, b6:thr & tsr is empty, b7=0
			if (uart->drqr) res |= 0x01;
			if (uart->drqw) res |= 0x20;
			break;
		case 6: res = uart->msr & 0xff;
			if (uart->iir == 0x00) uart->iir = 1;	// modem status interrupt
			break;
		case 7: res = uart->scr & 0xff; break;

	}
//	printf("com read %i = %.2X\n", port & 7, res);
	return res;
}

void u8250_wr(UART* uart, int port, int data) {
//	printf("com write %i, %.2X\n", port & 7, data);
	switch (port & 7) {
		case 0:
			if (uart->lcr & 0x80) {
				uart_set_div(uart, (uart->div & 0xff00) | (data & 0xff));
			} else if (uart->drqw) {
				uart->iir = 1;
				if (uart->devwr) uart->devwr(data, uart->devptr);
			}
			break;
		case 1:
			if (uart->lcr & 0x80) {
				uart_set_div(uart, ((data << 8) & 0xff00) | (uart->div & 0xff));
			} else {
				uart->ier = data;
			}
			break;
		case 3: uart->lcr = data; break;
		case 4: uart->mcr = data; break;
		case 5: uart->lsr = data; break;
		case 6: uart->msr = data; break;
		case 7: uart->scr = data; break;
	}
}

// upd8251 uart
// msr.b0 = wr.mode (0-mode, 1-command)
// mcr = mode byte
// lsr = status register (errors)
// lcr = command byte
// datar = data from device (set in sync if device is ready)
// drqr = data getted from device and writed to datar
#define F_ERR_FE	0x20
#define F_ERR_OE	0x10
#define F_ERR_PE	0x08
#define F_RXRDY		0x02

void u8251_reset(UART* uart) {
	uart->msr = 0;
	uart->lsr = 0;
	uart->lcr = 0;
	uart->mcr = 0;
}

int u8251_rd(UART* uart, int adr) {
	int res = -1;
	if (adr & 1) {
		res = uart->lsr & (F_ERR_FE | F_ERR_OE | F_ERR_PE);
		if (uart->drqr) res |= F_RXRDY;
	} else if (uart->drqr) {
		res = uart->datar;
		uart->drqr = 0;
	}
	return res;
}

void u8251_wr(UART* uart, int adr, int data) {
	if (adr & 1) {
		if (uart->msr & 1) {	// command
			uart->lcr = data;
			if (data & 0x40) uart->msr &= ~1;
			// TODO: kbd.lock = !!(data & 0x20)
			if (data & 0x10) uart->lsr &= ~(F_ERR_FE | F_ERR_OE | F_ERR_PE);
		} else {		// mode
			uart->mcr = data;
			uart->msr |= 1;
		}
	} else {
		// write to dev? or don't...
		// uart->devwr(data, uart->xptr);
	}
}

void u8251_sync(UART* uart, int ns) {
	if (uart->ready) {
		uart->nscnt -= ns;
		if (uart->nscnt < 0) {
			uart->nscnt += uart->nsrate;
			uart->datar = uart->devrd ? uart->devrd(uart->xptr) : -1;
			if (uart->datar < 0) {		// no data from device
				uart->ready = 0;
				uart->nscnt = 0;
			} else if (uart->drqr) {	// previous data didn't readed, overflow error
				uart->lsr |= F_ERR_OE;
			} else {			// set new data
				uart->drqr = 1;
			}
		}
	}
}

// core stuff

const UARTCore uart_core_tab[] = {
	{UART_8250, u8250_reset, u8250_rd, u8250_wr, u8250_sync},
	{UPD_8251, u8251_reset, u8251_rd, u8251_wr, u8251_sync},
	{-1, NULL, NULL, NULL, NULL}
};

void uart_set_type(UART* uart, int id) {
	int i = 0;
	while ((uart_core_tab[i].id != -1) && (uart_core_tab[i].id != id)) {
		i++;
	}
	if (uart_core_tab[i].id != -1) {
		uart->core = &uart_core_tab[i];
	}
}
