/*
 * daq_send.c
 *
 * Created: 12/27/2018 5:57:09 PM
 *  Author: David Knight
 */ 

#include <asf.h>
#include "daq_send.h"
#include "far_mars_onboard_firmware.h"

static QueueHandle_t sendQueue = NULL;
static uint8_t sendBuffer[sizeof(struct daqSendMsg)*2]; // *2 in case we need to escape characters
static TaskHandle_t xDaqSendHandle = NULL;

/**
 * @brief	                Task that sends messages to the data acquisition computer.
 * @param[in] *pvParameters Contains task parameters
 * @return	                none.
 */
void daqSendTask(void *pvParameters) {
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(USART_DATA_DIR, &config_port_pin);
	port_pin_set_output_level(USART_DATA_DIR, USART_DATA_DIR_DE);

	// init USART

	sendQueue = xQueueCreate(DAQ_SEND_QUEUE_LENGTH, sizeof(struct daqSendMsg));
	if (!sendQueue) {
		configASSERT(0);
	}
	
	xDaqSendHandle = xTaskGetCurrentTaskHandle();
	
	port_pin_set_output_level(USART_DATA_DIR, USART_DATA_DIR_RE);
	while (1) {
		if (xQueueReceive(sendQueue, sendBuffer, portMAX_DELAY) != pdTRUE) {
			configASSERT(0);
		}
		// escape sendBuffer if necessary
		port_pin_set_output_level(USART_DATA_DIR, USART_DATA_DIR_DE);
		// pass sendBuffer to RS485 peripheral (SERCOM USART), usart_write_buffer_job
		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100)); // returns zero if write didn't finish before timeout, put into a do-while loop later
		port_pin_set_output_level(USART_DATA_DIR, USART_DATA_DIR_RE);
	}
}

/**
 * @brief             Callback for USART write.
 * @param[in] *module Needed for callback registration
 * @return            none.
 */
void daqSendCallback(struct usart_module *const module) {
	vTaskNotifyGiveFromISR(xDaqSendHandle, NULL);
}

/**
 * @brief             Puts a message on the queue to be sent over RS485.
 * @param[in] *msg    Contains a message to be sent
 * @return    returns FMOF_SUCCESS, FMOF_LOGGER_MESSAGE_QUEUE_FULL, or FMOF_DAQ_SEND_QUEUE_NOT_INIT.
 */
int daqSendMessage(struct daqSendMsg *msg) {
	if (!sendQueue) {
		return FMOF_DAQ_SEND_QUEUE_NOT_INIT;
	}
	
	if (xQueueSendToBack(sendQueue, (void *)msg, (TickType_t) 0)) {
		return FMOF_LOGGER_MESSAGE_QUEUE_FULL;
	}
	return FMOF_SUCCESS;
}
