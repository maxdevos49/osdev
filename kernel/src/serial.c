#include <stdbool.h>
#include <io.h>

#define COM1 0x3F8

void init_serial(void)
{
	outb(COM1 + 1, 0x00); // Disable all interrupts
	outb(COM1 + 3, 0x80); // Enable DLAB (set baud rate divisor)
	outb(COM1 + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
	outb(COM1 + 1, 0x00); //                  (hi byte)
	outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
	outb(COM1 + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
	outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
	outb(COM1 + 4, 0x1E); // Set in loopback mode, test the serial chip
	outb(COM1 + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)

	// Check if serial is faulty (i.e: not same byte as sent)
	if (inb(COM1 + 0) != 0xAE)
	{
		return;
	}

	// If serial is not faulty set it in normal operation mode
	// (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
	outb(COM1 + 4, 0x0F);
	return;
}

static bool serial_received(void)
{
	return inb(COM1 + 5) & 1;
}

static bool serial_transmit_complete(void)
{
	return inb(COM1 + 5) & 0x20;
}

char serial_read(void)
{
	while (serial_received() == false)
		;

	return inb(COM1);
}

void serial_write(char byte)
{
	while (serial_transmit_complete() == 0)
		;

	outb(COM1, byte);
}
