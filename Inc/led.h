#ifndef LED_H_
#define LED_H_
#include <stdint.h>

#define LED_GREEN               12
#define LED_ORANGE              13
#define LED_RED                 14
#define LED_BLUE                15

#define DELAY_COUNT_1S  		1000U
#define DELAY_COUNT_500MS  		500U
#define DELAY_COUNT_250MS 		250U
#define DELAY_COUNT_125MS 		125U

void led_init_all(void);
void led_on(uint8_t led_no);
void led_off(uint8_t led_no);

#endif
