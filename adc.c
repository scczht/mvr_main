#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <errno.h>
#include <string.h>

#define ADC_SET_CHANNEL         0xc000fa01
#define ADC_SET_ADCTSC          0xc000fa02
#define CHANNELCOUNT 1          //可以6个通道,本程序只用0通道
int channels[CHANNELCOUNT] = {0};
int gas_data;

int init_adc(int *fd)
{
	*fd = open("/dev/adc", 0);
	if (*fd < 0) {
		perror("open ADC device:");
		return -1;
	}
}

int get_adc(int *fd)
{
	int i;
	char buffer[30];
    for (i=0; i<CHANNELCOUNT; i++) 
	{
        //channel = channels[i];
        if (ioctl(*fd, ADC_SET_CHANNEL, channels[i]) < 0) 
		{
            perror("Can't set channel for /dev/adc!");
            close(*fd);
            return -1;
        } 
        int len = read(*fd, buffer, sizeof buffer -1);
        if (len > 0) 
		{
            buffer[len] = '\0';
			gas_data=atoi(buffer);
			return gas_data;
        } 
		else 
		{
            perror("read ADC device:");
            close(*fd);
            return -1;
        } 	
	}
}
