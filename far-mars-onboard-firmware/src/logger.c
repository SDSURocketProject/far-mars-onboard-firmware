/*
 * logger.c
 *
 * Created: 12/26/2018 12:47:23 PM
 *  Author: David Knight
 */

#include <asf.h>
#include <string.h>
#include <arm_math.h>
#include "logger.h"
#include "com.h"

#define SD_CARD_WRITE_BUFFER_SIZE 256

//! Queue that holds the messages that need to be logged to the SD card.
static QueueHandle_t sensorMessageQueue = NULL;
//! Buffer that holds the messages prior to writing to the SD card.
volatile static uint8_t sdCardWriteBuffer[SD_CARD_WRITE_BUFFER_SIZE];
//! Tracks the size of the sdCardWriteBuffer.
volatile static uint16_t sdCardWriteBufferIdx;
//! Log file that is opened and written to during the program.
static FIL logFile;
//! Struct used for initializing fat file system.
static FATFS fs;

static int initFatFS(void);
static int logSensorMessages(void);
static int sendSdCardWriteBuffer(void);
static int openLog(void);

/**
 * @brief	                Task that logs messages to the SD card.
 * @param[in] *pvParameters Contains task parameters
 * @return	                none.
 */
void loggerTask(void *pvParameters) {
	/* Initialization */
	if (initFatFS() != FMOF_SUCCESS) {
		configASSERT(0);
	}
	sdCardWriteBufferIdx = 0;
	sensorMessageQueue = xQueueCreate(MESSAGE_QUEUE_LENGTH, sizeof(struct sensorMessage));
	if (!sensorMessageQueue) {
		configASSERT(0);
	}

	openLog();
	
	/* Task code */
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(200));
		logSensorMessages();
		sendSdCardWriteBuffer();
	}
	// Never reached, change to close file in the future
	f_close(&logFile);
}

/**
 * @brief  Initializes the SD card and fatfs.
 * @return Returns FMOF_SUCCESS or FMOF_FAILURE if failed to mount file system.
 */
static int initFatFS(void) {
	Ctrl_status status;
	FRESULT res;
	
	sd_mmc_init();
	
	/* Wait card present and ready */
	do {
		status = sd_mmc_test_unit_ready(0);
		if (CTRL_FAIL == status) {
			while (CTRL_NO_PRESENT != sd_mmc_check(0)) {

			}
		}
	} while (CTRL_GOOD != status);
	
	arm_fill_q7((q7_t)0, (q7_t *)&fs, sizeof(FATFS));

	res = f_mount(LUN_ID_SD_MMC_0_MEM, &fs);
	if (FR_INVALID_DRIVE == res) {
		return FMOF_FAILURE;
	}	
	return FMOF_SUCCESS;
}

/**
 * @brief  Creates a new log file on the SD card.
 * @return Returns FMOF_SUCCESS after the new log file has been created correctly.
 */
static int openLog(void) {
	FRESULT sdCardStatus = FR_OK;
	static uint16_t fileIdx = 0;
	static char fileName[13];
	snprintf(fileName, sizeof(fileName), "Data %i.log", fileIdx);

	do {
		if (sdCardStatus == FR_EXIST) {
			fileIdx++;
			snprintf(fileName, sizeof(fileName), "Data %i.log", fileIdx);
		}
		taskENTER_CRITICAL();
		sdCardStatus = f_open(&logFile, fileName, FA_CREATE_NEW | FA_WRITE);
		f_sync(&logFile);

		taskEXIT_CRITICAL();
	} while(sdCardStatus != FR_OK);

	return FMOF_SUCCESS;
}

/**
 * @brief  Logs all the sensor messages in the sensorMessageQueue to the SD Card.
 * @return Returns FMOF_SUCCESS.
 */
