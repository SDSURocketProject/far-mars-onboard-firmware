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
static struct sensorMessage sendMessage;
volatile static uint8_t sendBuffer[DAQ_MAX_MESSAGE_SIZE];
volatile static uint8_t sendBufferIdx;
static TaskHandle_t xDaqSendHandle = NULL;

static int configRS485(struct usart_module *module);
static int daqPackSendBuffer(void);

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

	struct usart_module rs485_module;
	configRS485(&rs485_module);

	sendQueue = xQueueCreate(DAQ_SEND_QUEUE_LENGTH, sizeof(struct sensorMessage));
	if (!sendQueue) {
		configASSERT(0);
	}
	
	xDaqSendHandle = xTaskGetCurrentTaskHandle();
	
	//port_pin_set_output_level(USART_DATA_DIR, USART_DATA_DIR_RE);
	port_pin_set_output_level(USART_DATA_DIR, USART_DATA_DIR_DE);
	while (1) {
		if (xQueueReceive(sendQueue, &sendMessage, portMAX_DELAY) != pdTRUE) {
			configASSERT(0);
		}
		daqPackSendBuffer();
		//port_pin_set_output_level(USART_DATA_DIR, USART_DATA_DIR_DE);
		usart_write_buffer_job(&rs485_module, sendBuffer, sendBufferIdx);
		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100)); // returns zero if write didn't finish before timeout, put into a do-while loop later
		//port_pin_set_output_level(USART_DATA_DIR, USART_DATA_DIR_RE);
	}
}

static int configRS485(struct usart_module *module) {
	struct usart_config config_usart;

	usart_get_config_defaults(&config_usart);
	config_usart.baudrate    = 38400;
	config_usart.mux_setting = USART_RX_3_TX_2_XCK_3;
	config_usart.pinmux_pad0 = PINMUX_UNUSED;
	config_usart.pinmux_pad1 = PINMUX_UNUSED;
	config_usart.pinmux_pad2 = PINMUX_PA20C_SERCOM5_PAD2;
	config_usart.pinmux_pad3 = PINMUX_PA21C_SERCOM5_PAD3;
	config_usart.rs485_guard_time = RS485_GUARD_TIME_1_BIT;
	config_usart.character_size = USART_CHARACTER_SIZE_8BIT;
	config_usart.stopbits = USART_STOPBITS_1;
	config_usart.parity = USART_PARITY_NONE;
	while (usart_init(module, SERCOM5, &config_usart) != STATUS_OK){};
	usart_enable(module);

	usart_register_callback(module, daqSendCallback, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_enable_callback(module, USART_CALLBACK_BUFFER_TRANSMITTED);

	return FMOF_SUCCESS;
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
 * @brief             Packs the sendMessage into the sendBuffer.
 * @return            Returns FMOF_SUCCESS.
 */
static int daqPackSendBuffer(void) {
	uint8_t *src;
	uint8_t bytesToCopy;
	sendBufferIdx = 0;

	sendBuffer[sendBufferIdx++] = 'A';
	sendBuffer[sendBufferIdx++] = 'B';
	sendBuffer[sendBufferIdx++] = 'C';
	sendBuffer[sendBufferIdx++] = sendMessage.msgID;
	sendBuffer[sendBufferIdx++] = (sendMessage.timestamp >>  0) & 0xFF;
	sendBuffer[sendBufferIdx++] = (sendMessage.timestamp >>  8) & 0xFF;
	sendBuffer[sendBufferIdx++] = (sendMessage.timestamp >> 16) & 0xFF;
	sendBuffer[sendBufferIdx++] = (sendMessage.timestamp >> 24) & 0xFF;
	
	// Write data
	if (sendMessage.msgID == strDataID) {
		src = (uint8_t *)sendMessage.str.str;
		while (*src != '\0') {
			sendBuffer[sendBufferIdx++] = *src++;
		}
		// Write the null terminator
		sendBuffer[sendBufferIdx++] = *src++;
	}
	else {
		bytesToCopy = sensorMessageSizes[sendMessage.msgID];
		src = (uint8_t *)&sendMessage.accelerationRaw; // All union members start at the same memory location
		for(; bytesToCopy > 0; bytesToCopy--) {
			sendBuffer[sendBufferIdx++] = *src++;
		}
	}
	return FMOF_SUCCESS;
}

/**
 * @brief          Puts a message on the queue to be sent over RS485.
 * @param[in] *msg Contains a message to be sent
 * 
 * @return Status of the send attempt.
 * @retval FMOF_SUCCESS                     The send was successful
 * @retval FMOF_DAQ_SEND_MESSAGE_QUEUE_FULL The send queue is full
 * @retval FMOF_DAQ_SEND_QUEUE_NOT_INIT     The send queue has not yet been initialized
 */
int daqSendSensorMessage(struct sensorMessage *msg) {
	if (!sendQueue) {
		return FMOF_DAQ_SEND_QUEUE_NOT_INIT;
	}

	if (xQueueSendToBack(sendQueue, (void *)msg, (TickType_t) 0) != pdPASS) {
		return FMOF_DAQ_SEND_MESSAGE_QUEUE_FULL;
	}
	return FMOF_SUCCESS;
}

/**
 * @brief          Puts a string on the queue to be sent over RS485.
 * @param[in] *msg Contains a string to be sent
 * 
 * @return Status of the send attempt.
 * @retval FMOF_SUCCESS                     The send was successful
 * @retval FMOF_DAQ_SEND_MESSAGE_QUEUE_FULL The send queue is full
 * @retval FMOF_DAQ_SEND_QUEUE_NOT_INIT     The send queue has not yet been initialized
 */
int daqSendString(const char *str) {
	if (!sendQueue) {
		return FMOF_DAQ_SEND_QUEUE_NOT_INIT;
	}
	struct sensorMessage msg;
	msg.msgID = strDataID;
	msg.timestamp = getTimestamp();
	msg.str.str = str;
	msg.str.size = 0;

	if (xQueueSendToBack(sendQueue, (void *)&msg, (TickType_t) 0) != pdPASS) {
		return FMOF_DAQ_SEND_MESSAGE_QUEUE_FULL;
	}
	return FMOF_SUCCESS;
}
