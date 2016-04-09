#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <setjmp.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/time.h>
//#include "rev_queue.h"
#include <pthread.h>
#include <sys/select.h>
#include "ds18b20.h"
#include "debug.h"
#include "mpu6050.h"
#include "hmc5883.h"
#include "bh1750.h"
#include "ir_bizhang.h"
#include "socket.h"
#define MAX_QUE_CONN_NM   5
#define  TRUE    1
#define  FALSE    0
#define PORT 2000
extern int fd_bh1750;
extern struct eeprom fd_hmc;
extern int fd_adc;
extern int fd_pwm;
extern int serial_fd;
struct Motor_Ctrl motor[3]={
{{64,192},{64,192},{64,192},{64,192},{64,192}},
{{85,213},{42,169},{169,85},{42,213},{64,192}},
{{80,208},{47,174},{174,80},{47,208},{64,192}},
};
struct Send_Sensor sensor;
int sockfd;
int mpu_thread_flag;
int ds18b20_thread_flag;
int steer_thread_flag;
int ir_thread_flag;
unsigned char rev_buf[3];
char steer_flag='n';


void get_imu_ypr_q4();
void get_linght(int *fd_light);
void get_compass(struct eeprom *fd_hm);
void get_gas(int *fd_gas);
void get_gear_position(int *fd,int *buf);
void proc_mpu();
void proc_temperature();
void proc_ir();
void proc_steer();
int control_motor(int serial_fd,unsigned char speed ,unsigned char position,unsigned char check,char *steer_flag);
int creat_socket()
{
	struct sockaddr_in server_sockaddr,client_sockaddr;
    //建立socket连接
    if ((sockfd = socket(AF_INET,SOCK_STREAM,0))== -1)
    {
        perror("socket");
        exit(1);
    }
	//设置sockaddr_in 结构中相关参数
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(PORT);
    server_sockaddr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_sockaddr.sin_zero), 8);
	//调用bind()函数
    if (bind(sockfd, (struct sockaddr *)&server_sockaddr,
             sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }
    //调用listen()函数，创建未处理请求的队列
    if (listen(sockfd, MAX_QUE_CONN_NM) == -1)
    {
        perror("listen");
        exit(1);
    }
	DEBUG_printf("Listening……\n");	
	return 1;
}

int main_thread()
{
	printf("accept>>>>>>>>\n");
	pthread_t thread_imu;
	pthread_t thread_temp;
	pthread_t thread_ir;
	pthread_t thread_steer;
	int client_sockfd;
	socklen_t  client_len;
	int buf;
	int sendbytes;
	int recvbytes=0;
	char send_str[100];
	struct sockaddr_in client_sockaddr;
	client_len =sizeof(client_sockaddr);
	while(1)
	{	
		creat_socket();
		fd_set rev_fd;
		struct timeval timeout={0,0};
		mpu_thread_flag=1;
		ds18b20_thread_flag=1;
		steer_thread_flag=1;
		ir_thread_flag=1;
		client_sockfd = accept(sockfd,(struct sockaddr *)&client_sockaddr,&client_len);
		if(-1==client_sockfd)
		{
			perror("accept");
		}
		
		if(pthread_create(&thread_temp,NULL,(void*)proc_temperature,NULL))     //温度采集线程
		{
			perror("pthread_ceate");
			goto socket_error;
		}
		if(pthread_create(&thread_ir,NULL,(void*)proc_ir,NULL))  			//建立红外避障线程
		{
			perror("pthread_ceate");
			goto socket_error;
		}
		if(pthread_create(&thread_steer,NULL,(void*)proc_steer,NULL))		//建立舵机控制线程
		{
			perror("pthread_error");
			goto socket_error;
		}
		/*//
		if(pthread_create(&thread_imu,NULL,(void*)proc_mpu,NULL))  			//建立mpu6050的读取线程
		{
			perror("pthread_ceate");
			goto socket_error;
		}
		 //*/
		while(1)
		{		
			FD_ZERO(&rev_fd);
			FD_SET(client_sockfd,&rev_fd);
			get_compass(&fd_hmc);
			get_linght(&fd_bh1750);
			get_gas(&fd_adc);
			get_gear_position(&fd_pwm,&buf);
			get_imu_ypr_q4();
			memset(send_str,0,sizeof(send_str));
			memcpy(send_str,&sensor,sizeof(sensor));  //把这个结构体中的信息从内存中读入到字符串temp中 
			if ((sendbytes = send(client_sockfd,&send_str, sizeof(sensor),0)) == -1)
			{
				perror("send");
				printf("11111\n");
				goto socket_error;
			}
			switch(select((client_sockfd+1),&rev_fd,NULL,NULL,&timeout))
			{
				case -1: exit(-1);break;
				case 0: break;
				default:
				if(FD_ISSET(client_sockfd,&rev_fd))
				{
					if ((recvbytes = recv(client_sockfd,&rev_buf,3, 0)) == -1)
					{
						perror("recv");
						goto socket_error;
					}
					control_motor(serial_fd,rev_buf[0],rev_buf[1],rev_buf[2],&steer_flag);
				}
			}
			usleep(30000);
		}
	socket_error:
		control_motor(serial_fd,0x01,0xee,0xef,&steer_flag);//进行了障碍处理
		close(sockfd);
		close(client_sockfd);
		mpu_thread_flag=0;
		ds18b20_thread_flag=0;
		steer_thread_flag=0;
		ir_thread_flag=0;
		pthread_join(thread_imu,NULL);
		pthread_join(thread_temp,NULL);
		pthread_join(thread_ir,NULL);
		pthread_join(thread_steer,NULL);
		sleep(10);
	}
}



