#ifndef _SERIAL_H
#define _SERIAL_H

void set_speed(int fd, int speed);
int set_Parity(int fd, int databits, int stopbits, int parity);
int OpenDev(char *Dev);
void init_serial(int *s_fd,int speed);
#endif
