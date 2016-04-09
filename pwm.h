#ifndef _PWM_H
#define _PWM_H

void close_pwm(int *fd);
void init_pwm(int *fd);
void set_pwm_duty(int *fd,int div);
void stop_pwm(int *fd);
void read_current_attitude(int *fd,int *buf);
int cruise(int *fd,int current_duty,char *r_flag,char thread_flag);

#endif