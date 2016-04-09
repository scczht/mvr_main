#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "hardware_iic.h"
#include "mpu6050.h"
#include "hmc5883.h"
#include "bh1750.h"
#include "ds18b20.h"
#include "adc.h"
#include "ir_bizhang.h"
#include "pwm.h"
#include "socket.h"
#include "serial.h"

struct eeprom e;
struct eeprom fd_hmc;
int fd_bh1750;
int fd_adc;
int fd_pwm;
int serial_fd;
//屏蔽信号
void mask_signal()
{
	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs,SIGPIPE);
	sigprocmask(SIG_BLOCK,&sigs,0);
	//signal(SIGPIPE, SIG_IGN);
	//signal(SIGPIPE, SIG_IGN);
	//signal(SIGINT, SIG_IGN);
}
void fun_sig()
{
	printf("out\n");
	usleep(1);
}
int main(int argc, char** argv)
{
	mask_signal();
	//signal(SIGPIPE, fun_sig);
	init_mpu6050(&e);
	init_hmc5883(&fd_hmc);
	init_bh1750(&fd_bh1750);
	init_adc(&fd_adc);
	init_pwm(&fd_pwm);
	init_serial(&serial_fd,9600);
	
	main_thread();
	/*//
	while(1)
	{
		get_imu_ypr_q4();
		usleep(30000);
	}
	//*/
	eeprom_close(&e);
	eeprom_close(&fd_hmc);
	close_pwm(&fd_pwm);
	close(fd_adc);
	close(fd_bh1750);
	return 0;
}

