#ifndef __MPU6050_H__
#define __MPU6050_H__
#include "hardware_iic.h"

#define q30  1073741824.0f 

extern float Pitch,Roll,Yaw;
extern float q0,q1,q2,q3;
void init_mpu6050(struct eeprom *fd);
void MPU6050_Pose(void);
void print_imu_ypr(float yaw,float pitch,float roll);
void print_imu_q4(float q0,float q1,float q2,float q3);

#endif
