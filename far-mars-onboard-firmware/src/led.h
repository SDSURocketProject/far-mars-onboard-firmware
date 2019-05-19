/*
 * led.h
 *
 * Created: 12/20/2018 3:58:37 PM
 *  Author: David Knight
 */

/**
 * @defgroup E_statusGroup Logger
 * @brief FreeRTOS task for indicating OBC status.
 * 
 * @{
 */

#ifndef LED_H_
#define LED_H_

void ledTask(void *pvParameters);

/**
 * @} E_statusGroup
 */

#endif /* LED_H_ */