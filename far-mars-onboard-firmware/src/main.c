/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include "timestamp.h"
#include "led.h"
#include "logger.h"
#include "daq_send.h"
#include "pressure.h"
#include "navigation.h"
#include "far_mars_adc1.h"
#include "hall.h"

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName );

#define ledTaskPriority        (tskIDLE_PRIORITY + 1) //!< Priority of the task that blinks the LED.
#define loggerTaskPriority     (tskIDLE_PRIORITY + 2) //!< Priority of the task that logs to the SD card.
#define daqSendTaskPriority    (tskIDLE_PRIORITY + 3) //!< Priority of the task that sends data over RS485.
#define navigationTaskPriority (tskIDLE_PRIORITY + 4) //!< Priority of the task that gathers sensor data.

/**
 * @brief		Entry point for the program.
 * @return		never returns if working properly.
 */
int main (void) {
	system_init();
	configRTC();
	pressureInit();
	adc1Init();
	hallInit();

	BaseType_t xReturned;
	TaskHandle_t xLedHandle = NULL;
	TaskHandle_t xloggerHandle = NULL;
	TaskHandle_t xDaqSendHandle = NULL;
	TaskHandle_t xNavigationHandle = NULL;

	xReturned = xTaskCreate(ledTask,
							"LED",
							configMINIMAL_STACK_SIZE,
							NULL,
							ledTaskPriority,
							&xLedHandle);
	if (xReturned != pdPASS) {
		configASSERT(0);
	}
	xReturned = xTaskCreate(loggerTask,
							"Logger",
							configMINIMAL_STACK_SIZE*5,
							NULL,
							loggerTaskPriority,
							&xloggerHandle);
	if (xReturned != pdPASS) {
		configASSERT(0);
	}
	xReturned = xTaskCreate(daqSendTask,
	                        "DAQ send",
							configMINIMAL_STACK_SIZE*5,
							NULL,
							daqSendTaskPriority,
							&xDaqSendHandle);
	if (xReturned != pdPASS) {
		configASSERT(0);
	}

	xReturned = xTaskCreate(navigationTask,
	                        "Navigation",
							configMINIMAL_STACK_SIZE,
							NULL,
							navigationTaskPriority,
							&xNavigationHandle);
	if (xReturned != pdPASS) {
		configASSERT(0);
	}

	/* Insert application code here, after the board has been initialized. */
	vTaskStartScheduler();
	// Should never reach here
	while(1);
}

/**
 * @brief		Pauses program if a stack overflow is detected.
 * @param[in]	xTask contains the hook of the task that overflowed
 * @param[in]	*pcTaskName contains the name of the task that overflowed
 * @return		none.
 */
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ) {
	while(1) {
		configASSERT(0);
	}
}
