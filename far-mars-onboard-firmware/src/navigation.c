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
#include "far_mars_adc1.h"
#include "hall.h"

/**
 * @brief This task retrieves sensor data and sends it to logger and daq_send tasks.
 * @param[in] *pvParameters Contains task parameters
 * @return none.
 */
void navigationTask(void *pvParameters) {
	const TickType_t xFrequency = pdMS_TO_TICKS(50);
	TickType_t xLastWakeupTime;
	struct sensorMessage pressure;
	struct sensorMessage voltage;
	struct sensorMessage pressureAdc1;
	struct sensorMessage hall;
	uint32_t pressureReturn;
	uint32_t adc1Return;
	uint32_t hallReturn;

	xLastWakeupTime = xTaskGetTickCount();
	while(1) {
		vTaskDelayUntil(&xLastWakeupTime, xFrequency);
		// Start conversions
		if ((pressureReturn = pressureStartConversion(10)) != FMOF_SUCCESS) {
			logString("Starting pressure conversion timed out\n", LOG_LEVEL_ERROR);
		}
		if ((adc1Return = adc1StartConversion(10)) != FMOF_SUCCESS) {
			logString("Starting adc 1 conversion timed out\n", LOG_LEVEL_ERROR);
		}

		// Read conversions
		if(pressureReturn == FMOF_SUCCESS) {
			if (pressureReadConversion(&pressure, &voltage, 10) != FMOF_SUCCESS) {
				logString("Reading pressure conversion timed out\n", LOG_LEVEL_ERROR);
			}
		}
		if(adc1Return == FMOF_SUCCESS) {
			if (adc1ReadConversion(&pressureAdc1, 10) != FMOF_SUCCESS) {
				logString("Reading adc 1 conversion timed out\n", LOG_LEVEL_ERROR);
			}
		}
		if (hallReadConversion(&hall) != FMOF_SUCCESS) {
			logString("Failed to read hall effect sensors\n", LOG_LEVEL_ERROR);
		}
		
		// Read conversions
		//if(pressureReturn == FMOF_SUCCESS) {
		//	if (pressureRawToPSIG(&pressure, &pressure) != FMOF_SUCCESS) {
		//		logString("Converting pressure failed\n", LOG_LEVEL_ERROR);
		//	}
		//}

		// Log conversions
		if (pressureReturn == FMOF_SUCCESS) {
			daqSendSensorMessage(&pressure);
			logSensorMessage(&pressure, LOG_LEVEL_DATA);
			logSensorMessage(&voltage, LOG_LEVEL_DATA);
		}
		if (adc1Return == FMOF_SUCCESS) {
			logSensorMessage(&pressureAdc1, LOG_LEVEL_DATA);
		}
		if (hallReturn == FMOF_SUCCESS) {
			logSensorMessage(&hall, LOG_LEVEL_DATA);
		}
	}
}