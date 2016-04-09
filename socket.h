#ifndef _SOCKET_H
#define _SOCKET_H
struct Send_Sensor
{
	float yaw;
	float pitch;
	float roll;
	float q0;
	float q1;
	float q2;
	float q3;	
	float temperature;
	float angle;
	float light;
	int   gas;
	int   gear_flat;
};
struct Motor_Ctrl
{
	unsigned char qian[2];
	unsigned char hou[2];
	unsigned char zuo[2];
	unsigned char you[2];
	unsigned char ting[2];
};
extern struct Motor_Ctrl  motor[3];
extern struct Send_Sensor sensor;
int main_socket();
int socket_thread();
void get_imu_ypr_q4();
#endif