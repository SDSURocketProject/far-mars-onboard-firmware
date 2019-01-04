/*
 * timestamp.c
 *
 * Created: 1/4/2019 1:57:20 PM
 *  Author: David Knight
 */ 

#include <asf.h>
#include "timestamp.h"
#include "far_mars_onboard_firmware.h"

static struct rtc_module rtc_instance;

int configRTC(void) {
	struct rtc_count_config config_rtc_count;
	rtc_count_get_config_defaults(&config_rtc_count);
	config_rtc_count.prescaler = RTC_COUNT_PRESCALER_DIV_32;
	config_rtc_count.mode = RTC_COUNT_MODE_32BIT;
	rtc_count_init(&rtc_instance, RTC, &config_rtc_count);
	rtc_count_enable(&rtc_instance);
	return FMOF_SUCCESS;
}

uint32_t getTimestamp(void) {
	return rtc_count_get_count(&rtc_instance);
}