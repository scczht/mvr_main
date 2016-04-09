#ifndef _BH1750_H
#define _BH1750_H
int init_bh1750(int *fd);
float getdata_bh1750(int *fd);
void close_bh1750(int *fd);
#endif