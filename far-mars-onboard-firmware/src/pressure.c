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
#define PRESSURE_DIVISION_CONSTANT 4096UL
//! The integer value of the half volt bias that some of the PTs output.
#define PRESSURE_DC_BIAS 409UL

//! The maximum pressure that the methane PT can read
#define PRESSURE_METHANE_MAX_PRESSURE 1500UL
//! The maximum pressure that the lox PT can read
#define PRESSURE_LOX_MAX_PRESSURE 1500UL
//! The maximum pressure that the helium PT can read
#define PRESSURE_HELIUM_MAX_PRESSURE 5800UL

struct adc_module pressureADCModule;
//! Buffer used for receiving data from an ADC read.
static int16_t adcBuffer[numPressureSensors];
//! Contains the timestamp when the most recent ADC read completed.
static uint32_t lastTimestamp;
static SemaphoreHandle_t pressureADCSemaphore;
static SemaphoreHandle_t pressureSyncSemaphore;

// Updated 3-8-2019
//! Number of values used inside the adcOffsetTable.
#define OFFSET_TABLE_SIZE 16
//! Offset table used for correcting the error of the onboard ADC.
static const int16_t adcOffsetTable[OFFSET_TABLE_SIZE] = {
	11, 11, 12, 13,
	13, 13, 13, 13,
	 5,  5,  6,  6,
	 5,  5,  5,  6
};

void pressureAdcCallback(struct adc_module *const module);

/**
 * @brief Initializes the pressure sensors.
 */
int pressureInit(void) {
	struct adc_config adcConfig;
	uint32_t returned;

	pressureADCSemaphore = xSemaphoreCreateMutex();
	if (pressureADCSemaphore == NULL) {
		configASSERT(0);
	}
	pressureSyncSemaphore = xSemaphoreCreateBinary();
	if (pressureSyncSemaphore == NULL) {
		configASSERT(0);
	}

	adc_get_config_defaults(&adcConfig);
	adcConfig.clock_prescaler = ADC_CLOCK_PRESCALER_DIV16;
	adcConfig.reference = ADC_REFERENCE_INTVCC2; // VDDANA
	adcConfig.positive_input = ADC_POSITIVE_INPUT_PIN0;
	adcConfig.resolution = ADC_RESOLUTION_CUSTOM;
	adcConfig.accumulate_samples = ADC_ACCUMULATE_SAMPLES_16;
	adcConfig.divide_result = ADC_DIVIDE_RESULT_16;
	
	adcConfig.positive_input_sequence_mask_enable = (1 << ADC_POSITIVE_INPUT_PIN0) | // Methane
	                                                (1 << ADC_POSITIVE_INPUT_PIN1) | // Battery Sense
	                                                (1 << ADC_POSITIVE_INPUT_PIN2) | // LOX
													(1 << ADC_POSITIVE_INPUT_PIN3);  // Helium
	
	if ((returned = adc_init(&pressureADCModule, ADC0, &adcConfig)) != STATUS_OK) {
		configASSERT(0);
		return FMOF_FAILURE;
	}

	*((volatile uint16_t *)0x4200440A) |= (1<<7); // Enable Rail-to-rail mode

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
	if (xSemaphoreTake(pressureADCSemaphore, pdMS_TO_TICKS(0)) != pdPASS) {
		return FMOF_FAILURE;
	}

	if (adc_read_buffer_job(&pressureADCModule, adcBuffer, numPressureSensors) != STATUS_OK) {
		configASSERT(0);
		xSemaphoreGive(pressureADCSemaphore);
		xSemaphoreGive(pressureSyncSemaphore);
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
int pressureReadConversion(struct sensorMessage *pressures, struct sensorMessage *voltage, uint8_t wait) {
	if (xSemaphoreGetMutexHolder(pressureADCSemaphore) != xTaskGetCurrentTaskHandle()) {
		return FMOF_PRESSURE_START_CONVERSION;
	}
	// Synchronize with ISR
	if (xSemaphoreTake(pressureSyncSemaphore, pdMS_TO_TICKS(wait)) != pdPASS) {
		xSemaphoreGive(pressureADCSemaphore);
		return FMOF_FAILURE;
	}

	pressures->pressureRaw.methane = adcBuffer[pressureMethane];
	voltage->batteryRaw.voltage = adcBuffer[volts];
	pressures->pressureRaw.LOX = adcBuffer[pressureLOX];
	pressures->pressureRaw.helium = adcBuffer[pressureHelium];

	pressures->msgID = pressureRawDataID;
	pressures->timestamp = lastTimestamp;
	
	voltage->msgID = batteryRawDataID;
	voltage->timestamp = lastTimestamp;
	
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
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	for(; i < numPressureSensors; i++) {
		adcBuffer[i] += adcOffsetTable[adcBuffer[i]/(PRESSURE_DIVISION_CONSTANT/OFFSET_TABLE_SIZE)];
	}
	lastTimestamp = getTimestamp();
	// Synchronize with processing task
	xSemaphoreGiveFromISR(pressureSyncSemaphore, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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

	int32_t methane, LOX, helium;

	methane = RAW->pressureRaw.methane;
	LOX     = RAW->pressureRaw.LOX;
	helium  = RAW->pressureRaw.helium;

	methane -= PRESSURE_DC_BIAS; // Remove .5v DC bias
	if (methane < 0) { // Check overflow
		methane = 0;
	}
	PSIG->pressurePSIG.methane = (methane*PRESSURE_METHANE_MAX_PRESSURE*5/4)/PRESSURE_DIVISION_CONSTANT;
	
	LOX -= PRESSURE_DC_BIAS; // Remove .5v DC bias
	if (LOX < 0) { // Check overflow
		LOX = 0;
	}
	PSIG->pressurePSIG.LOX = (LOX*PRESSURE_LOX_MAX_PRESSURE*5/4)/PRESSURE_DIVISION_CONSTANT;

	PSIG->pressurePSIG.helium = (helium*PRESSURE_HELIUM_MAX_PRESSURE)/PRESSURE_DIVISION_CONSTANT;

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

	PSIG->pressurePSIG.methane = PSIA->pressurePSIA.methane+15;
	PSIG->pressurePSIG.LOX     = PSIA->pressurePSIA.LOX+15;
	PSIG->pressurePSIG.helium  = PSIA->pressurePSIA.helium+15;
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

	PSIA->pressurePSIA.methane = PSIG->pressurePSIG.methane-15;
	PSIA->pressurePSIA.LOX     = PSIG->pressurePSIG.LOX-15;
	PSIA->pressurePSIA.helium  = PSIG->pressurePSIG.helium-15;
	PSIA->msgID = pressurePSIADataID;
	PSIA->timestamp = PSIG->timestamp;
	return FMOF_SUCCESS;
}
