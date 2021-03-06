#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include <math.h>
#include <stdio.h>
#include "hardware_iic.h"
#include "debug.h"
#define die_if(a, msg) do { do_die_if( a , msg, __LINE__); } while(0);


static signed char gyro_orientation[9] = {-1, 0, 0,
                                           0,-1, 0,
                                           0, 0, 1};
float q0=1.0f,q1=0.0f,q2=0.0f,q3=0.0f;
float Pitch,Roll,Yaw;
unsigned long sensor_timestamp;
short gyro[3], accel[3], sensors;
unsigned char more;
long quat[4];


void init_mpu6050(struct eeprom *e)
{
	int result=0;
	die_if(eeprom_open("/dev/i2c/0", 0x68, EEPROM_TYPE_8BIT_ADDR, e) < 0,
			"unable to open MPU6050 device file ");
	result=mpu_init();
	if(!result)
	{	 		 
	
		printf("mpu initialization complete......\n ");		//mpu initialization complete	 	  

		if(!mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL))		//mpu_set_sensor
			printf("mpu_set_sensor complete ......\n");
		else
			printf("mpu_set_sensor come across error ......\n");

		if(!mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL))	//mpu_configure_fifo
			printf("mpu_configure_fifo complete ......\n");
		else
			printf("mpu_configure_fifo come across error ......\n");

		if(!mpu_set_sample_rate(50))	   	  		//mpu_set_sample_rate
		 printf("mpu_set_sample_rate complete ......\n");
		else
		 	printf("mpu_set_sample_rate error ......\n");

		if(!dmp_load_motion_driver_firmware())   	  			//dmp_load_motion_driver_firmvare
			printf("dmp_load_motion_driver_firmware complete ......\n");
		else
			printf("dmp_load_motion_driver_firmware come across error ......\n");

		if(!dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation))) 	  //dmp_set_orientation
		 	printf("dmp_set_orientation complete ......\n");
		else
		 	printf("dmp_set_orientation come across error ......\n");

		if(!dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
		    DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
		    DMP_FEATURE_GYRO_CAL))		   	 					 //dmp_enable_feature
		 	printf("dmp_enable_feature complete ......\n");
		else
		 	printf("dmp_enable_feature come across error ......\n");

		if(!dmp_set_fifo_rate(50))   	 			 //dmp_set_fifo_rate
		 	printf("dmp_set_fifo_rate complete ......\n");
		else
		 	printf("dmp_set_fifo_rate come across error ......\n");

		run_self_test();		//自检

		if(!mpu_set_dmp_state(1))
		 	printf("mpu_set_dmp_state complete ......\n");
		else
		 	printf("mpu_set_dmp_state come across error ......\n");
	}
	else
	{
	 	printf("mpu  INIT INIT INIT error ......\r\n");
	}
}


void MPU6050_Pose(void)
{
	int i=0;
	i=dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors,&more);	
	if(i<0)
	DEBUG_printf("ERROR  dmp_read_fifo\n");
	DEBUG_printf("gyro   %d   %d   %d\n",gyro[0],gyro[1],gyro[2]);	
	DEBUG_printf("accel   %d   %d   %d  %d\n",accel[0],accel[1],accel[2]);
	DEBUG_printf("quat   %d   %d   %d  %d  %d\n",quat[0],quat[1],quat[2],quat[3]);

	/* Gyro and accel data are written to the FIFO by the DMP in chip frame and hardware units.
	 * This behavior is convenient because it keeps the gyro and accel outputs of dmp_read_fifo and mpu_read_fifo consistent.
	**/
	/*if (sensors & INV_XYZ_GYRO )
	send_packet(PACKET_TYPE_GYRO, gyro);
	if (sensors & INV_XYZ_ACCEL)
	send_packet(PACKET_TYPE_ACCEL, accel); */
	/* Unlike gyro and accel, quaternions are written to the FIFO in the body frame, q30.
	 * The orientation is set by the scalar passed to dmp_set_orientation during initialization. 
	**/
	if(sensors & INV_WXYZ_QUAT )
	{
		q0 = quat[0] / q30;	
		q1 = quat[1] / q30;
		q2 = quat[2] / q30;
		q3 = quat[3] / q30;

		Pitch = asin(-2 * q1 * q3 + 2 * q0* q2)* 57.3;	// pitch
		Roll  = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3;	// roll
		Yaw   = atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * 57.3;	//yaw
	}
}
/**************************实现函数********************************************
*函数原型:		void UART1_ReportIMU(int16_t yaw,int16_t pitch,int16_t roll
				,int16_t alt,int16_t tempr,int16_t press)
*功　　能:		向上位机发送经过解算后的姿态数据
输入参数：
		int16_t yaw 经过解算后的航向角度。单位为0.1度 0 -> 3600  对应 0 -> 360.0度
		int16_t pitch 解算得到的俯仰角度，单位 0.1度。-900 - 900 对应 -90.0 -> 90.0 度
		int16_t roll  解算后得到的横滚角度，单位0.1度。 -1800 -> 1800 对应 -180.0  ->  180.0度
		int16_t alt   气压高度。 单位0.1米。  范围一个整型变量
		int16_t tempr 温度 。 单位0.1摄氏度   范围：直到你的电路板不能正常工作
		int16_t press 气压压力。单位10Pa  一个大气压强在101300pa 这个已经超过一个整型的范围。需要除以10再发给上位机
		int16_t IMUpersec  姿态解算速率。运算IMUpersec每秒。
输出参数：没有
*******************************************************************************/


void print_imu_ypr(float yaw,float pitch,float roll)
{
	char str_yaw[10]={0};
	char str_pitch[10]={0};
	char str_roll[10]={0};
	sprintf(str_yaw,"%5.1f",yaw);
	sprintf(str_pitch,"%5.1f",pitch);
	sprintf(str_roll,"%5.1f",roll);
	sprintf(str_roll,"%5.1f",roll);

	printf("%s",str_yaw);
	printf("  ");
	printf("%s",str_pitch);
	printf("  ");
	printf("%s",str_roll);

}
void print_imu_q4(float q0,float q1,float q2,float q3)
{
	char str_q0[10]={0};
	char str_q1[10]={0};
	char str_q2[10]={0};
	char str_q3[10]={0};
	sprintf(str_q0,"%5.4f",q0);
	sprintf(str_q1,"%5.4f",q1);
	sprintf(str_q2,"%5.4f",q2);
	sprintf(str_q3,"%5.4f",q3);

    printf("  ");
	printf("  ");
	printf("%s",str_q0);
	printf("  ");
	printf("%s",str_q1);
	printf("  ");
	printf("%s",str_q2);
	printf("  ");
	printf("%s",str_q3);
	printf("\n");
	//usleep(30000);
}