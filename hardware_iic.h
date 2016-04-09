/***************************************************************************
    copyright            : (C) by 2003-2004 Stefano Barbato
    email                : stefano@codesink.org

    $Id: 24cXX.h,v 1.6 2004/02/29 11:05:28 tat Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _24CXX_H_
#define _24CXX_H_
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#define EEPROM_TYPE_UNKNOWN	0
#define EEPROM_TYPE_8BIT_ADDR	1
#define EEPROM_TYPE_16BIT_ADDR 	2

struct eeprom
{
	char *dev; 	// device file i.e. /dev/i2c-N
	int addr;	// i2c address
	int fd;		// file descriptor
	int type; 	// eeprom type
};

/*
 * opens the eeprom device at [dev_fqn] (i.e. /dev/i2c-N) whose address is
 * [addr] and set the eeprom_24c32 [e]
 */
int eeprom_open(char *dev_fqn, int addr, int type, struct eeprom*);
/*
 * closees the eeprom device [e] 
 */
int eeprom_close(struct eeprom *e);
/*
 * read and returns the eeprom byte at memory address [mem_addr] 
 * Note: eeprom must have been selected by ioctl(fd,I2C_SLAVE,address)  
 */
int eeprom_read_byte(struct eeprom* e, __u16 mem_addr);
/*
 * read the current byte
 * Note: eeprom must have been selected by ioctl(fd,I2C_SLAVE,address) 
 */
int eeprom_read_current_byte(struct eeprom *e);
/*
 * writes [data] at memory address [mem_addr] 
 * Note: eeprom must have been selected by ioctl(fd,I2C_SLAVE,address) 
 */
int eeprom_write_byte(struct eeprom *e, __u16 mem_addr, __u8 data);
int my_i2c_write_1b(struct eeprom *e, __u8 buf);
__u8 my_read_byte(struct eeprom* e, __u8 mem_addr);
int my_i2c_write_2b(struct eeprom *e, __u8 buf[2]);
int i2c_write(struct eeprom *e,__u8 reg, __u8 len, __u8* data);
int i2c_read(struct eeprom *e, __u8 reg, __u8 len, __u8 *buf);
void do_die_if(int b, char* msg, int line);
#endif
