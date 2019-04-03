/*
 * com.h
 *
 * Created: 3/4/2019 12:38:33 PM
 */


#ifndef COM_H_
#define COM_H_

#include <stdint.h>

int escapeBuffer(uint8_t *in, uint32_t inSize, uint8_t *out, uint32_t outSize);
int unEscapeBuffer(uint8_t *in, uint32_t inSize, uint8_t *out, uint32_t outSize);

uint16_t CRCCCITT(uint8_t *data, uint32_t length);

#define COM_ASSERT(x)

// Configure the value of each return code for each project
enum escapeReturns {
	COM_BUFFER_SIZE = -1,
	COM_NO_EOM = -2,
	COM_NULL_POINTER = -3,
};


#endif /* COM_H_ */