/*
 * adc1.h
 *
 * Created: 3/28/2019 8:22:32 PM
 *  Author: bigbird42
 */ 


#ifndef ADC1_H_
#define ADC1_H_

#include "far_mars_onboard_firmware.h"
#include "messages.h"

enum adc1Inputs {
	heliumReg,
	adc1InputNum
};

int adc1Init(void);
int adc1StartConversion(uint8_t wait);
int adc1ReadConversion(struct sensorMessage *pressures, uint8_t wait);


#endif /* ADC1_H_ */