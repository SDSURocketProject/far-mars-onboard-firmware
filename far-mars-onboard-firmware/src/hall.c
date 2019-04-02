/*
 * hall.c
 *
 * Created: 3/28/2019 9:21:41 PM
 *  Author: David Knight
 */ 

#include "hall.h"
#include <asf.h>
#include "timestamp.h"

//! @brief Hall effect sensor input for methane vent valve
#define METHANE_VENT_VALVE_PIN PIN_PB03
//! @brief Hall effect sensor input for LOX vent valve
#define LOX_VENT_VALVE_PIN     PIN_PA28
//! @brief Set to nonzero when hall effect sensor is ready to be read
static uint8_t isHallInit = 0;

/**
 * @brief Initializes the hall effect sensors.
 */
int hallInit(void) {
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction = PORT_PIN_DIR_INPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_UP;
	port_pin_set_config(METHANE_VENT_VALVE_PIN, &config_port_pin);
	port_pin_set_config(LOX_VENT_VALVE_PIN, &config_port_pin);
	
	isHallInit = 1;
	
	return FMOF_SUCCESS;
}

/**
 * @brief Reads the value of the hall effect sensors.
 * @param[out] *sensor Contains the state of the hall effect sensors
 *
 * @return Status of the conversion.
 */
int hallReadConversion(struct sensorMessage *sensor) {
	// Check if GPIO port has been set up yet
	if (!isHallInit) {
		return FMOF_FAILURE;
	}
	sensor->hallEffect.methane = port_pin_get_input_level(METHANE_VENT_VALVE_PIN);
	sensor->hallEffect.LOX = port_pin_get_input_level(LOX_VENT_VALVE_PIN);

	sensor->msgID = hallEffectDataID;
	sensor->timestamp = getTimestamp();

	return FMOF_SUCCESS;
}