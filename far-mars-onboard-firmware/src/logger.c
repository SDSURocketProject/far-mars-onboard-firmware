/*
 * logger.c
 *
 * Created: 12/26/2018 12:47:23 PM
 *  Author: David Knight
 */

#include <asf.h>
#include <string.h>
#include "logger.h"
#include "far_mars_onboard_firmware.h"

static QueueHandle_t messageQueue = NULL;
volatile static uint8_t sdCardWriteBuffer[sizeof(struct loggerMessage)*10];
volatile static uint16_t sdCardWriteBufferIdx;

static int appendMessage(void);

/**
 * @brief		Task that logs messages to the SD card.
 * @param[in]	*pvParameters Contains task parameters
 * @return		none.
 */
void loggerTask(void *pvParameters) {
	/* Initialization */
	// Init SD MMC
	uint8_t messagesWaiting = 0;
	sdCardWriteBufferIdx = 0;
	messageQueue = xQueueCreate(MESSAGE_QUEUE_LENGTH, sizeof(struct loggerMessage));
	if (!messageQueue) {
		//configASSERT(0);
	}

	/* Task code */
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(1000));
		messagesWaiting = uxQueueMessagesWaiting(messageQueue);
		for(; messagesWaiting > 0; messagesWaiting--) {
			if (sdCardWriteBufferIdx+sizeof(struct loggerMessage) >= sizeof(sdCardWriteBuffer)) {
				// sdCardWriteBuffer is full, pass sdCardWriteBuffer to SD card
				sdCardWriteBufferIdx = 0;
			}
			appendMessage();
		}
		// pass sdCardWriteBuffer to SD card
		sdCardWriteBufferIdx = 0;
	}
}

/**
 * @brief  Appends a message to the sdCardWriteBuffer.
 * @return Returns FMOF_SUCCESS.
 */
static int appendMessage(void) {
	struct loggerMessage msg;
	xQueueReceive(messageQueue, &msg, (TickType_t)0);
	sdCardWriteBuffer[sdCardWriteBufferIdx] = msg.timestamp;
	sdCardWriteBufferIdx += sizeof(msg.timestamp);
	sdCardWriteBuffer[sdCardWriteBufferIdx++] = msg.msgID;
	switch(msg.msgID) {
		// Fall through is intentional, each case will need to be implemented to ensure data is packed properly
		case pressureData:
		case gyroscopeData:
		case accelerometerData:
		case cpuTemperatureData:
		case IMUTemperatureData:
		case genericMessage:
		default:
			{
				uint8_t i = 0;
				for(; i < sizeof(msg.data); i++) {
					sdCardWriteBuffer[sdCardWriteBufferIdx + i] = msg.data[i];
				}
				sdCardWriteBufferIdx += sizeof(msg.data);
			}
	}
	return FMOF_SUCCESS;
}

/**
 * @brief			Puts a message on the queue to be logged to the SD card.
 * @param[in] msg	Contains message to be logged
 * @param[in] level Contains the logging level of the message
 * @return			Returns FMOF_SUCCESS, FMOF_LOGGER_LOW_LOGGING_LEVEL, FMOF_LOGGER_MESSAGE_QUEUE_FULL, or FMOF_FAILURE.
 */
int logMessage(struct loggerMessage msg, uint8_t level) {
	if (level < LOGGING_LEVEL) {
		return FMOF_LOGGER_LOW_LOGGING_LEVEL;
	}
	if (!messageQueue) {
		return FMOF_FAILURE;
	}
	
	if (xQueueSendToBack(messageQueue, (void *)&msg, (TickType_t) 0)) {
		return FMOF_LOGGER_MESSAGE_QUEUE_FULL;
	}
	return FMOF_SUCCESS;
}

/**
 * @brief			Puts a message on the queue to be logged to the SD card.
 * @param[in] msg	Contains message to be logged
 * @param[in] level Contains the logging level of the message
 * @return			Returns FMOF_SUCCESS, FMOF_LOGGER_LOW_LOGGING_LEVEL, FMOF_LOGGER_MESSAGE_QUEUE_FULL, or FMOF_FAILURE.
 */
int logMessageString(const char *msg, uint8_t level) {
	struct loggerMessage messageToLog;
	if (level < LOGGING_LEVEL) {
		return FMOF_LOGGER_LOW_LOGGING_LEVEL;
	}
	strncpy((char *)messageToLog.data, msg, sizeof(messageToLog.data));
	messageToLog.msgID = genericMessage;
	messageToLog.timestamp = 0;
	return logMessage(messageToLog, level);
}