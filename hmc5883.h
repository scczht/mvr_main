#ifndef _HMC5883_H
#define _HMC5883_H
void init_hmc5883(struct eeprom *e);
float multiple_read_HMC5883(struct eeprom *e);
void close_hmc(struct eeprom *e);


#endif