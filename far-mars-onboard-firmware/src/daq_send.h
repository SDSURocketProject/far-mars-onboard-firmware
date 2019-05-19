/*
 * daq_send.h
 *
 * Created: 12/27/2018 5:57:31 PM
 *  Author: David Knight
 */

/**
* @defgroup C_daqGroup Data Acquisition
* @brief FreeRTOS task for sending sensor data to the data acquisition computer.
* 
* @{
*/

#ifndef DAQ_SEND_H_
#define DAQ_SEND_H_
#include "messages.h"
#include "timestamp.h"

void daqSendTask(void *pvParameters);
int daqSendSensorMessage(struct sensorMessage *msg);
//int daqSendString(const char *str); // DEPRECATED

/**
 * @} C_daqGroup
 */

#endif /* DAQ_SEND_H_ */