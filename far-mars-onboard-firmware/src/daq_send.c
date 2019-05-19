/*
 * daq_send.c
 *
 * Created: 12/27/2018 5:57:09 PM
 *  Author: David Knight
 */

/**
 * @addtogroup C_daqGroup
 * @{
 */
#include "com.h"
#include "daq_send.h"
#include "far_mars_onboard_firmware.h"
#include <asf.h>

struct daqSensors {
    uint32_t timestamp;
    int32_t TC_uaf;
    uint8_t HALL_methane, HALL_LOX;
    uint16_t BATT_voltage;
    int16_t PT_methane, PT_LOX, PT_helium, PT_chamber, PT_heliumReg;
} __attribute__((packed));

//! @brief Maximum length of a message to be sent in bytes.
#define DAQ_MAX_MESSAGE_SIZE ((sizeof(struct daqSensors) * 2) + 2)
//! @brief Pin used to control DATA_DIR on RS485 transceiver.
#define USART_DATA_DIR PIN_PA22
#define USART_DATA_DIR_DE 1
#define USART_DATA_DIR_RE 0

//! Mutex for protecting the allSensors struct.
static SemaphoreHandle_t allSensorsMutex;
//! Structure that holds the sensor data.
static struct daqSensors allSensors;
//! The buffer that is filled with the message after processing for communication.
volatile static uint8_t sendBuffer[DAQ_MAX_MESSAGE_SIZE];
//! Tracks the size of the message held in the sendBuffer.
volatile static int32_t sendBufferIdx;
//! Task handle used by the RS485 ISR to alert the daqSendTask task that a message has been sent successfully over RS485.
static TaskHandle_t xDaqSendHandle = NULL;

static int configRS485(struct usart_module *module);
void daqSendCallback(struct usart_module *const module);
static int daqPackSendBuffer(void);

/**
 * @brief	                Task that sends messages to the data acquisition computer.
 * @param[in] *pvParameters Contains task parameters
 * @return	                none.
 */
void daqSendTask(void *pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(50);
    TickType_t xLastWakeupTime;
    struct port_config config_port_pin;
    port_get_config_defaults(&config_port_pin);
    config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
    port_pin_set_config(USART_DATA_DIR, &config_port_pin);
    port_pin_set_output_level(USART_DATA_DIR, USART_DATA_DIR_DE);

    struct usart_module rs485_module;
    configRS485(&rs485_module);

    allSensorsMutex = xSemaphoreCreateMutex();
    if (!allSensorsMutex) {
        configASSERT(0);
    }

    xDaqSendHandle = xTaskGetCurrentTaskHandle();

    //port_pin_set_output_level(USART_DATA_DIR, USART_DATA_DIR_RE);
    port_pin_set_output_level(USART_DATA_DIR, USART_DATA_DIR_DE);

    xLastWakeupTime = xTaskGetTickCount();
    while (1) {
        vTaskDelayUntil(&xLastWakeupTime, xFrequency);
        daqPackSendBuffer();
        //port_pin_set_output_level(USART_DATA_DIR, USART_DATA_DIR_DE);
        usart_write_buffer_job(&rs485_module, sendBuffer, sendBufferIdx);
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50)); // returns zero if write didn't finish before timeout, put into a do-while loop later

        //port_pin_set_output_level(USART_DATA_DIR, USART_DATA_DIR_RE);
    }
}

/**
 * @brief	          Configures a sercom peripheral for RS485 communication.
 * @param[in] *module Pointer to the module that needs to be configured for RS485 communication.
 * @return	          Returns FMOF_SUCCESS upon success.
 */
static int configRS485(struct usart_module *module) {
    struct usart_config config_usart;

    usart_get_config_defaults(&config_usart);
    config_usart.baudrate         = 38400;
    config_usart.mux_setting      = USART_RX_3_TX_2_XCK_3;
    config_usart.pinmux_pad0      = PINMUX_UNUSED;
    config_usart.pinmux_pad1      = PINMUX_UNUSED;
    config_usart.pinmux_pad2      = PINMUX_PA20C_SERCOM5_PAD2;
    config_usart.pinmux_pad3      = PINMUX_PA21C_SERCOM5_PAD3;
    config_usart.rs485_guard_time = RS485_GUARD_TIME_1_BIT;
    config_usart.character_size   = USART_CHARACTER_SIZE_8BIT;
    config_usart.stopbits         = USART_STOPBITS_1;
    config_usart.parity           = USART_PARITY_NONE;
    while (usart_init(module, SERCOM5, &config_usart) != STATUS_OK) {};
    usart_enable(module);

    usart_register_callback(module, daqSendCallback, USART_CALLBACK_BUFFER_TRANSMITTED);
    usart_enable_callback(module, USART_CALLBACK_BUFFER_TRANSMITTED);

    return FMOF_SUCCESS;
}

