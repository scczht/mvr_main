OBJECTS = mputest1 

all:$(OBJECTS)
CC = arm-linux-gcc

$(OBJECTS): inv_mpu_dmp_motion_driver.o mpu6050.o inv_mpu.o main.o hardware_iic.o  hmc5883.o bh1750.o ds18b20.o adc.o ir_bizhang.o serial.o socket.o pwm.o
	$(CC) $(LDFLAGS) $^ -o $@  -lm  -lpthread

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	@-rm *.o $(OBJECTS)
