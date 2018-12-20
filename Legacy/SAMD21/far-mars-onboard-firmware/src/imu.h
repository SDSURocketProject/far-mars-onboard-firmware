/*
 * imu.h
 *
 * Created: 3/27/2018 7:22:07 PM
 *  Author: Jake Carver
 */ 


#ifndef IMU_H_
#define IMU_H_

#include <asf.h>
#include "bno055.h"
#include "delay.h"

#define I2C_DATA_LENGTH 8
#define SLAVE_ADDRESS 0x28
#define TIMEOUT 1000
#define	BNO055_I2C_BUS_WRITE_ARRAY_INDEX	((u8)1)
#define	I2C_BUFFER_LEN 8
#define I2C0 5


void configure_i2c_master(void);
void bno_api_init(void);
s8 BNO055_I2C_bus_write(u8 , u8 , u8 *, u8 );
s8 BNO055_I2C_bus_read(u8 , u8 , u8 *, u8);
void BNO055_delay_msek(u32);


#endif /* IMU_H_ */