/**
 * @brief             Callback for USART write.
 * @param[in] *module Needed for callback registration
 * @return            none.
 */
void daqSendCallback(struct usart_module *const module) {
    vTaskNotifyGiveFromISR(xDaqSendHandle, NULL);
}

/**
 * @brief  Packs the allSensors struct into the sendBuffer.
 * @return Returns FMOF_SUCCESS.
 */
static int daqPackSendBuffer(void) {
    if (xSemaphoreTake(allSensorsMutex, pdMS_TO_TICKS(50)) != pdPASS) {
        configASSERT(0);
        return FMOF_FAILURE;
    }
    allSensors.timestamp = getTimestamp();
    sendBufferIdx        = escapeBuffer(&allSensors, sizeof(struct daqSensors), sendBuffer, DAQ_MAX_MESSAGE_SIZE);
    xSemaphoreGive(allSensorsMutex);

    if (sendBufferIdx < 0) {
        configASSERT(0);
        return FMOF_FAILURE;
    }
    return FMOF_SUCCESS;
}

/**
 * @brief          Puts a message on the queue to be sent over RS485.
 * @param[in] *msg Contains a message to be sent
 * 
 * @return Status of the send attempt.
 * @retval FMOF_SUCCESS     The send was successful
 * @retval FMOF_INVALID_ARG The pointer *msg points to null
 * @retval FMOF_NOT_INIT    The allSensorsMutex has not yet been initialized
 */
int daqSendSensorMessage(struct sensorMessage *msg) {
    if (!msg) {
        configASSERT(0);
        return FMOF_INVALID_ARG;
    }

    if (xSemaphoreTake(allSensorsMutex, pdMS_TO_TICKS(50)) != pdPASS) {
        return FMOF_NOT_INIT;
    }
    switch (msg->msgID) {
    case pressureRawDataID:
        allSensors.PT_methane = msg->pressureRaw.methane;
        allSensors.PT_LOX     = msg->pressureRaw.LOX;
        allSensors.PT_helium  = msg->pressureRaw.helium;
        allSensors.PT_chamber = msg->pressureRaw.chamber;
        break;
    case pressureRawADC1DataID:
        allSensors.PT_heliumReg = msg->pressureRawADC1.heliumReg;
        break;
    case thermocoupleRawDataID:
        allSensors.TC_uaf = msg->thermocoupleRaw.uaf;
        break;
    case hallEffectDataID:
        allSensors.HALL_methane = msg->hallEffect.methane;
        allSensors.HALL_LOX     = msg->hallEffect.LOX;
        break;
    case batteryRawDataID:
        allSensors.BATT_voltage = msg->batteryRaw.voltage;
        break;
    default:
        break;
    }
    xSemaphoreGive(allSensorsMutex);

    return FMOF_SUCCESS;
}

/**
 * @brief          Puts a string on the queue to be sent over RS485. DEPRECATED
 * @param[in] *msg Contains a string to be sent
 * 
 * @return Status of the send attempt.
 * @retval FMOF_SUCCESS                     The send was successful
 * @retval FMOF_DAQ_SEND_MESSAGE_QUEUE_FULL The send queue is full
 * @retval FMOF_DAQ_SEND_QUEUE_NOT_INIT     The send queue has not yet been initialized
 */
/* DEPRECATED
int daqSendString(const char *str) {
	if (!str) {
		configASSERT(0);
		return FMOF_FAILURE;
	}
	if (!sendQueue) {
		return FMOF_DAQ_SEND_QUEUE_NOT_INIT;
	}
	struct sensorMessage msg;
	msg.msgID = strDataID;
	msg.timestamp = getTimestamp();
	msg.str.str = str;
	msg.str.size = 0;

	if (xQueueSendToBack(sendQueue, (void *)&msg, (TickType_t) 0) != pdPASS) {
		return FMOF_DAQ_SEND_MESSAGE_QUEUE_FULL;
	}
	return FMOF_SUCCESS;
}
*/

/**
 * @} C_daqGroup
 */