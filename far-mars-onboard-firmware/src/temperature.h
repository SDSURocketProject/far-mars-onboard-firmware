/*
 * temperature.h
 *
 * Created: 4/21/2019 3:43:58 PM
 *  Author: David Knight
 */ 


#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include <stdint.h>
#include "messages.h"

enum thermocoupleSensors {
	UAF,
	numThermocoupleSensors
};

int thermocoupleInit();
int thermocoupleStartConversion(uint8_t wait);
int thermocoupleReadConversion(struct sensorMessage *thermocouples, uint8_t wait);


#endif /* TEMPERATURE_H_ */