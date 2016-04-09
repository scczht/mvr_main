#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "hardware_iic.h"
#include <getopt.h>
#include <sys/stat.h>
#include <math.h>
#define die_if(a, msg) do { do_die_if( a , msg, __LINE__); } while(0);
unsigned char  BUF[8]={0};
short x=0;
short y=0;
short z=0;
float angle=0.0;

void init_hmc5883(struct eeprom *fd)
{
	die_if(eeprom_open("/dev/i2c-2", 0x1e, EEPROM_TYPE_8BIT_ADDR, fd) < 0,
			"unable to open HMC5883 device file ");	
	unsigned char buf[2]={0x20,0x00};
	i2c_write(fd,0x01, 2, buf);	
}
float multiple_read_HMC5883(struct eeprom *fd)
{  
	i2c_read(fd, 0x03,6, BUF);
	 x=BUF[0] << 8 | BUF[1]; 
    //z=BUF[2] << 8 | BUF[3]; 
    y=BUF[4] << 8 | BUF[5]; 
	//angle=atan2((float)y,(float)x) * (180 / 3.14159265)+180; // angle in degrees
	angle=atan2(((double)y-50),((double)x-27.8)) * (180 / 3.14159265)+180; // angle in degrees
	return angle;
}
void close_hmc(struct eeprom *fd)
{
	eeprom_close(fd);
}

