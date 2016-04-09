#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PWM_IOCTL_SET_FREQ		1
#define PWM_IOCTL_STOP			0

void close_pwm(int *fd);
void init_pwm(int *fd)
{
	*fd = open("/dev/pwm_test", 0);
	if (*fd < 0) {
		perror("open pwm_pwm device");
		exit(1);
	}
}

void close_pwm(int *fd)
{
	if (*fd >= 0) {
		ioctl(*fd, PWM_IOCTL_STOP);
		if (ioctl(*fd, 2) < 0) {
			perror("ioctl 2:");
		}
		close(*fd);
		*fd = -1;
	}
}

void set_pwm_duty(int *fd,int div)
{
	// this IOCTL command is the key to set frequency
	int ret = ioctl(*fd, PWM_IOCTL_SET_FREQ, div);
	if(ret < 0) {
		perror("set the frequency of the pwm");
		exit(1);
	}
}


void stop_pwm(int *fd)
{
	int ret = ioctl(*fd, PWM_IOCTL_STOP);
	if(ret < 0) {
		perror("stop the pwm");
		exit(1);
	}
	if (ioctl(*fd, 2) < 0) {
		perror("ioctl 2:");
	}
}

void read_current_attitude(int *fd,int *buf)
{
	read(*fd,buf,sizeof(int));
}

int cruise(int *fd,int current_duty,char *r_flag,char *thread_flag)
{
	int flag=0;

	if(0==current_duty)
	{
		flag=0;
	}
	else if(360==current_duty)
	{
		flag=1;
	}
	else
	{
		flag=2;
	}
	while(*thread_flag)
	{
		switch(flag)
		{
			case 0:
				current_duty++;
				break;
			case 1:
				current_duty--;
				break;
			case 2:
				current_duty--;
				break;
		}
		set_pwm_duty(fd,current_duty);
		usleep(30000);
		stop_pwm(fd);
		if(current_duty==360)
			flag=1;
		else if(current_duty==1)
			flag=0;
		if('r'!=*r_flag)
			break;
	}  
	stop_pwm(fd);
}