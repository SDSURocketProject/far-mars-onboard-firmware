/*
 * pressure.c
 *
 * Created: 1/2/2019 2:58:04 PM
 *  Author: David Knight
 */

#include "pressure.h"
#include <asf.h>
#include "daq_send.h"
#include "logger.h"

//! Constant by which each pressure reading is divided by when converting to PSI. Defined as 2^(adc resolution).
#define PRESSURE_DIVISION_CONSTANT 4096.0f

#define PRESSURE_METHANE_MAX_PRESSURE 1500.0f
#define PRESSURE_LOX_MAX_PRESSURE 1500.0f
#define PRESSURE_HELIUM_MAX_PRESSURE 5800.0f

struct adc_module pressureADCModule;
static uint16_t adcBuffer[numPressureSensors];
static uint32_t lastTimestamp;
static QueueHandle_t pressureQueue;
static SemaphoreHandle_t pressureADCSemaphore;

void pressureAdcCallback(struct adc_module *const module);

/**
 * @brief Initializes the pressure sensors.
 */
int pressureInit(void) {
	struct adc_config adcConfig;
	uint32_t returned;

	pressureQueue = xQueueCreate(numPressureSensors, sizeof(uint16_t));
	if (!pressureQueue) {
		configASSERT(0);
	}

	pressureADCSemaphore = xSemaphoreCreateMutex();

	adc_get_config_defaults(&adcConfig);
	adcConfig.clock_prescaler = ADC_CLOCK_PRESCALER_DIV16;
	adcConfig.reference = ADC_REFERENCE_INTVCC1;
	adcConfig.positive_input = ADC_POSITIVE_INPUT_PIN0;
	adcConfig.resolution = ADC_RESOLUTION_CUSTOM;
	adcConfig.accumulate_samples = ADC_ACCUMULATE_SAMPLES_16;
	adcConfig.divide_result = ADC_DIVIDE_RESULT_16;
	
	adcConfig.positive_input_sequence_mask_enable = (1 << ADC_POSITIVE_INPUT_PIN0) | // Methane
	                                                (1 << ADC_POSITIVE_INPUT_PIN2) | // LOX
													(1 << ADC_POSITIVE_INPUT_PIN3);  // Helium
	
	if ((returned = adc_init(&pressureADCModule, ADC0, &adcConfig)) != STATUS_OK) {
		configASSERT(0);
	}
	adc_enable(&pressureADCModule);
	adc_enable_positive_input_sequence(&pressureADCModule, adcConfig.positive_input_sequence_mask_enable);

	adc_register_callback(&pressureADCModule, pressureAdcCallback, ADC_CALLBACK_READ_BUFFER);
	adc_enable_callback(&pressureADCModule, ADC_CALLBACK_READ_BUFFER);

	return FMOF_SUCCESS;
}

/**
 * @brief Starts ADC conversions for the pressure sensors.
 * @param[in] wait Maximum amount of time in milliseconds to wait for the ADC be become available
 * 
 * @return Status of the conversion.
 * @retval FMOF_SUCCESS The conversion has been successfully started
 * @retval FMOF_FAILURE The conversion has failed to start
 */
int pressureStartConversion(uint8_t wait) {
	if (xSemaphoreTake(pressureADCSemaphore, pdMS_TO_TICKS(0)) != pdTRUE) {
		return FMOF_FAILURE;
	}
	if (uxQueueSpacesAvailable(pressureQueue) != numPressureSensors) {
		xQueueReset(pressureQueue);
	}
	if (adc_read_buffer_job(&pressureADCModule, adcBuffer, numPressureSensors) != STATUS_OK) {
		configASSERT(0);
	}
	return FMOF_SUCCESS;
}

/**
 * @brief Reads all the pressure sensors after a conversion has been started.
 * @param[out] *pressures Contains pressure values upon return
 * @param[in]  wait       Maximum amount of time in milliseconds to wait for a single pressure sensor to be received
 * 
 * @retval FMOF_SUCCESS                   Successfully read conversion
 * @retval FMOF_PRESSURE_START_CONVERSION Read conversion was called before start conversion
 * @retval FMOF_FAILURE                   Failed to read conversion before timeout
 */
int pressureReadConversion(struct sensorMessage *pressures, uint8_t wait) {
	if (xSemaphoreGetMutexHolder(pressureADCSemaphore) != xTaskGetCurrentTaskHandle()) {
		return FMOF_PRESSURE_START_CONVERSION;
	}

	if (xQueueReceive(pressureQueue, (void *)&(pressures->pressureRaw.methane), pdMS_TO_TICKS(wait)) != pdTRUE) {
		xSemaphoreGive(pressureADCSemaphore);
		return FMOF_FAILURE;
	}
	if (xQueueReceive(pressureQueue, (void *)&(pressures->pressureRaw.LOX), pdMS_TO_TICKS(wait)) != pdTRUE) {
		xSemaphoreGive(pressureADCSemaphore);
		return FMOF_FAILURE;
	}
	if (xQueueReceive(pressureQueue, (void *)&(pressures->pressureRaw.helium), pdMS_TO_TICKS(wait)) != pdTRUE) {
		xSemaphoreGive(pressureADCSemaphore);
		return FMOF_FAILURE;
	}
	pressures->msgID = pressureRawDataID;
	pressures->timestamp = lastTimestamp;
	xSemaphoreGive(pressureADCSemaphore);
	return FMOF_SUCCESS;
}

