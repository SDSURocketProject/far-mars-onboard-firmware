/**
 * @addtogroup A_mainGroup Main
 *
 * @brief Initialization of peripherals and FreeRTOS tasks.
 *  
 * Initialization of peripherals, drivers and FreeRTOS takes place in the main
 * function which is where the firmware begins after startup. The main function
 * needs to do the following:
 *
 * -# call <a href="http://asf.atmel.com/docs/latest/common.components.wifi.winc1500.wifi_serial_example.samd21_xplained_pro/html/group__asfdoc__sam0__system__group.html#ga43f5e0d6db0fb41a437cc9096b32e9b5">system_init()</a>
 *     - ASF requires that we call this function at the start of main.
 * -# call configRTC()
 *     - Starts the RTC which will provide timestamps for the program.
 * -# initialize peripherals
 *     - pressureInit(), thermocoupleInit(), adc1Init(), and hallInit() must
 *       all be called before starting the FreeRTOS scheduler as the tasks we
 *       will create expect some things to already be initialized. The order
 *       that these functions are called is not important.
 * -# Create FreeRTOS tasks
 *     - Creates each of the tasks specified by the Theory of Operation section
 *       in this manual. Each of the tasks are assigned a priority that is
 *       relative to the idle task as it is always lowest priority task in
 *       every FreeRTOS program. Most tasks only require the smallest stack
 *       size specified in FreeRTOSConfig.h except for the logger task because
 *       it has a large queue that it needs extra space for, see the @ref
 *       D_loggerGroup "Logger" for more details. Each task also needs to have
 *       it's return value checked to ensure the task was properly created by
 *       the FreeRTOS kernel.
 * -# call vTaskStartScheduler()
 *     - Finally we can start the FreeRTOS scheduler which passes control over
 *       to the highest priority task that we just created.
 * @{
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include "daq_send.h"
#include "far_mars_adc1.h"
#include "hall.h"
#include "led.h"
#include "logger.h"
#include "navigation.h"
#include "pressure.h"
#include "temperature.h"
#include "timestamp.h"
#include <asf.h>

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);

// clang-format off
#define ledTaskPriority        (tskIDLE_PRIORITY + 2) //!< Priority of the task that blinks the LED.
#define loggerTaskPriority     (tskIDLE_PRIORITY + 2) //!< Priority of the task that logs to the SD card.
#define daqSendTaskPriority    (tskIDLE_PRIORITY + 3) //!< Priority of the task that sends data over RS485.
#define navigationTaskPriority (tskIDLE_PRIORITY + 4) //!< Priority of the task that gathers sensor data.
// clang-format on

/**
 * @brief		Entry point for the program.
 * @return		never returns if working properly.
 */
int main(void) {
    // ASF Initialization
    system_init();
    // far-mars-onboard-firmware peripheral initialization
    configRTC();
    pressureInit();
    thermocoupleInit();
    adc1Init();
    hallInit();

    // FreeRTOS task initialization
    BaseType_t xReturned;
    TaskHandle_t xLedHandle        = NULL;
    TaskHandle_t xloggerHandle     = NULL;
    TaskHandle_t xDaqSendHandle    = NULL;
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
                            configMINIMAL_STACK_SIZE * 5,
                            NULL,
                            loggerTaskPriority,
                            &xloggerHandle);
    if (xReturned != pdPASS) {
        configASSERT(0);
    }
    xReturned = xTaskCreate(daqSendTask,
                            "DAQ send",
                            configMINIMAL_STACK_SIZE * 5,
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

    // Start FreeRTOS tasks
    vTaskStartScheduler();
    // Should never reach here
    while (1) {};
}

/**
 * @brief		Pauses program if a stack overflow is detected.
 * @param[in]	xTask contains the hook of the task that overflowed
 * @param[in]	*pcTaskName contains the name of the task that overflowed
 * @return		none.
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    while (1) {
        configASSERT(0);
    }
}

/**
 * @} end of A_mainGroup group
 */