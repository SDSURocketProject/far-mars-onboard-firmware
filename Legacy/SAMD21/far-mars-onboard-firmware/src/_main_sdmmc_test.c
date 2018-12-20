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

/**
 * \brief Application entry point.
 *
 * \return Unused (ANSI-C compatibility).
 */
int _main_sdmmc_test(void)
{
	char test_file_name[] = "0:sd_mmc_test.txt";
	Ctrl_status status;
	FRESULT res;
	FATFS fs;
	FIL file_object;

	system_init();
	//delay_init();
	cdc_uart_init();

	/* Initialize SD MMC stack */
	sd_mmc_init();

	printf("\x0C\n\r-- SD/MMC Card Example on FatFs --\n\r");
	printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
	while (1) {
		printf("Please plug an SD/MMC card in slot.\n\r");

		/* Wait card present and ready */
		do {
			status = sd_mmc_test_unit_ready(0);
			if (CTRL_FAIL == status) {
				printf("Card install FAIL\n\r");
				printf("Please unplug and re-plug the card.\n\r");
				while (CTRL_NO_PRESENT != sd_mmc_check(0)) {
				}
			}
		} while (CTRL_GOOD != status);

		printf("Mount disk (f_mount)...\r\n");
		memset(&fs, 0, sizeof(FATFS));
		res = f_mount(LUN_ID_SD_MMC_0_MEM, &fs);
		if (FR_INVALID_DRIVE == res) {
			printf("[FAIL] res %d\r\n", res);
			goto main_end_of_test;
		}
		printf("[OK]\r\n");

		printf("Create a file (f_open)...\r\n");
		test_file_name[0] = LUN_ID_SD_MMC_0_MEM + '0';
		res = f_open(&file_object,
				(char const *)test_file_name,
				FA_CREATE_ALWAYS | FA_WRITE);
		if (res != FR_OK) {
			printf("[FAIL] res %d\r\n", res);
			goto main_end_of_test;
		}
		printf("[OK]\r\n");

		printf("Write to test file (f_puts)...\r\n");
		if (0 == f_puts("Test SD/MMC stack\n", &file_object)) {
			f_close(&file_object);
			printf("[FAIL]\r\n");
			goto main_end_of_test;
		}
		printf("[OK]\r\n");
		f_close(&file_object);
		printf("Test is successful.\n\r");

main_end_of_test:
		printf("Please unplug the card.\n\r");
		while (CTRL_NO_PRESENT != sd_mmc_check(0)) {
		}
	}
}