/**
 * @brief	          Callback for the ADC.
 * @param[in] *module Needed for callback registration
 * @return	          none.
 */
void pressureAdcCallback(struct adc_module *const module) {
	uint8_t i = 0;
	for(; i < numPressureSensors; i++) {
		if (xQueueSendFromISR(pressureQueue, (void *)&adcBuffer[i], NULL) != pdTRUE) {
			configASSERT(0);
		}
	}
	lastTimestamp = getTimestamp();
}

/**
 * @brief Converts a RAW pressure from pressureReadConversion into PSIA.
 * @param[in]  *RAW  The pressure to be converted
 * @param[out] *PSIA The converted pressure
 *
 * @return Returns the status of the conversion.
 * @retval FMOF_SUCCESS The conversion was successful
 * @retval FMOF_FAILURE The message passed in does not contain RAW pressure data
 */
int pressureRawToPSIA(struct sensorMessage *RAW, struct sensorMessage *PSIA) {
	struct sensorMessage PSIG;
	int returned;
	
	if (RAW->msgID != pressureRawDataID) {
		configASSERT(0);
		return FMOF_FAILURE;
	}

	returned = pressureRawToPSIG(RAW, &PSIG);
	if (returned != FMOF_SUCCESS) {
		return returned;
	}
	return pressurePSIGToPSIA(&PSIG, PSIA);;
}

/**
 * @brief Converts a RAW pressure from pressureReadConversion into PSIG.
 * @param[in]  *RAW  The pressure to be converted
 * @param[out] *PSIG The converted pressure
 *
 * @return Returns the status of the conversion.
 * @retval FMOF_SUCCESS The conversion was successful
 * @retval FMOF_FAILURE The message passed in does not contain RAW pressure data
 */
int pressureRawToPSIG(struct sensorMessage *RAW, struct sensorMessage *PSIG) {
	if (RAW->msgID != pressureRawDataID) {
		configASSERT(0);
		return FMOF_FAILURE;
	}

	float methane, LOX, helium;

	methane = (float)RAW->pressureRaw.methane;
	LOX = (float)RAW->pressureRaw.LOX;
	helium = (float)RAW->pressureRaw.helium;

	PSIG->pressurePSIG.methane = ((methane/(PRESSURE_DIVISION_CONSTANT))*(4.5f/4.0f) - (0.5f/4.0f))*PRESSURE_METHANE_MAX_PRESSURE;
	PSIG->pressurePSIG.LOX = ((LOX/(PRESSURE_DIVISION_CONSTANT))*(4.5f/4.0f) - (0.5f/4.0f))*PRESSURE_LOX_MAX_PRESSURE;
	PSIG->pressurePSIG.helium = ((helium/(PRESSURE_DIVISION_CONSTANT))*(5.0f/4.0f) - (1.0f/4.0f))*PRESSURE_HELIUM_MAX_PRESSURE;

	PSIG->msgID = pressurePSIGDataID;
	PSIG->timestamp = RAW->timestamp;
	return FMOF_SUCCESS;
}

/**
 * @brief Converts pressures in PSIA to PSIG.
 * @param[in]  *PSIA The pressure to be converted
 * @param[out] *PSIG The converted pressure
 *
 * @return Returns the status of the conversion.
 * @retval FMOF_SUCCESS The conversion was successful
 * @retval FMOF_FAILURE The message passed in does not contain PSIA data
 */
int pressurePSIAToPSIG(struct sensorMessage *PSIA, struct sensorMessage *PSIG) {
	if (PSIA->msgID != pressurePSIADataID) {
		configASSERT(0);
		return FMOF_FAILURE;
	}

	PSIG->pressurePSIG.methane = PSIA->pressurePSIA.methane+14.7f;
	PSIG->pressurePSIG.LOX     = PSIA->pressurePSIA.LOX+14.7f;
	PSIG->pressurePSIG.helium  = PSIA->pressurePSIA.helium+14.7f;
	PSIG->msgID = pressurePSIGDataID;
	PSIG->timestamp = PSIA->timestamp;
	return FMOF_SUCCESS;
}

/**
 * @brief Converts pressures in PSIG to PSIA.
 * @param[in]  *PSIG The pressure to be converted
 * @param[out] *PSIA The converted pressure
 *
 * @return Returns the status of the conversion.
 * @retval FMOF_SUCCESS The conversion was successful
 * @retval FMOF_FAILURE The message passed in does not contain PSIG data
 */
int pressurePSIGToPSIA(struct sensorMessage *PSIG, struct sensorMessage *PSIA) {
	if (PSIG->msgID != pressurePSIGDataID) {
		configASSERT(0);
		return FMOF_FAILURE;
	}

	PSIA->pressurePSIA.methane = PSIG->pressurePSIG.methane-14.7f;
	PSIA->pressurePSIA.LOX     = PSIG->pressurePSIG.LOX-14.7f;
	PSIA->pressurePSIA.helium  = PSIG->pressurePSIG.helium-14.7f;
	PSIA->msgID = pressurePSIADataID;
	PSIA->timestamp = PSIG->timestamp;
	return FMOF_SUCCESS;
}
