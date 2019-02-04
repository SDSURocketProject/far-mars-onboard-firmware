/*
 * navigation.c
 *
 * Created: 1/9/2019 6:59:25 PM
 *  Author: David Knight
 */ 

#include <asf.h>
#include "navigation.h"
#include "daq_send.h"
#include "logger.h"
#include "pressure.h"

/**
 * @brief This task retrieves sensor data and sends it to logger and daq_send tasks.
 * @param[in] *pvParameters Contains task parameters
 * @return none.
 */
void navigationTask(void *pvParameters) {
	const TickType_t xFrequency = pdMS_TO_TICKS(50);
	TickType_t xLastWakeupTime;
	struct sensorMessage pressure;
	uint32_t pressureReturn;

	xLastWakeupTime = xTaskGetTickCount();
	while(1) {
		vTaskDelayUntil(&xLastWakeupTime, xFrequency);
		// Start conversions
		if ((pressureReturn = pressureStartConversion(10)) != FMOF_SUCCESS) {
			logString("Starting pressure conversion timed out\n", LOG_LEVEL_ERROR);
		}

		// Read conversions
		if(pressureReturn == FMOF_SUCCESS) {
			if (pressureReadConversion(&pressure, 10) != FMOF_SUCCESS) {
				logString("Reading pressure conversion timed out\n", LOG_LEVEL_ERROR);
			}
		}
		
		// Read conversions
		if(pressureReturn == FMOF_SUCCESS) {
			if (pressureRawToPSIG(&pressure, &pressure) != FMOF_SUCCESS) {
				logString("Converting pressure failed\n", LOG_LEVEL_ERROR);
			}
		}
		
		// Log conversions
		if (pressureReturn == FMOF_SUCCESS) {
			daqSendSensorMessage(&pressure);
			logSensorMessage(&pressure, LOG_LEVEL_DATA);
		}
	}
}