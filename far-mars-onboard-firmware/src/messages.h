/*
 * messages.h
 *
 * Created: 1/6/2019 7:03:07 PM
 *  Author: bigbird42
 */ 


#ifndef MESSAGES_H_
#define MESSAGES_H_

#include <stdint.h>
#include "bno055.h"

//! @brief IDs for all of the valid messages holding sensor data
enum sensorMessageIDs {
	accelerationRawDataID,
	gyroscopeRawDataID,
	eulerRawDataID,
	quaternionRawDataID,
	linearAccelerationRawDataID,
	gravityRawDataID,
	accelerationMpssDataID,
	gyroscopeRpsDataID,
	gyroscopeDpsDataID,
	eulerRadianDataID,
	eulerDegreeDataID,
	linearAccelerationMpssDataID,
	gravityMpssDataID,
	cpuTemperatureRawDataID,
	cpuTemperatureCelciusDataID,
	cpuTemperatureFahrenheitDataID,
	cpuTemperatureKelvinDataID
};

//! @brief Struct containing raw CPU temperature data
struct cpuTemperatureData {
	uint16_t temperature;
};

//! @brief Struct containing CPU temperature data in degrees
struct cpuTemperatureFloatData {
	float temperature;
};

/**
 * @brief Struct that contains all messages that can be sent, must be packed for communication.
 */
struct __attribute__((packed, aligned(1))) sensorMessage {
	uint32_t timestamp;
	uint8_t msgID;

	// Data
	union {
		struct bno055_accel_t              accelerationRawData;
		struct bno055_gyro_t               gyroscopeRawData;
		struct bno055_euler_t              eulerRawData;
		struct bno055_quaternion_t         quaternionRawData;
		struct bno055_linear_accel_t       linearAccelerationRawData;
		struct bno055_gravity_t            gravityRawData;

		struct bno055_accel_float_t        accelerationMpssData;       // Meters per second squared
		struct bno055_gyro_float_t         gyroscopeRpsData;           // Degrees per second
		struct bno055_gyro_float_t         gyroscopeDpsData;           // Radians per second
		struct bno055_euler_float_t        eulerRadianData;
		struct bno055_euler_float_t        eulerDegreeData; 
		struct bno055_linear_accel_float_t linearAccelerationMpssData; // Meters per second squared
		struct bno055_gravity_float_t      gravityMpssData;            // Meters per second squared

		struct cpuTemperatureData          cpuTemperatureRawData;
		struct cpuTemperatureFloatData     cpuTemperatureCelciusData;
		struct cpuTemperatureFloatData     cpuTemperatureFahrenheitData;
		struct cpuTemperatureFloatData     cpuTemperatureKelvinData;
	};
};


#endif /* MESSAGES_H_ */