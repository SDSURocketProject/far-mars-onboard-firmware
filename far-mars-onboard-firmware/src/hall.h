/*
 * hall.h
 *
 * Created: 3/28/2019 9:23:42 PM
 *  Author: David Knight
 */ 


#ifndef HALL_H_
#define HALL_H_

#include "far_mars_onboard_firmware.h"
#include "messages.h"

int hallInit(void);
int hallReadConversion(struct sensorMessage *sensor);

#endif /* HALL_H_ */