/*
 * com.c
 *
 * Created: 3/4/2019 12:38:22 PM
 */

#include "com.h"


#include "com.h"

#define ESCAPE_SOM 0x30 // Byte signifying start of message
#define ESCAPE_EOM 0x03 // Byte signifying end of message
#define ESCAPE_FLAG '~' // Byte signifying the following byte should have a special interpretation, don't use null for the escape flag

/**
 * @brief Escapes an array of bytes for data transmission.
 *
 * @param[in]  *in     Pointer to the array of bytes to be escaped
 * @param[in]  inSize  Size of the array of bytes to be escaped
 * @param[out] *out    Pointer to the array for the escaped bytes to be written to
 * @param[in]  outSize Size of the array for the escaped bytes to be written to, must be at least (inSize*2)+2 to guarantee success
 *
 * @return Returns the number of bytes in the output, negative for errors.
 * @retval COM_BUFFER_SIZE  There was not enough space in the output buffer for the escaped message
 * @retval COM_NULL_POINTER One of the pointers points to NULL
 */
int escapeBuffer(uint8_t *in, uint32_t inSize, uint8_t *out, uint32_t outSize) {
	uint32_t inIdx = 0, outIdx = 0;
	if (!in) {
		COM_ASSERT(0);
		return COM_NULL_POINTER;
	}
	if (!out) {
		COM_ASSERT(0);
		return COM_NULL_POINTER;
	}

	// Insert start of message byte
	if (outSize - outIdx < 1) { return COM_BUFFER_SIZE; }
	out[outIdx++] = ESCAPE_SOM;

	while (inIdx < inSize) {
		switch (in[inIdx]) { // Fall through intended
			case ESCAPE_EOM:
			case ESCAPE_SOM:
			case ESCAPE_FLAG:
				if (outSize - outIdx < 2) { return COM_BUFFER_SIZE; }
				out[outIdx++] = ESCAPE_FLAG;
				out[outIdx++] = in[inIdx++] ^ ESCAPE_FLAG;
				break;
			default:
				if (outSize - outIdx < 1) { return COM_BUFFER_SIZE; }
				out[outIdx++] = in[inIdx++];
				break;
		}	
	}

	// Insert end of message byte
	if (outSize - outIdx < 1) { return COM_BUFFER_SIZE; }
	out[outIdx++] = ESCAPE_EOM;

	return outIdx;
}

/**
 * @brief unescapes an array of bytes from data transmission.
 *
 * @param[in]  *in     Pointer to the array of bytes to be unescaped
 * @param[in]  inSize  Size of the array of bytes to be unescaped
 * @param[out] *out    Pointer to the array for the unescaped bytes to be written to, this may be written to even upon failure
 * @param[in]  outSize Size of the array for the unescaped bytes to be written to
 *
 * @return Returns the number of bytes in the output, negative for errors.
 * @retval COM_BUFFER_SIZE    There was not enough space in the output buffer for the unescaped message
 * @retval COM_NO_ESCAPE_EOM There was not a valid escape character in the input buffer
 * @retval COM_NULL_POINTER   One of the pointers points to NULL
 */
int unEscapeBuffer(uint8_t *in, uint32_t inSize, uint8_t *out, uint32_t outSize) {
	uint32_t inIdx = 0, outIdx = 0;
	if (!in) {
		COM_ASSERT(0);
		return COM_NULL_POINTER;
	}
	if (!out) {
		COM_ASSERT(0);
		return COM_NULL_POINTER;
	}

	// Find start of message
	while (inIdx < inSize) {
		if (in[inIdx++] == ESCAPE_SOM) {
			break;
		}
	}
	
	// Process message
	while (inIdx < inSize) {
		// Process characters that will not add a character to out buffer
		if (in[inIdx] == ESCAPE_EOM) {
			break;
		}
		else if (outSize - outIdx < 1) {
			return COM_BUFFER_SIZE;
		}
		// Process characters that will add a character to out buffer
		else if (in[inIdx] == ESCAPE_FLAG) {
			out[outIdx++] = in[++inIdx] ^ ESCAPE_FLAG;
			inIdx++;
		}
		else if (in[inIdx] == ESCAPE_SOM) {
			out[outIdx++] = in[++inIdx] ^ ESCAPE_FLAG;
			inIdx++;	
		}
		else {
			out[outIdx++] = in[inIdx++];
		}
	}

	if (inIdx == inSize) {
		return COM_NO_EOM;
	}
	return outIdx;
}