/*************************************************
注：
	舵机子线程。
**************************************************/
void proc_steer()
{
	int duty=180;
	set_pwm_duty(&fd_pwm,duty);
	while(steer_thread_flag)
	{
		get_gear_position(&fd_pwm,&duty);
		//printf("duty: %d\n",duty);
		switch(steer_flag)
		{
			case '+':
				if(duty < 360)
					duty += 1;
				set_pwm_duty(&fd_pwm,duty);
			break;
			case '-':
				if(duty > 1)
					duty-=1;
				set_pwm_duty(&fd_pwm,duty);				
			break;
			case 'f':
				duty=180;
				set_pwm_duty(&fd_pwm,duty);
				sleep(2);
			break;
			case 'r':
				cruise(&fd_pwm,duty,&steer_flag,&steer_thread_flag);
			break;
		}
		steer_flag='n';
		usleep(33000);
		stop_pwm(&fd_pwm);
	}
	stop_pwm(&fd_pwm);
	printf("steer exit\n");
	pthread_exit(NULL);	

}




/*************************************************
注：
	由于mpu6050的欧拉角和四元数采用了传感器FIFO输出
    缓冲大小为1KB,当读取过慢时会产生溢出。
	这里单独建立一个线程是为了保证对mpu6050的读取
	速度。
**************************************************/
void  proc_mpu()
{
	while(mpu_thread_flag)
	{
		get_imu_ypr_q4();
		usleep(30000);
	}
	printf("mpu exit\n");
	pthread_exit(NULL);
}

/*************************************************
//建立一个子线程获取温度值
//因为温度读取有系统延时
**************************************************/
void proc_temperature()
{
	sleep(1);
	while(ds18b20_thread_flag)
	{
		sensor.temperature = get_temperature("/dev/ds18b20");
		sleep(1);
	}
	printf("ds18b20 exit\n");
	pthread_exit(NULL);
}
/*************************************************
//建立红外避障线程，实时监测障碍
//并设置一个全局标志位
**************************************************/
void proc_ir()
{
	while(ir_thread_flag)
	{
		//if(common_bizhang(serial_fd,&sensor.yaw))
		if(common_bizhang_no_mpu(serial_fd))
		{
			control_motor(serial_fd,rev_buf[0],rev_buf[1],rev_buf[2],&steer_flag);//进行了障碍处理,恢复初始命令
		}
		else
		{
			//printf("no ir deal  \n");									//没进行障碍处理
		}
		usleep(500000);
	}
	printf("ir exit\n");
	pthread_exit(NULL);

}
//得到mpu6050 欧拉角 及四元数的值
void get_imu_ypr_q4()
{
	MPU6050_Pose();
	sensor.yaw=Yaw+180.0;
	//printf("%.1f\n",sensor.yaw);
	sensor.pitch=Pitch;
	sensor.roll=Roll;
	sensor.q0=q0;
	sensor.q1=q1;
	sensor.q2=q2;
	sensor.q3=q3;
}
//得到光照传感器数值
void get_linght(int *fd_light)
{
	sensor.light = getdata_bh1750(fd_light);
}
//得到电子罗盘数值
void get_compass(struct eeprom *fd_hmc)
{
	sensor.angle = multiple_read_HMC5883(fd_hmc);
	//printf("sensor.angle %f\n",multiple_read_HMC5883(fd_hmc));
}

//得到气体传感器数值
void get_gas(int *fd_gas)
{
	sensor.gas = get_adc(fd_gas);	
}

//得到舵机位置
void get_gear_position(int *fd,int *buf)
{
	read_current_attitude(fd,buf);
	sensor.gear_flat = *buf;
}

/*************************************************
由于对电机的控制，要做到尽量不能出错
这里采用了简单的加和校验
另外客户端只发送数据标识，具体的数据在本地设置
数据格式：
    速度 0x00 0x02 0x03  快  中  慢
	方向 0xaa 0xbb 0xcc 0xdd 0xee 前 后 左 右 停
		
		speed   position   check

		0x00	0xaa      
		0x02	……		buf[0]+buf[1]
		0x03	0xee
		steer_flag 控制舵机
**************************************************/
int control_motor(int serial_fd,unsigned char speed ,unsigned char position,unsigned char check,char *steer_flag)
{
	if(speed+position == check)
	{
		if((0x00==speed) || (0x01==speed) || (0x02==speed) ||(0x03==speed))							//高速
		{
			switch(position)
			{
				case 0xaa: 						//前进
					write(serial_fd,&motor[speed].qian[0],1);
					write(serial_fd,&motor[speed].qian[1],1);
					break;
				case 0xbb:						//后退
					write(serial_fd,&motor[speed].hou[0],1);
					write(serial_fd,&motor[speed].hou[1],1);
					break;
				case 0xcc:						//左转
					write(serial_fd,&motor[speed].zuo[0],1);
					write(serial_fd,&motor[speed].zuo[1],1);
					break;
				case 0xdd:						//右转
					write(serial_fd,&motor[speed].you[0],1);
					write(serial_fd,&motor[speed].you[1],1);
					break;
				case 0xee:						//停止
					write(serial_fd,&motor[speed].ting[0],1);
					write(serial_fd,&motor[speed].ting[1],1);
					break;
				case 0x11:
					*steer_flag='+';		//云台左转
					break;
				case 0x22:
					*steer_flag='-';		//云台右转
					break;
				case 0x33:
					*steer_flag='r';		//云台巡航
					break;
				case 0x44:
					*steer_flag='f';		//云台复位
					break;
			}
		}
		
	}
	return 1;
}




