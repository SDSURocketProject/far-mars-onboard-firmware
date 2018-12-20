/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>

/**
 * @brief		Entry point for the program.
 * @return		never returns if working properly.
 */
int main (void)
{
	system_init();

	/* Insert application code here, after the board has been initialized. */
}

/**
 * @brief		Pauses program if a stack overflow is detected.
 * @param[in]	xTask contains the hook of the task that overflowed
 * @param[in]	*pcTaskName contains the name of the task that overflowed
 * @return		none.
 */
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ) {
	while(1) {
		configASSERT(0);
	}
}
