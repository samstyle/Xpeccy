#ifndef _DEVBUS_H
#define _DEVBUS_H

// universal bus to device

typedef struct {
	unsigned busy:1;	// bus is busy (not CPU signal): to avoid devices conflict
	unsigned iorq:1;	// io request
	unsigned memrq:1;	// mem request
	unsigned rd:1;		// 1:read (device will return data in the 'data' field), 0:write
	unsigned dos:1;
	unsigned short adr;	// address bus (16 bit)
	unsigned char data;	// data bus (8 bit)
} xDevBus;

#endif
