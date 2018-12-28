/*
 * daq_send.h
 *
 * Created: 12/27/2018 5:57:31 PM
 *  Author: David Knight
 */ 


#ifndef DAQ_SEND_H_
#define DAQ_SEND_H_

#include "logger.h"

#warning SERCOM USART has not yet been implemented

//! @brief Length of the daq send queue
#define DAQ_SEND_QUEUE_LENGTH 10
//! @brief Pin used to control DATA_DIR on RS485 transceiver
#define USART_DATA_DIR PIN_PA22
#define USART_DATA_DIR_DE 1
#define USART_DATA_DIR_RE 0

//! @brief Structure for message to be sent to daq
struct daqSendMsg {
	uint8_t msgID;
	uint8_t data[3+8]; // Change size of data so daqSendMsg is aligned to 4 bytes
};

void daqSendTask(void *pvParameters);
void daqSendCallback(struct usart_module *const module);
int daqSendMessage(struct daqSendMsg *msg);

#endif /* DAQ_SEND_H_ */