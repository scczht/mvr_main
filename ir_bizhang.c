#include "socket.h"
#include <linux/input.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
//测试文件是否存在
int isExist(char *filename)
{
  return (access(filename, 0) == 0); 
}
int get_gpio_156_ir_bizhang()											//右侧
{
   unsigned int value;
   FILE *pRreco = NULL;
   if(!isExist("/sys/class/gpio/gpio156"))   
   {
	unsigned int value;
	FILE *pRreco = NULL;
	if((pRreco = fopen("/sys/class/gpio/export", "w")) == NULL)
	{
	    printf("Open read gpio/export\n");
		return -1;
	}
   if(fwrite("156", strlen("156"), 1, pRreco) != 1)
    printf("File Write Error!\n");
	fclose(pRreco);  
  } 
	if((pRreco = fopen("/sys/class/gpio/gpio156/value","r")) == NULL)
	{
	    printf("Open read recovery button failure\n");
		return -1;
	}
	fscanf(pRreco,"%d",&value);
	fclose(pRreco);
	if(value)
		return 1;
	else
		return 0;
}
int get_gpio_149_ir_bizhang()											//右侧
{
    unsigned int value;
   FILE *pRreco = NULL;
   if(!isExist("/sys/class/gpio/gpio148"))   
   {
	unsigned int value;
	FILE *pRreco = NULL;
	if((pRreco = fopen("/sys/class/gpio/export", "w")) == NULL)
	{
	    printf("Open read gpio/export\n");
		return -1;
	}
   if(fwrite("149", strlen("149"), 1, pRreco) != 1)
    printf("File Write Error!\n");
	fclose(pRreco);  
  } 
	if((pRreco = fopen("/sys/class/gpio/gpio149/value","r")) == NULL)
	{
	    printf("Open read recovery button failure\n");
		return -1;
	}
	fscanf(pRreco,"%d",&value);
	fclose(pRreco);
	if(value)
		return 1;
	else
		return 0;
}
int control_motor_bizhang(int serial_fd, char position)
{
	switch(position)        //85,213},{42,169},{169,85},{42,213},{64,192}
	{
		case 'q': 						//前进
			write(serial_fd,&motor[1].qian[0],1);
			write(serial_fd,&motor[1].qian[1],1);
			break;
		case 'h':						//后退
			write(serial_fd,&motor[1].hou[0],1);
			write(serial_fd,&motor[1].hou[1],1);
			break;
		case 'z':						//左转
			write(serial_fd,&motor[1].zuo[0],1);
			write(serial_fd,&motor[1].zuo[1],1);
			break;
		case 'y':						//右转
			write(serial_fd,&motor[1].you[0],1);
			write(serial_fd,&motor[1].you[1],1);
			break;
		case 't':						//停止
			write(serial_fd,&motor[1].ting[0],1);
			write(serial_fd,&motor[1].ting[1],1);
			break;
	}
	return 1;
}
/***************************************************************
	检测小车当前障碍物的，状态
	左障碍
	右障碍
	两侧障碍
***************************************************************/
char check_bizhang_state()
{
	if((0==get_gpio_149_ir_bizhang()) && (0==get_gpio_156_ir_bizhang()))  //右左全有障碍
		return 'd';
	if((0==get_gpio_149_ir_bizhang()) && (1==get_gpio_156_ir_bizhang()))  //右侧障碍
		return 'y';
	if((1==get_gpio_149_ir_bizhang()) && (0==get_gpio_156_ir_bizhang()))  //左侧障碍	
		return 'z';
	if((1==get_gpio_149_ir_bizhang()) && (1==get_gpio_156_ir_bizhang()))  //无障障碍	
		return 'n';
}
/****************************************************************
	执行上一步逻辑后如果任然没有摆脱障碍
	就采用后退1s的策略。
	退一步海阔天空
	通用避障算法，只做此简单处理。
*****************************************************************/
int check_hou_1s(int serial_fd)
{
	if('n' != check_bizhang_state())
	{
		control_motor_bizhang(serial_fd, 'h');
		sleep(1);
		return 1;
	}
	return 0;
}
/****************************************************************
	执行左拐90度的动作，
	此处的角度值来自
	mpu6050航向角
*****************************************************************/
void zuo_90(int serial_fd,float *c_yaw)
{
	float tmp_yaw;
	float tmp=*c_yaw;
	tmp_yaw = tmp +90;
	float after;
	if(tmp_yaw>=360)
	{
		after=90-(360-tmp_yaw);
		control_motor_bizhang(serial_fd, 'z');
		while(*c_yaw>=tmp);
		while(*c_yaw>after);
		control_motor_bizhang(serial_fd, 't');
	}
	else if(tmp_yaw<360)
	{
		after=tmp_yaw-3;
		control_motor_bizhang(serial_fd, 'z');
		while(*c_yaw<after);
		control_motor_bizhang(serial_fd, 't');
	}
}
/****************************************************************
	执行右拐90度的动作，
	此处的角度值来自
	mpu6050航向角
*****************************************************************/
void you_90(int serial_fd,float *c_yaw)
{
	float tmp_yaw;
	float tmp=*c_yaw;
	tmp_yaw = tmp -90;
	float after;
	if(tmp_yaw>=0)
	{
		after=tmp_yaw+3;
		control_motor_bizhang(serial_fd, 'y');
		while(*c_yaw>after);
		control_motor_bizhang(serial_fd, 't');
	}
	else if(tmp_yaw<0)
	{
		after= 360-(90-tmp);
		control_motor_bizhang(serial_fd, 'y');
		while(*c_yaw<=tmp);
		while(*c_yaw>after);
	}
}
int common_bizhang(int serial_fd,float *current_yaw)
{
	int mid_test;
	if('d' == check_bizhang_state())
	{
		control_motor_bizhang(serial_fd, 'h');
		sleep(1);
		check_hou_1s(serial_fd);
		zuo_90(serial_fd,current_yaw);
		if(check_hou_1s(serial_fd))
		{
			control_motor_bizhang(serial_fd, 'h');  //左转有障碍
			sleep(1);
			you_90(serial_fd,current_yaw);
		}
		else
		{
			control_motor_bizhang(serial_fd, 'q');  //左转无障碍
			sleep(1);
			you_90(serial_fd,current_yaw);
		
		}
		return 1;
	}
	else if('y' == check_bizhang_state())
	{
		zuo_90(serial_fd,current_yaw);
		if(check_hou_1s(serial_fd))
		{
			control_motor_bizhang(serial_fd, 'h');  //左转有障碍
			sleep(1);
			you_90(serial_fd,current_yaw);
		}
		else
		{
			control_motor_bizhang(serial_fd, 'q');  //左转无障碍
			sleep(1);
			you_90(serial_fd,current_yaw);
		}
		return 1;
	}
	else if('z' == check_bizhang_state())
	{
		you_90(serial_fd,current_yaw);
		if(check_hou_1s(serial_fd))
		{
			control_motor_bizhang(serial_fd, 'h');  //右转有障碍
			sleep(1);
			zuo_90(serial_fd,current_yaw);
		}
		else
		{
			control_motor_bizhang(serial_fd, 'q');  //右转无障碍
			sleep(1);
			zuo_90(serial_fd,current_yaw);
		}
		return 1;
	}
	return 0;
}

