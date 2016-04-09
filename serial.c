#include     <stdio.h>      /*标准输入输出定义*/
#include     <stdlib.h>     /*标准函数库定义*/
#include     <unistd.h>     /*Unix标准函数定义*/
#include     <sys/types.h>  /**/
#include     <sys/stat.h>   /**/
#include     <fcntl.h>      /*文件控制定义*/
#include     <termios.h>    /*PPSIX终端控制定义*/
#include     <errno.h>      /*错误号定义*/
#include     <string.h>
#define  TRUE 0
#define  FALSE -1
/***@brief  设置串口通信速率
*@param  fd     类型int  打开串口的文件句柄
*@param  speed  类型int  串口速度
*@return  void*/

int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	    B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {115200,38400,  19200,  9600,  4800,  2400,  1200,  300,
	    38400,  19200,  9600, 4800, 2400, 1200,  300, };

//设置波特率
void set_speed(int fd, int speed)
{
  int   i;
  int   status;
  struct termios   Opt;
  tcgetattr(fd, &Opt);
  for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
   {
   	if  (speed == name_arr[i])
   	{
   	    tcflush(fd, TCIOFLUSH);  //清空串口的缓冲区
    	cfsetispeed(&Opt, speed_arr[i]);
    	cfsetospeed(&Opt, speed_arr[i]);
    	status = tcsetattr(fd, TCSANOW, &Opt);
    	if  (status != 0)
            perror("tcsetattr fd1");
     	return;
     	}
   		tcflush(fd,TCIOFLUSH);//清空串口的缓冲区
   }
}
/**
*@brief   设置串口数据位，停止位和效验位
*@param  fd     类型 int  打开的串口文件句柄*
*@param  databits 类型 int 数据位  取值为7 或者*
*@param  stopbits 类型 int 停止位  取值为1 或者*
*@param  parity  类型 int  效验类型取值为N,E,O,,S
*/
//此类是封好的比较完整的类 可以直接使用
int set_Parity(int fd, int databits, int stopbits, int parity)
{
	struct termios options;

	if(tcgetattr(fd,&options)!=0)
	{
		perror("SetupSerial 1");
		return(FALSE);
	}
	options.c_cflag &= ~CSIZE;

	switch (databits) /*设置数据位数*/
	{
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr,"Unsupported data size\n"); return (FALSE);
	}
	//设置校验类型
	switch (parity)
	{
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
			options.c_iflag |= INPCK;             /* Disnable parity checking */
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;     /* Enable parity */
			options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
			options.c_iflag |= INPCK;       /* Disnable parity checking */
			break;
		case 'S':
		case 's':  /*as no parity*/
	    		options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;break;
		default:
			fprintf(stderr,"Unsupported parity\n");
			return (FALSE);
	}

	//设置停止位
	switch (stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
	   		break;
		default:
		 	fprintf(stderr,"Unsupported stop bits\n");
		 	return (FALSE);
	}

	/* Set input parity option */
	if (parity != 'n')options.c_iflag |= INPCK ;
	//清bit位  关闭字符映射 0x0a 0x0d
	options.c_iflag &= ~(INLCR|ICRNL);
	//清bit位  关闭流控字符 0x11 0x13
	options.c_iflag &= ~(IXON);

	//需要注意的是:
	//如果不是开发终端之类的，只是串口传输数据，而不需要串口来处理，那么使用原始模式(Raw Mode)方式来通讯，设置方式如下：
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	options.c_oflag  &= ~OPOST;   /*Output*/

	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME]=100;//设置超时10秒
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	if (tcsetattr(fd,TCSANOW,&options) != 0)
	{
		perror("SetupSerial 3");
		return (FALSE);
	}
	return 1;
}
/**
*@breif 打开串口
*/
int OpenDev(char *Dev)
{
int	fd = open( Dev, O_RDWR| O_NOCTTY | O_NDELAY );         //
	if (-1 == fd)
		{ /*设置数据位数*/
			perror("Can't Open Serial Port");
			return -1;
		}
	return fd;
}

void init_serial(int *s_fd,int speed)
{
int ret=0;
 char *dev ="/dev/ttySAC2";

	//打开串口文件
	*s_fd = OpenDev(dev);

	//如果文件打开成功
	if (*s_fd>0)
	{
        printf("Open Succeed\n");
    	set_speed(*s_fd,speed);//设置波特率
	}
	else
	{
	printf("Can't Open Serial Port!\n");
	exit(0);
	}
	//设置串口的属性
	ret=set_Parity(*s_fd,8,1,'N');
	if (ret== 0)
        {
    	printf("Set Parity Error\n");
    	exit(1);
  	}
}
