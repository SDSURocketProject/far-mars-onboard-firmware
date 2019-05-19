/*
 * navigation.h
 *
 * Created: 1/9/2019 6:59:37 PM
 *  Author: David Knight
 */

#ifndef NAVIGATION_H_
#define NAVIGATION_H_

/**
 * @defgroup B_navigationGroup Navigation
 * @brief FreeRTOS task for collecting data from pressure transducers, thermocouples, hall effect sensors, and the IMU.
 * 
 * @{
 */

void navigationTask(void *pvParameters);

/**
 * @} B_navigationGroup
 */

#endif /* NAVIGATION_H_ */