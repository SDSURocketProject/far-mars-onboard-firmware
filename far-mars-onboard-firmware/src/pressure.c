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

	pressureADCSemaphore = xSemaphoreCreateBinary();

	adc_get_config_defaults(&adcConfig);
	adcConfig.clock_prescaler = ADC_CLOCK_PRESCALER_DIV16;
	adcConfig.reference = ADC_REFERENCE_INTVCC1;
	adcConfig.positive_input = ADC_POSITIVE_INPUT_PIN0;
	adcConfig.resolution = ADC_RESOLUTION_CUSTOM;
	adcConfig.accumulate_samples = ADC_ACCUMULATE_SAMPLES_16;
	adcConfig.divide_result = ADC_DIVIDE_RESULT_16;
	
	adcConfig.positive_input_sequence_mask_enable = (1 << ADC_POSITIVE_INPUT_PIN0) |
	                                                (1 << ADC_POSITIVE_INPUT_PIN1) |
													(1 << ADC_POSITIVE_INPUT_PIN2);
	
	if ((returned = adc_init(&pressureADCModule, ADC1, &adcConfig)) != STATUS_OK) {
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
 * @param[in] wait Maximum amount of time in milliseconds to wait for the ADC be become available.
 */
int pressureStartConversion(uint8_t wait) {
	xSemaphoreTake(pressureADCSemaphore, pdMS_TO_TICKS(wait));
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
 * @return Returns FMOF_SUCCESS upon success or FMOF_FAILURE upon failure.
 */
int pressureReadConversion(struct sensorMessage *pressures, uint8_t wait) {
	if (xQueueReceive(pressureQueue, (void *)&(pressures->pressureRaw.P1), pdMS_TO_TICKS(wait)) != pdTRUE) {
		xSemaphoreGive(pressureADCSemaphore);
		return FMOF_FAILURE;
	}
	if (xQueueReceive(pressureQueue, (void *)&(pressures->pressureRaw.P2), pdMS_TO_TICKS(wait)) != pdTRUE) {
		xSemaphoreGive(pressureADCSemaphore);
		return FMOF_FAILURE;
	}
	if (xQueueReceive(pressureQueue, (void *)&(pressures->pressureRaw.P3), pdMS_TO_TICKS(wait)) != pdTRUE) {
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

int pressureRawToPSIA(struct sensorMessage *RAW, struct sensorMessage *PSIA) {
	return FMOF_SUCCESS;
}

int pressureRawToPSIG(struct sensorMessage *RAW, struct sensorMessage *PSIG) {
	return FMOF_SUCCESS;
}

/**
 * @brief Converts pressures in PSIA to PSIG.
 * @param[in] *PSIA The pressure to be converted
 * @param[out] *PSIG The converted pressure
 * @return Returns FMOF_SUCCESS upon successful conversion, otherwise returns FMOF_FAILURE.
 */
int pressurePSIAToPSIG(struct sensorMessage *PSIA, struct sensorMessage *PSIG) {
	if (PSIA->msgID != pressurePSIADataID) {
		configASSERT(0);
		return FMOF_FAILURE;
	}
	PSIG->pressurePSIG.P1 = PSIA->pressurePSIA.P1+14.7;
	PSIG->pressurePSIG.P2 = PSIA->pressurePSIA.P2+14.7;
	PSIG->pressurePSIG.P3 = PSIA->pressurePSIA.P3+14.7;
	PSIG->msgID = pressurePSIGDataID;
	return FMOF_SUCCESS;
}

/**
 * @brief Converts pressures in PSIG to PSIA.
 * @param[in] *PSIG The pressure to be converted
 * @param[out] *PSIA The converted pressure
 * @return Returns FMOF_SUCCESS upon successful conversion, otherwise returns FMOF_FAILURE.
 */
int pressurePSIGToPSIA(struct sensorMessage *PSIG, struct sensorMessage *PSIA) {
	if (PSIG->msgID != pressurePSIGDataID) {
		configASSERT(0);
		return FMOF_FAILURE;
	}
	PSIA->pressurePSIA.P1 = PSIG->pressurePSIG.P1-14.7;
	PSIA->pressurePSIA.P2 = PSIG->pressurePSIG.P2-14.7;
	PSIA->pressurePSIA.P3 = PSIG->pressurePSIG.P3-14.7;
	PSIG->msgID = pressurePSIADataID;
	return FMOF_SUCCESS;
}
