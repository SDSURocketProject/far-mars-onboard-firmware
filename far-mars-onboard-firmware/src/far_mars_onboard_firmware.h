/*
 * far_mars_onboard_firmware.h
 *
 * Created: 12/26/2018 1:52:11 PM
 *  Author: bigbird42
 */ 


#ifndef FAR_MARS_ONBOARD_FIRMWARE_H_
#define FAR_MARS_ONBOARD_FIRMWARE_H_

//! All of the return values for functions in the Far Mars Onboard Firmware (FMOF)
enum FMOF_returns {
	FMOF_SUCCESS,				   //! Generic success
	FMOF_LOGGER_LOW_LOGGING_LEVEL, //! A message was send to be logged but it's logging level was less than LOGGING_LEVEL
	FMOF_FAILURE				   //! Generic failure
};



#endif /* FAR_MARS_ONBOARD_FIRMWARE_H_ */