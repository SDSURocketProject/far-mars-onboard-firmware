/*
 * logger.h
 *
 * Created: 12/26/2018 12:47:35 PM
 *  Author: David Knight
 */ 


#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdint.h>

#warning SD Card and FatFS stuff has not yet been implemented

//! @brief Logging levels for messages sent to SD card
enum loggingLevels {
	ALL = 0,
	LDEBUG = 10,
	INFO = 20,
	WARN = 30,
	ERROR = 40,
	FATAL = 50,
	OFF = 255
};

//! @brief Current logging level
#define LOGGING_LEVEL ALL
//! @brief Length of the message queue
#define MESSAGE_QUEUE_LENGTH 25

//! @brief All of the valid messages to be sent
enum messageIDs {
	pressureData,
	gyroscopeData,
	accelerometerData,
	cpuTemperatureData,
	IMUTemperatureData,
	genericMessage, // Just a string
};

//! @brief Structure for message to be sent to SD card
struct loggerMessage {
	uint32_t timestamp;
	uint8_t msgID;
	uint8_t data[3+8]; // Change size of data so loggerMessage is aligned to 4 bytes
};

void loggerTask(void *pvParameters);
int logMessage(struct loggerMessage msg, uint8_t level);
int logMessageString(const char *msg, uint8_t level);

#endif /* LOGGER_H_ */