//ARDUINO TEST BOARD PIN HOOKUP NOTES
/*
	0.ASSUME Non-USB power port is in bottom left while holding
	1.Grab SD card breakout, assign its pins in the following way:
	2.CD pin -> DIG_PIN_4
	3.CS pin -> DIG_PIN_5
	4.DI pin -> ANALOG_1
	5.DO pin -> SPI_MIDDLE_LEFT  (What Step 0 was for)
	6.CLK pin -> ANALOG_2
	7.PULLUP CS to 3v 
	8.Make Sure BAUD is the same for serial terminal
*/

#include <asf.h>
#include "debugger_serial_config.h"
#include <string.h>
#include <stdio.h>



#define NUM_SAMP 32
uint16_t adc_buffer[NUM_SAMP];

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

struct adc_module adc_instance;

void adc_complete_callback(const struct adc_module *const module)
{
	//compute the average
	uint32_t avg = 0;
	float raw_voltage = 0;
	for(uint8_t i=0;i<NUM_SAMP;i++)
	{
		avg += adc_buffer[i];
	}

	//avg /= NUM_SAMP;

	raw_voltage = (((float)avg)*(4.9f))/(((float)NUM_SAMP)*(float)(1<<12));//4.9 is the scalar, when taking into account 0 source impediance, 5 when 100 ohms

	printf("ADC RAW VOTLAGE: %f V\r\n", raw_voltage);
}

void configure_adc(void)
{
	struct adc_config config_adc;
	adc_get_config_defaults(&config_adc);

	config_adc.gain_factor = ADC_GAIN_FACTOR_DIV2;
	config_adc.clock_prescaler = ADC_CLOCK_PRESCALER_DIV16;
	config_adc.reference = ADC_REFERENCE_INTVCC1;
	config_adc.positive_input = ADC_POSITIVE_INPUT_PIN2;
	config_adc.resolution = ADC_RESOLUTION_12BIT;

	adc_init(&adc_instance, ADC, &config_adc);

	adc_enable(&adc_instance);

	adc_register_callback(&adc_instance, adc_complete_callback, ADC_CALLBACK_READ_BUFFER);
	adc_enable_callback(&adc_instance, ADC_CALLBACK_READ_BUFFER);
}




/**
 * \brief Application entry point.
 *
 * \return Unused (ANSI-C compatibility).
 */
int _main_coms_adc_test(void)
{
	system_init();
	cdc_uart_init();
	configure_adc();
	delay_init();

	

	/* Configure analog pins */
	struct system_pinmux_config config;
	system_pinmux_get_config_defaults(&config);

	/* Analog functions are all on MUX setting B */
	config.input_pull   = SYSTEM_PINMUX_PIN_PULL_NONE;
	config.mux_position = 1;

	system_pinmux_pin_set_config(PIN_PB08, &config);

	printf("ADC AND RS485 TEST\n\r");
	printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
	
	while (1) 
	{
		adc_read_buffer_job(&adc_instance, adc_buffer, NUM_SAMP);
		delay_ms(100);
	}
}
