/*
 * _main_test.c
 *
 * Created: 3/27/2018 5:22:51 PM
 *  Author: Jake Carver
 */ 

 #include <asf.h>
 #include "debugger_serial_config.h"
 #include <string.h>


 #define CONF_PWM_MODULE      TCC2
 #define CONF_PWM_CHANNEL     1
 #define CONF_PWM_OUTPUT      1
 #define CONF_PWM_OUT_PIN     PIN_PA01
 #define CONF_PWM_OUT_MUX     PINMUX_PA01E_TCC2_WO1

 #define PWM_PERIOD 1500
 #define PWM_MIN_VAL PWM_PERIOD/2

 struct tcc_module tcc_instance;

 static void configure_tcc(void)
 {
	 struct tcc_config config_tcc;
	 tcc_get_config_defaults(&config_tcc, CONF_PWM_MODULE);
	 config_tcc.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1;
	 config_tcc.counter.period = PWM_PERIOD;
	 config_tcc.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
	 config_tcc.compare.match[CONF_PWM_CHANNEL] = 0;
	 config_tcc.pins.enable_wave_out_pin[CONF_PWM_OUTPUT] = true;
	 config_tcc.pins.wave_out_pin[CONF_PWM_OUTPUT]        = CONF_PWM_OUT_PIN;
	 config_tcc.pins.wave_out_pin_mux[CONF_PWM_OUTPUT]    = CONF_PWM_OUT_MUX;
	 tcc_init(&tcc_instance, CONF_PWM_MODULE, &config_tcc);
	 tcc_enable(&tcc_instance);
 }

 int _main_led_test(void)
 {
	configure_tcc();
	delay_init();

	while(1)
	{
		for(int i = PWM_MIN_VAL; i < PWM_PERIOD; i++)
		{
			delay_ms(1);
			tcc_set_compare_value(&tcc_instance, 1, i);
		}
		
		for(int i = PWM_PERIOD; i > PWM_MIN_VAL; i--)
		{
			delay_ms(1);
			tcc_set_compare_value(&tcc_instance, 1, i);
		}
	}

 }