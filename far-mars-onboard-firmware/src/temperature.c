/*
 * temperature.c
 *
 * Created: 4/21/2019 3:43:34 PM
 *  Author: David Knight
 */

#include "far_mars_onboard_firmware.h"
#include "temperature.h"
#include "timestamp.h"
#include <asf.h>

static struct sdadc_module sdadc_instance;
//! Number of times to sample the SDADC when doing a conversion
#define SDADC_NUM_SAMPLES 16
//! Buffer used for receiving data from the SDADC read.
int32_t sdadcBuffer[numThermocoupleSensors * SDADC_NUM_SAMPLES];
//! Contains the timestamp when the most recently completed SDADC read.
static uint32_t lastTimestamp;
static SemaphoreHandle_t thermocoupleSDADCSemaphore;
static SemaphoreHandle_t thermocoupleSyncSemaphore;

static void sdadcCallback(const struct sdadc_module *const module);

/**
 * @brief Initializes the SDADC.
 */
int thermocoupleInit() {
    struct sdadc_config config_sdadc;
    uint32_t returned;

    thermocoupleSDADCSemaphore = xSemaphoreCreateMutex();
    if (thermocoupleSDADCSemaphore == NULL) {
        configASSERT(0);
        return FMOF_FAILURE;
    }
    thermocoupleSyncSemaphore = xSemaphoreCreateBinary();
    if (thermocoupleSyncSemaphore == NULL) {
        configASSERT(0);
        return FMOF_FAILURE;
    }

    sdadc_get_config_defaults(&config_sdadc);

    config_sdadc.reference.ref_sel            = SDADC_REFERENCE_INTREF;
    config_sdadc.reference.ref_range          = SDADC_REFRANGE_0;
    config_sdadc.mux_input                    = SDADC_MUX_INPUT_AIN1;
    config_sdadc.skip_count                   = 2; // The first 2 samples are invalid
    config_sdadc.correction.gain_correction   = 1;
    config_sdadc.correction.offset_correction = 0;
    // The default output is 24 bit, we need to right shift it to be 16 bit. For some reason it needs to be 7 bit shift not 8, I don't know why
    config_sdadc.correction.shift_correction = 7;

    if ((returned = sdadc_init(&sdadc_instance, SDADC, &config_sdadc)) != STATUS_OK) {
        configASSERT(0);
        return FMOF_FAILURE;
    }

    sdadc_enable(&sdadc_instance);
    sdadc_register_callback(&sdadc_instance, sdadcCallback, SDADC_CALLBACK_READ_BUFFER);
    sdadc_enable_callback(&sdadc_instance, SDADC_CALLBACK_READ_BUFFER);

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
int thermocoupleStartConversion(uint8_t wait) {
    if (xSemaphoreTake(thermocoupleSDADCSemaphore, pdMS_TO_TICKS(0)) != pdPASS) {
        return FMOF_FAILURE;
    }

    if (sdadc_read_buffer_job(&sdadc_instance, sdadcBuffer, numThermocoupleSensors * SDADC_NUM_SAMPLES) != STATUS_OK) {
        configASSERT(0);
        xSemaphoreGive(thermocoupleSDADCSemaphore);
        xSemaphoreGive(thermocoupleSyncSemaphore);
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
int thermocoupleReadConversion(struct sensorMessage *thermocouples, uint8_t wait) {
    if (xSemaphoreGetMutexHolder(thermocoupleSDADCSemaphore) != xTaskGetCurrentTaskHandle()) {
        return FMOF_PRESSURE_START_CONVERSION;
    }
    // Synchronize with ISR
    if (xSemaphoreTake(thermocoupleSyncSemaphore, pdMS_TO_TICKS(wait)) != pdPASS) {
        xSemaphoreGive(thermocoupleSDADCSemaphore);
        return FMOF_FAILURE;
    }
    int32_t average = 0;
    uint32_t i      = 0;
    for (i = 0; i < SDADC_NUM_SAMPLES; i++) {
        average += sdadcBuffer[i];
    }
    thermocouples->thermocoupleRaw.uaf = average / SDADC_NUM_SAMPLES;
    thermocouples->timestamp           = lastTimestamp;
    thermocouples->msgID               = thermocoupleRawDataID;

    xSemaphoreGive(thermocoupleSDADCSemaphore);
    return FMOF_SUCCESS;
}

/**
 * @brief	          Callback for the SDDC.
 * @param[in] *module Needed for callback registration
 * @return	          none.
 */
static void sdadcCallback(const struct sdadc_module *const module) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    lastTimestamp                       = getTimestamp();
    // Synchronize with processing task
    xSemaphoreGiveFromISR(thermocoupleSyncSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
