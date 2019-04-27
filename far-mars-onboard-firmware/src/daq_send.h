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

void daqSendTask(void *pvParameters);
void daqSendCallback(struct usart_module *const module);
int daqSendSensorMessage(struct sensorMessage *msg);
//int daqSendString(const char *str); // DEPRECATED

#endif /* DAQ_SEND_H_ */