static int logSensorMessages(void) {
	struct sensorMessage msg;
	uint16_t bytesToCopy;
	uint8_t *src;
	uint8_t messagesWaiting = uxQueueMessagesWaiting(sensorMessageQueue);
	for (; messagesWaiting; messagesWaiting--) {
		xQueueReceive(sensorMessageQueue, &msg, (TickType_t)0);

		// Write msgID
		if (sdCardWriteBufferIdx >= sizeof(sdCardWriteBuffer)) {
			sendSdCardWriteBuffer();
		}
		sdCardWriteBuffer[sdCardWriteBufferIdx++] = msg.msgID;

		// Check if timestamp will overflow the write buffer
		if (sdCardWriteBufferIdx + sizeof(msg.timestamp) >= sizeof(sdCardWriteBuffer)) {
			sendSdCardWriteBuffer();
		}
		// Write the timestamp to the write buffer in little endian
		sdCardWriteBuffer[sdCardWriteBufferIdx++] = (msg.timestamp >>  0) & 0xFF;
		sdCardWriteBuffer[sdCardWriteBufferIdx++] = (msg.timestamp >>  8) & 0xFF;
		sdCardWriteBuffer[sdCardWriteBufferIdx++] = (msg.timestamp >> 16) & 0xFF;
		sdCardWriteBuffer[sdCardWriteBufferIdx++] = (msg.timestamp >> 24) & 0xFF;
		
		// Write data
		if (msg.msgID == strDataID) {
			src = (uint8_t *)msg.str.str;
			while (*src != '\0') {
				if (sdCardWriteBufferIdx >= sizeof(sdCardWriteBuffer)) {
					sendSdCardWriteBuffer();
				}
				sdCardWriteBuffer[sdCardWriteBufferIdx++] = *src++;
			}
			// Write the null terminator
			if (sdCardWriteBufferIdx >= sizeof(sdCardWriteBuffer)) {
				sendSdCardWriteBuffer();
			}
			sdCardWriteBuffer[sdCardWriteBufferIdx++] = *src++;
		}
		else {
			bytesToCopy = sensorMessageSizes[msg.msgID];
			src = (uint8_t *)&msg.accelerationRaw; // All union members start at the same memory location
			if (sdCardWriteBufferIdx + bytesToCopy >= sizeof(sdCardWriteBuffer)) {
				sendSdCardWriteBuffer();
			}
			for(; bytesToCopy > 0; bytesToCopy--) {
				sdCardWriteBuffer[sdCardWriteBufferIdx++] = *src++;
			}
		}
	}

	return FMOF_SUCCESS;
}

/**
 * @brief  Writes the sdCardWriteBuffer to the SD Card.
 * @return Returns FMOF_SUCCESS.
 */
static int sendSdCardWriteBuffer(void) {
	UINT bytesWritten = 0;
	FRESULT result = 0;

	//taskENTER_CRITICAL();
	result = f_write(&logFile, (void *)sdCardWriteBuffer, sdCardWriteBufferIdx, &bytesWritten);
	f_sync(&logFile);
	//taskEXIT_CRITICAL();

	if (result != FR_OK) {
		return FMOF_FAILURE;
	}
	
	if (sdCardWriteBufferIdx != bytesWritten) {
		return FMOF_FAILURE;
	}
	sdCardWriteBufferIdx = 0;
	return FMOF_SUCCESS;
}

/**
 * @brief			Puts a sensor message on the queue to be logged to the SD card.
 * @param[in] *msg	Contains sensor message to be logged
 * @param[in] level Contains the logging level of the message
 *
 * @return Status of the logging attempt.
 * @retval FMOF_SUCCESS                  The logging was successful
 * @retval FMOF_LOGGER_LOW_LOGGING_LEVEL The logging level of the message was too low
 * @retval FMOF_LOGGER_QUEUE_FULL        The logging queue has not yet been initialized
 * @retval FMOF_LOGGER_QUEUE_NOT_INIT    The logging queue is full
 */
int logSensorMessage(struct sensorMessage *msg, uint8_t level) {
	if (!msg) {
		configASSERT(0);
		return FMOF_FAILURE;
	}
	configASSERT(msg->msgID < NUM_SENSOR_MESSAGES);
	if (level < LOGGING_LEVEL) {
		return FMOF_LOGGER_LOW_LOGGING_LEVEL;
	}
	if (!sensorMessageQueue) {
		return FMOF_LOGGER_QUEUE_NOT_INIT;
	}

	if (xQueueSendToBack(sensorMessageQueue, (void *)msg, (TickType_t) 0) != pdPASS) {
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
	if (!msg) {
		configASSERT(0);
		return FMOF_FAILURE;
	}
	struct sensorMessage messageToLog;
	if (level < LOGGING_LEVEL) {
		return FMOF_LOGGER_LOW_LOGGING_LEVEL;
	}
	if (!sensorMessageQueue) {
		return FMOF_LOGGER_QUEUE_NOT_INIT;
	}
	
	messageToLog.timestamp = getTimestamp();
	messageToLog.msgID = strDataID;
	messageToLog.str.str = msg;
	messageToLog.str.size = 0;

	if (xQueueSendToBack(sensorMessageQueue, (void *)&messageToLog, (TickType_t) 0) != pdPASS) {
		return FMOF_LOGGER_QUEUE_FULL;
	}
	return FMOF_SUCCESS;
}
