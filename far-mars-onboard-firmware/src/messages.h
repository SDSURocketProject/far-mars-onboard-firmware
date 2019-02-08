/*
 * messages.h
 *
 * Created: 1/6/2019 7:03:07 PM
 *  Author: David Knight
 */ 


#ifndef MESSAGES_H_
#define MESSAGES_H_

#include <stdint.h>
#include "bno055.h"

//! @brief IDs for all of the valid messages holding sensor data
enum sensorMessageIDs {
	accelerationRawDataID,          // 0
	gyroscopeRawDataID,             // 1
	eulerRawDataID,                 // 2
	quaternionRawDataID,            // 3
	linearAccelerationRawDataID,    // 4
	gravityRawDataID,               // 5
	accelerationMpssDataID,		    // 6
	gyroscopeRpsDataID,			    // 7
	gyroscopeDpsDataID,			    // 8
	eulerRadianDataID,			    // 9
	eulerDegreeDataID,			    // 10
	linearAccelerationMpssDataID,   // 11
	gravityMpssDataID,			    // 12
	cpuTemperatureRawDataID,	    // 13
	cpuTemperatureCelciusDataID,    // 14
	cpuTemperatureFahrenheitDataID, // 15
	cpuTemperatureKelvinDataID,	    // 16
	pressureRawDataID,			    // 17
	pressurePSIADataID,			    // 18
	pressurePSIGDataID,			    // 19
	batteryRawDataID,			    // 20
	batteryFloatDataID,			    // 21
	strDataID,					    // 22
	NUM_SENSOR_MESSAGES
};

//! @brief Struct containing raw CPU temperature data
struct cpuTemperatureData {
	uint16_t temperature;
};

//! @brief Struct containing CPU temperature data in degrees
struct cpuTemperatureFloatData {
	float temperature;
};

//! @brief Struct containing pressure sensor data
struct pressureData {
	uint16_t methane, LOX, helium;
};

//! @brief Struct containing pressure sensor data
struct pressureFloatData {
	float methane, LOX, helium;
};

struct batteryData {
	uint16_t voltage;
};

struct batteryFloatData {
	float voltage;
};

//! @brief Struct containing a pointer to a string. Be very careful when using to hold strings that are not string literals.
struct strData {
	uint32_t size;   // Optional parameter, set to zero when unused
	const char *str; // str designed to point to string literals, there is no synchronization between tasks
};

/**
 * @brief Struct that contains all messages that can be sent.
 */
struct sensorMessage {
	uint8_t msgID;
	uint32_t timestamp;

	// Data
	union {
		struct bno055_accel_t              accelerationRaw;
		struct bno055_gyro_t               gyroscopeRaw;
		struct bno055_euler_t              eulerRaw;
		struct bno055_quaternion_t         quaternionRaw;
		struct bno055_linear_accel_t       linearAccelerationRaw;
		struct bno055_gravity_t            gravityRaw;

		struct bno055_accel_float_t        accelerationMpss;       // Meters per second squared
		struct bno055_gyro_float_t         gyroscopeRps;           // Degrees per second
		struct bno055_gyro_float_t         gyroscopeDps;           // Radians per second
		struct bno055_euler_float_t        eulerRadian;
		struct bno055_euler_float_t        eulerDegree; 
		struct bno055_linear_accel_float_t linearAccelerationMpss; // Meters per second squared
		struct bno055_gravity_float_t      gravityMpss;            // Meters per second squared

		struct cpuTemperatureData          cpuTemperatureRaw;
		struct cpuTemperatureFloatData     cpuTemperatureCelcius;
		struct cpuTemperatureFloatData     cpuTemperatureFahrenheit;
		struct cpuTemperatureFloatData     cpuTemperatureKelvin;

		struct pressureData                pressureRaw;
		struct pressureFloatData           pressurePSIA; // Absolute pressure
		struct pressureFloatData           pressurePSIG; // Gauge pressure

		struct batteryData                 batteryRaw;
		struct batteryFloatData            batteryVoltage;

		struct strData                     str;
	};
};

extern uint16_t sensorMessageSizes[];

#endif /* MESSAGES_H_ */