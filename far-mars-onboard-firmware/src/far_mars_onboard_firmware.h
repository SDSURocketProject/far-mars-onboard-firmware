/*
 * far_mars_onboard_firmware.h
 *
 * Created: 12/26/2018 1:52:11 PM
 *  Author: David Knight
 */ 


#ifndef FAR_MARS_ONBOARD_FIRMWARE_H_
#define FAR_MARS_ONBOARD_FIRMWARE_H_

//! Returns the length of an array
#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

//! All of the return values for functions in the Far Mars Onboard Firmware (FMOF)
enum FMOF_returns {
	FMOF_SUCCESS,				      //! Generic success
	FMOF_INVALID_ARG,                 //! Generic invalid argument, see function return values for more detail
	FMOF_NOT_INIT,                    //! Something needed by the function is not yet initialized, see function return values for more detail
	FMOF_LOGGER_LOW_LOGGING_LEVEL,    //! A message was send to be logged but it's logging level was less than LOGGING_LEVEL
	FMOF_LOGGER_QUEUE_FULL,           //! The logger queue is completely full of messages
	FMOF_LOGGER_QUEUE_NOT_INIT,       //! The logger queue is not yet initialized
	FMOF_LOGGER_INVALID_MSGID,	      //! An invalid message ID was passed to the message logger
	FMOF_PRESSURE_SENSOR_IN_USE,      //! The pressure sensor is being used by another task
	FMOF_PRESSURE_QUEUE_FULL,         //! The pressure sensor is unavailable
	FMOF_PRESSURE_START_CONVERSION,   //! Read conversion was called before start conversion
	FMOF_FAILURE                      //! Generic failure
};

#endif /* FAR_MARS_ONBOARD_FIRMWARE_H_ */