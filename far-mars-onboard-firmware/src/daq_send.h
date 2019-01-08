/*
 * daq_send.h
 *
 * Created: 12/27/2018 5:57:31 PM
 *  Author: David Knight
 */ 


#ifndef DAQ_SEND_H_
#define DAQ_SEND_H_
#include "messages.h"
#include "timestamp.h"

#warning SERCOM USART has not yet been implemented

//! @brief Maximum length of a message to be sent in bytes
#define DAQ_MAX_MESSAGE_SIZE 50
//! @brief Length of the daq send queue
#define DAQ_SEND_QUEUE_LENGTH 10
//! @brief Pin used to control DATA_DIR on RS485 transceiver
#define USART_DATA_DIR PIN_PA22
#define USART_DATA_DIR_DE 1
#define USART_DATA_DIR_RE 0

void daqSendTask(void *pvParameters);
void daqSendCallback(struct usart_module *const module);
int daqSendSensorMessage(struct sensorMessage *msg);
int daqSendString(const char *str);

#endif /* DAQ_SEND_H_ */