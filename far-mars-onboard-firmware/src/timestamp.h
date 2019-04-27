/*
 * timestamp.h
 *
 * Created: 1/4/2019 1:54:38 PM
 *  Author: David Knight
 */

#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <stdint.h>

int configRTC(void);
uint32_t getTimestamp(void);

#endif /* TIMESTAMP_H_ */