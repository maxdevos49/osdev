#ifndef __SERIAL_H
#define __SERIAL_H 1

void init_serial(void);
char serial_read(void);
void serial_write(char byte);

#endif
