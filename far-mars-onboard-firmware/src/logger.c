/*
 * logger.c
 *
 * Created: 12/26/2018 12:47:23 PM
 *  Author: David Knight
 */

#include <asf.h>
#include <string.h>
#include "logger.h"

#define SD_CARD_WRITE_BUFFER_SIZE 200

static QueueHandle_t messageQueue = NULL;
static QueueHandle_t stringQueue = NULL;
volatile static uint8_t sdCardWriteBuffer[SD_CARD_WRITE_BUFFER_SIZE];
volatile static uint16_t sdCardWriteBufferIdx;

struct stringMessage {
	uint32_t timestamp;
	const char *str;
};

static int appendMessage(void);
static int logStrings(void);
static int sendSdCardWriteBuffer(void);

/**
 * @brief	                Task that logs messages to the SD card.
 * @param[in] *pvParameters Contains task parameters
 * @return	                none.
 */
void loggerTask(void *pvParameters) {
	/* Initialization */
	// Init SD MMC
	uint8_t messagesWaiting = 0;
	sdCardWriteBufferIdx = 0;
	messageQueue = xQueueCreate(MESSAGE_QUEUE_LENGTH, sizeof(struct loggerMessage));
	if (!messageQueue) {
		configASSERT(0);
	}
	stringQueue = xQueueCreate(STRING_QUEUE_LENGTH, sizeof(struct stringMessage));
	if (!stringQueue) {
		configASSERT(0);
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

		logStrings();
		sendSdCardWriteBuffer();
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
		case pressureDataID:
		case gyroscopeDataID:
		case accelerometerDataID:
		case cpuTemperatureDataID:
		case IMUTemperatureDataID:
		case genericMessageID:
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
 * @brief  Logs all of the strings in the stringQueue to the SD Card.
 * @return Returns FMOF_SUCCESS.
 */
static int logStrings(void) {
	struct stringMessage msg;
	uint8_t i;
	uint8_t messagesWaiting = uxQueueMessagesWaiting(stringQueue);
	for (; messagesWaiting; messagesWaiting--) {
		xQueueReceive(stringQueue, &msg, (TickType_t)0);

		// Check if timestamp will overflow the write buffer
		if (sdCardWriteBufferIdx + sizeof(msg.timestamp) >= sizeof(sdCardWriteBuffer)) {
			sendSdCardWriteBuffer();
		}
		// Write the timestamp to the write buffer
		sdCardWriteBuffer[sdCardWriteBufferIdx++] = (msg.timestamp >> 24) & 0xFF;
		sdCardWriteBuffer[sdCardWriteBufferIdx++] = (msg.timestamp >> 16) & 0xFF;
		sdCardWriteBuffer[sdCardWriteBufferIdx++] = (msg.timestamp >>  8) & 0xFF;
		sdCardWriteBuffer[sdCardWriteBufferIdx++] = (msg.timestamp >>  0) & 0xFF;

		// Write the string to the write buffer
		for(i = 0; msg.str[i] != 0; i++) {
			// Check for overflow
			if (sdCardWriteBufferIdx == sizeof(sdCardWriteBuffer)) {
				sendSdCardWriteBuffer();
			}
			sdCardWriteBuffer[sdCardWriteBufferIdx++] = msg.str[i];
		} 
	}
	return FMOF_SUCCESS;
}

/**
 * @brief  Writes the sdCardWriteBuffer to the SD Card.
 * @return Returns FMOF_SUCCESS.
 */
static int sendSdCardWriteBuffer(void) {
	// Implement later
	sdCardWriteBufferIdx = 0;
	return FMOF_SUCCESS;
}

/**
 * @brief			Puts a message on the queue to be logged to the SD card.
 * @param[in] *msg	Contains message to be logged
 * @param[in] level Contains the logging level of the message
 * @return			Returns FMOF_SUCCESS, FMOF_LOGGER_LOW_LOGGING_LEVEL, FMOF_LOGGER_QUEUE_FULL, or FMOF_LOGGER_QUEUE_NOT_INIT.
 */
int logMessage(struct loggerMessage *msg, uint8_t level) {
	if (level < LOGGING_LEVEL) {
		return FMOF_LOGGER_LOW_LOGGING_LEVEL;
	}
	if (!messageQueue) {
		return FMOF_LOGGER_QUEUE_NOT_INIT;
	}
	
	if (xQueueSendToBack(messageQueue, (void *)msg, (TickType_t) 0)) {
		return FMOF_LOGGER_QUEUE_FULL;
	}
	return FMOF_SUCCESS;
}

/**
 * @brief			Puts a message on the queue to be logged to the SD card.
 * @param[in] *msg	Contains message to be logged
 * @param[in] level Contains the logging level of the message
 *
 * @return Status of the logging attempt.
 * @retval FMOF_SUCCESS                  The logging was successful
 * @retval FMOF_LOGGER_LOW_LOGGING_LEVEL The logging level of the message was too low
 * @retval FMOF_LOGGER_QUEUE_NOT_INIT    The logging queue has not yet been initialized
 * @retval FMOF_LOGGER_QUEUE_FULL        The logging queue is full
 */
int logString(const char *msg, uint8_t level) {
	struct stringMessage messageToLog;
	if (level < LOGGING_LEVEL) {
		return FMOF_LOGGER_LOW_LOGGING_LEVEL;
	}
	if (!stringQueue) {
		return FMOF_LOGGER_QUEUE_NOT_INIT;
	}
	
	messageToLog.timestamp = getTimestamp();
	messageToLog.str = msg;

	if (xQueueSendToBack(stringQueue, (void *)&messageToLog, (TickType_t) 0)) {
		return FMOF_LOGGER_QUEUE_FULL;
	}
	return FMOF_SUCCESS;
}