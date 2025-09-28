#include "led.h"

void led_init_all(void)
{
	uint32_t *ptr_ahb1_clock_register = (uint32_t*)0x40023830;
	uint32_t *ptr_gpiod_mode_register = (uint32_t*)0x40020C00;
	
	*ptr_ahb1_clock_register |= ( 1 << 3);
	*ptr_gpiod_mode_register |= ( 1 << (2 * LED_GREEN));
	*ptr_gpiod_mode_register |= ( 1 << (2 * LED_ORANGE));
	*ptr_gpiod_mode_register |= ( 1 << (2 * LED_RED));
	*ptr_gpiod_mode_register |= ( 1 << (2 * LED_BLUE));
	
	led_off(LED_GREEN);
	led_off(LED_ORANGE);
	led_off(LED_RED);
	led_off(LED_BLUE);
}

void led_on(uint8_t led_no)
{
	uint32_t *ptr_gpiod_output_data_register = (uint32_t*)0x40020C14;
	*ptr_gpiod_output_data_register |= ( 1 << led_no);
}

void led_off(uint8_t led_no)
{
	uint32_t *ptr_gpiod_output_data_register = (uint32_t*)0x40020C14;
	*ptr_gpiod_output_data_register &= ~( 1 << led_no);
}

