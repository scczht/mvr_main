#ifndef _IR_BIZHANG_H
#define _IR_BIZHANG_H

int get_gpio_156_ir_bizhang();
int get_gpio_149_ir_bizhang();
int common_bizhang(int serial_fd,float *current_yaw);
int common_bizhang_no_mpu(int serial_fd);
#endif