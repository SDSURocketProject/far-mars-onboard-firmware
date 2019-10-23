/*
 * logger.h
 *
 * Created: 12/26/2018 12:47:35 PM
 *  Author: David Knight
 */

#ifndef LOGGER_H_
#define LOGGER_H_

/**
 * @defgroup D_loggerGroup Logger
 * @brief FreeRTOS task for logging data and errors to the SD card.
 * 
 * @{
 */

#include "far_mars_onboard_firmware.h"
#include "messages.h"
#include "timestamp.h"
#include <stdint.h>

//! @brief Logging levels for messages sent to SD card
enum loggingLevels {
    LOG_LEVEL_ALL   = 0,
    LOG_LEVEL_DEBUG = 10,
    LOG_LEVEL_INFO  = 20,
    LOG_LEVEL_DATA  = 30,
    LOG_LEVEL_WARN  = 40,
    LOG_LEVEL_ERROR = 50,
    LOG_LEVEL_FATAL = 60,
    LOG_LEVEL_OFF   = 255
};

//! @brief Current logging level
#define LOGGING_LEVEL LOG_LEVEL_ALL
//! @brief Length of the message queue
#define MESSAGE_QUEUE_LENGTH 100

void loggerTask(void *pvParameters);
int logSensorMessage(struct sensorMessage *msg, uint8_t level);
int logString(const char *msg, uint8_t level);

/**
 * @} D_loggerGroup
 */

#endif /* LOGGER_H_ */