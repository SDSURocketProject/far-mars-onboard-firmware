/*
 * _main_imu_test.c
 *
 * Created: 3/28/2018 6:26:29 PM
 *  Author: Jake Carver
 */ 




#include <asf.h>
#include "imu.h"
#include "debugger_serial_config.h"
#include <string.h>

//! Structure for UART module connected to EDBG (used for unit test output)
struct usart_module cdc_uart_module;

/**
 * \brief Initialize the USART for output
 *
 * Initializes the SERCOM USART used for sending the output to the
 * computer via the EDBG CDC gateway.
 */
static void cdc_uart_init(void)
{
	struct usart_config usart_conf;

	/* Configure USART for unit test output */
	usart_get_config_defaults(&usart_conf);
	usart_conf.mux_setting = CONF_STDIO_MUX_SETTING;
	usart_conf.pinmux_pad0 = CONF_STDIO_PINMUX_PAD0;
	usart_conf.pinmux_pad1 = CONF_STDIO_PINMUX_PAD1;
	usart_conf.pinmux_pad2 = CONF_STDIO_PINMUX_PAD2;
	usart_conf.pinmux_pad3 = CONF_STDIO_PINMUX_PAD3;
	usart_conf.baudrate    = CONF_STDIO_BAUDRATE;

	stdio_serial_init(&cdc_uart_module, CONF_STDIO_USART, &usart_conf);
	usart_enable(&cdc_uart_module);
}

struct bno055_euler_float_t eulerData;

void _main_imu_test(void)
{

	
	system_init();	
	cdc_uart_init();
	delay_init();
	//delay_s(1);
	bno_api_init();
	//bno055_convert_float_euler_hpr_deg(&eulerData);



	printf("RESTART\r\n");


	while(1)
	{

		uint8_t accel_calib_status = 1;
		uint8_t gyro_calib_status = 1;
		uint8_t mag_calib_status = 1;
		uint8_t sys_calib_status = 1;
		uint8_t chip_id = 1;
		uint8_t self_test = 8;
		//uint16_t sw_rev_id = 0;
		int8_t error_result = 0;

		delay_ms(1000);
		error_result += bno055_get_accel_calib_stat(&accel_calib_status);
		error_result += bno055_get_gyro_calib_stat(&gyro_calib_status);
		error_result += bno055_get_mag_calib_stat(&mag_calib_status);
		error_result += bno055_get_sys_calib_stat(&sys_calib_status);


		//error_result += bno055_read_accel_rev_id(&accel_calib_status);
		//error_result += bno055_read_gyro_rev_id(&gyro_calib_status);
		//error_result += bno055_read_mag_rev_id(&mag_calib_status);
		//error_result += bno055_read_sw_rev_id(&sys_calib_status);
		error_result += bno055_get_selftest(&self_test);
		//error_result += bno055_read_chip_id(&chip_id);

		//printf("chip_id: %x\r\n", chip_id);
		printf("self_test: %d\r\n", self_test);
		printf("accel_calib_status: %d\r\n", accel_calib_status);
		printf("gyro_calib_status: %d\r\n", gyro_calib_status);
		printf("mag_calib_status: %d\r\n", mag_calib_status);
		printf("sys_calib_status: %d\r\n", sys_calib_status);
		printf("error: %d\r\n\r\n", error_result);
		
	}



}
