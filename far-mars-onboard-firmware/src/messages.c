/*
 * messages.c
 *
 * Created: 1/6/2019 10:38:46 PM
 *  Author: David Knight
 */ 

#include "messages.h"

uint16_t sensorMessageSizes[NUM_SENSOR_MESSAGES] = {
	sizeof(struct bno055_accel_t),
	sizeof(struct bno055_gyro_t),
	sizeof(struct bno055_euler_t),
	sizeof(struct bno055_quaternion_t),
	sizeof(struct bno055_linear_accel_t),
	sizeof(struct bno055_gravity_t),
	sizeof(struct bno055_accel_float_t),
	sizeof(struct bno055_gyro_float_t),
	sizeof(struct bno055_gyro_float_t),
	sizeof(struct bno055_euler_float_t),
	sizeof(struct bno055_euler_float_t),
	sizeof(struct bno055_linear_accel_float_t),
	sizeof(struct bno055_gravity_float_t),
	sizeof(struct cpuTemperatureData),
	sizeof(struct cpuTemperatureFloatData),
	sizeof(struct cpuTemperatureFloatData),
	sizeof(struct cpuTemperatureFloatData),
	sizeof(struct pressureData),
	sizeof(struct pressureConvertedData),
	sizeof(struct pressureConvertedData),
	sizeof(struct batteryData),
	sizeof(struct batteryFloatData),
	sizeof(struct strData) // use the size data member to find the length of the string
};