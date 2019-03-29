/*
 * adc1.c
 *
 * Created: 3/28/2019 8:21:47 PM
 *  Author: David Knight
 */ 

#include "far_mars_adc1.h"
#include <asf.h>
#include "timestamp.h"

//! Constant by which each pressure reading is divided by when converting to PSI. Defined as 2^(adc resolution).
#define PRESSURE_DIVISION_CONSTANT 4096UL

struct adc_module adc1Module;
//! Buffer used for receiving data from an ADC read.
static int16_t adc1Buffer[adc1InputNum];
//! Contains the timestamp when the most recent ADC read completed.
static uint32_t lastTimestamp;
static SemaphoreHandle_t adc1ProtectSemaphore;
static SemaphoreHandle_t adc1SyncSemaphore;

// Updated never
//! Number of values used inside the adcOffsetTable.
#define ADC1_OFFSET_TABLE_SIZE 16
//! Offset table used for correcting the error of the onboard ADC.
static const int16_t adc1OffsetTable[ADC1_OFFSET_TABLE_SIZE] = {
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0
};

void adc1Callback(struct adc_module *const module);

/**
 * @brief Initializes the pressure sensors.
 */
int adc1Init(void) {
	struct adc_config adcConfig;
	uint32_t returned;

	adc1ProtectSemaphore = xSemaphoreCreateMutex();
	if (adc1ProtectSemaphore == NULL) {
		configASSERT(0);
	}
	adc1SyncSemaphore = xSemaphoreCreateBinary();
	if (adc1SyncSemaphore == NULL) {
		configASSERT(0);
	}

	adc_get_config_defaults(&adcConfig);
	adcConfig.clock_prescaler = ADC_CLOCK_PRESCALER_DIV16;
	adcConfig.reference = ADC_REFERENCE_INTVCC2; // VDDANA
	adcConfig.positive_input = ADC_POSITIVE_INPUT_PIN2;
	adcConfig.resolution = ADC_RESOLUTION_CUSTOM;
	adcConfig.accumulate_samples = ADC_ACCUMULATE_SAMPLES_8;
	adcConfig.divide_result = ADC_DIVIDE_RESULT_8;

	if ((returned = adc_init(&adc1Module, ADC1, &adcConfig)) != STATUS_OK) {
		configASSERT(0);
		return FMOF_FAILURE;
	}
	
	adc_enable(&adc1Module);

	adc_register_callback(&adc1Module, adc1Callback, ADC_CALLBACK_READ_BUFFER);
	adc_enable_callback(&adc1Module, ADC_CALLBACK_READ_BUFFER);

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
int adc1StartConversion(uint8_t wait) {
	if (xSemaphoreTake(adc1ProtectSemaphore, pdMS_TO_TICKS(0)) != pdPASS) {
		return FMOF_FAILURE;
	}

	if (adc_read_buffer_job(&adc1Module, adc1Buffer, adc1InputNum) != STATUS_OK) {
		configASSERT(0);
		xSemaphoreGive(adc1ProtectSemaphore);
		xSemaphoreGive(adc1SyncSemaphore);
		return FMOF_FAILURE;
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
int adc1ReadConversion(struct sensorMessage *pressures, uint8_t wait) {
	if (xSemaphoreGetMutexHolder(adc1ProtectSemaphore) != xTaskGetCurrentTaskHandle()) {
		return FMOF_PRESSURE_START_CONVERSION;
	}
	// Synchronize with ISR
	if (xSemaphoreTake(adc1SyncSemaphore, pdMS_TO_TICKS(wait)) != pdPASS) {
		xSemaphoreGive(adc1ProtectSemaphore);
		return FMOF_FAILURE;
	}

	pressures->pressureRawADC1.heliumReg = adc1Buffer[heliumReg];

	pressures->msgID = pressureRawADC1DataID;
	pressures->timestamp = lastTimestamp;
	
	xSemaphoreGive(adc1ProtectSemaphore);
	return FMOF_SUCCESS;
}

/**
 * @brief	          Callback for the ADC.
 * @param[in] *module Needed for callback registration
 * @return	          none.
 */
void adc1Callback(struct adc_module *const module) {
	uint8_t i = 0;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	for(; i < adc1InputNum; i++) {
		adc1Buffer[i] += adc1OffsetTable[adc1Buffer[i]/(PRESSURE_DIVISION_CONSTANT/ADC1_OFFSET_TABLE_SIZE)];
	}
	lastTimestamp = getTimestamp();
	// Synchronize with processing task
	xSemaphoreGiveFromISR(adc1SyncSemaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
