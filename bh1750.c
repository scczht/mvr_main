/*
 关于该传感器详情请参考：
 http://wenku.baidu.com/link?url=CqbFokA6ICqxuXnKL54UTgA07odnvFf3gL2-G3jeyaS0sEsbjiEU_LqR_OYAzipBPqxU4DIYNFjXPvlIW4p5qpso_JXLqE4UPNb--3WlBEG
*/

#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include "hardware_iic.h"
//BH1750 IIC 光照传感器 器件地址
#define I2C_ADDR 0x23
char buf[3];
float flight;
//初始化光照传感器
//打开设备描述符
//向传感器写入0x01 0x10
int init_bh1750(int *fd)
{
	*fd=open("/dev/i2c-2",O_RDWR);
    if(*fd<0)
    {
        printf("err open file: error\r\n"); return 1;
    }
	char val;
	if(ioctl(*fd,I2C_SLAVE,I2C_ADDR)<0 )
    {
        printf("ioctl error : error\n");
		return -1;
    }
    val=0x01;
    if(write(*fd,&val,1)<0)
    {
        printf("write 0x01 err\r\n");
    }
    val=0x10;
    if(write(*fd,&val,1)<0)
    {
        printf("write 0x10 err\r\n");
    }
	return 0;
}
void close_bh1750(fd)
{
	close(fd);
}

//该函数一次读取了，3个字节
//只使用前两字节，作为高低8位
//最后相当于读取了应答位
float getdata_bh1750(int *fd)
{
	if(read(*fd,&buf,3))
    {
        flight=(buf[0]*256+buf[1])/1.2;
        //printf("light is %6.3f\r\n",flight);
		return flight;
    }
    else
    {
       // printf("read light error\r\n");
		return -1;
    }

}