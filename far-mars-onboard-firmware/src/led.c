/*
 * led.c
 *
 * Created: 12/20/2018 3:58:19 PM
 *  Author: David Knight
 */
/**
 * @defgroup E_statusGroup Status
 * @brief FreeRTOS task for logging data and errors to the SD card.
 * 
 * @{
 */

#include "led.h"
#include "logger.h"
#include <asf.h>

//! @brief Red LED
#define LED0 PIN_PA12
//! @brief Green LED
#define LED1 PIN_PA13

/**
 * @brief		Task that controls onboard LEDs.
 * @param[in]	*pvParameters Contains task parameters
 * @return		none.
 */
void ledTask(void *pvParameters) {
    /* Initialization */
    struct port_config config_port_pin;
    port_get_config_defaults(&config_port_pin);
    config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
    port_pin_set_config(LED0, &config_port_pin);
    port_pin_set_config(LED1, &config_port_pin);

    port_pin_set_output_level(LED0, 1);
    port_pin_set_output_level(LED1, 1);

    /* Task code */
    while (1) {
        port_pin_set_output_level(LED0, 1);
        port_pin_set_output_level(LED1, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        port_pin_set_output_level(LED0, 0);
        port_pin_set_output_level(LED1, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @} E_statusGroup
 */