int common_bizhang_no_mpu(int serial_fd)
{
	int mid_test;
	if('d' == check_bizhang_state())
	{
		control_motor_bizhang(serial_fd, 'h');
		sleep(1);
		check_hou_1s(serial_fd);
		control_motor_bizhang(serial_fd, 'z');
		sleep(2);
		//zuo_90(serial_fd,current_yaw);
		if(check_hou_1s(serial_fd))
		{
			control_motor_bizhang(serial_fd, 'h');  //左转有障碍
			sleep(1);
			control_motor_bizhang(serial_fd, 'y');
			sleep(2);
			control_motor_bizhang(serial_fd, 't');
		}
		else
		{
			control_motor_bizhang(serial_fd, 'q');  //左转无障碍
			sleep(1);
			control_motor_bizhang(serial_fd, 'y');
			sleep(2);
			control_motor_bizhang(serial_fd, 't');
		}
		return 1;
	}
	else if('y' == check_bizhang_state())
	{
		control_motor_bizhang(serial_fd, 'z');
		sleep(2);
		if(check_hou_1s(serial_fd))
		{
			control_motor_bizhang(serial_fd, 'h');  //左转有障碍
			sleep(1);
			control_motor_bizhang(serial_fd, 'y');
			sleep(2);
			control_motor_bizhang(serial_fd, 't');
		}
		else
		{
			control_motor_bizhang(serial_fd, 'q');  //左转无障碍
			sleep(1);
			control_motor_bizhang(serial_fd, 'y');
			sleep(2);
			control_motor_bizhang(serial_fd, 't');
		}
		return 1;
	}
	else if('z' == check_bizhang_state())
	{
		control_motor_bizhang(serial_fd, 'y');
		sleep(2);
		if(check_hou_1s(serial_fd))
		{
			control_motor_bizhang(serial_fd, 'h');  //右转有障碍
			sleep(1);
			control_motor_bizhang(serial_fd, 'z');
			sleep(2);
			control_motor_bizhang(serial_fd, 't');
		}
		else
		{
			control_motor_bizhang(serial_fd, 'q');  //右转无障碍
			sleep(1);
			control_motor_bizhang(serial_fd, 'z');
			sleep(2);
			control_motor_bizhang(serial_fd, 't');
		}
		return 1;
	}
	return 0;
}




























