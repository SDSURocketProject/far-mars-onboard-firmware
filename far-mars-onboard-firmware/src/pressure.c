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

#define NUM_SAMPLES 32
static uint16_t adcBuffer[NUM_SAMPLES];
static TaskHandle_t xPressureTaskHandle = NULL;

void pressureAdcCallback(struct adc_module *const module);

void pressureTask(void *pvParameters) {
	struct adc_module adcModule;
	struct adc_config adcConfig;
	struct system_pinmux_config config;
	uint32_t avg = 0;
	uint8_t i = 0;
	struct loggerMessage pressureMessage;

	xPressureTaskHandle = xTaskGetCurrentTaskHandle();
	pressureMessage.msgID = pressureDataID;

	/* Config adc */
	adc_get_config_defaults(&adcConfig);

	adcConfig.clock_prescaler = ADC_CLOCK_PRESCALER_DIV16;
	adcConfig.reference = ADC_REFERENCE_INTVCC1;
	adcConfig.positive_input = ADC_POSITIVE_INPUT_PIN2;
	adcConfig.resolution = ADC_RESOLUTION_12BIT;

	adc_init(&adcModule, ADC1, &adcConfig);
	adc_enable(&adcModule);

	/* Register adc callback */
	adc_register_callback(&adcModule, pressureAdcCallback, ADC_CALLBACK_READ_BUFFER);
	adc_enable_callback(&adcModule, ADC_CALLBACK_READ_BUFFER);

	/* Configure analog pins */
	system_pinmux_get_config_defaults(&config);

	/* Analog functions are all on MUX setting B */
	/*
	config.input_pull   = SYSTEM_PINMUX_PIN_PULL_NONE;
	config.mux_position = 1;

	system_pinmux_pin_set_config(PIN_PB08, &config);
	*/
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(10));
		adc_read_buffer_job(&adcModule, adcBuffer, NUM_SAMPLES);
		if (!ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50))) {
			logMessageString("ADC Fail", LOG_LEVEL_ERROR);
		}

		pressureMessage.timestamp = getTimestamp();
		for(; i < NUM_SAMPLES; i++) {
			avg += adcBuffer[i];
		}

		avg /= NUM_SAMPLES;

		pressureMessage.data[0] = avg;

		/*
		daqSendMessage((struct daqSendMsg *)&pressureMessage);
		logMessage(&pressureMessage, LOG_LEVEL_INFO);
		*/
	}
}

void pressureAdcCallback(struct adc_module *const module) {
	vTaskNotifyGiveFromISR(xPressureTaskHandle, NULL);
}