#ifndef _ADC_H
#define _ADC_H

int init_adc(int *fd);
int get_adc(int *fd);
void close_adc(int *fd);
#endif
