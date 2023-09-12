#ifndef __COLOR_LED_H
#define __COLOR_LED_H

#ifdef __cplusplus
 extern "C" {
#endif

/**========================================================================
 *                           includes
 *========================================================================**/
#include "bsp.h"

/**========================================================================
 *                           exported types
 *========================================================================**/
typedef enum{
    LED_R = 0,
    LED_G,
    LED_B,

}RGB_ELEMENT_t;


//* how many RGB leds are used 
typedef enum{
    RGB_PWR = 0,        /* power leds */
    RGB_BLE,            /* BLE leds */
    NUM_OF_RGB_LEDs,

}RGB_LEDs_DEFINE_t;


/**========================================================================
 *                           exported constants
 *========================================================================**/
#define COLOR_WHITE          0xFFFFFF
#define COLOR_BLACK          0x000000
#define COLOR_GREY           0xC0C0C0
#define COLOR_BLUE           0x0000FF
#define COLOR_RED            0xFF0000
#define COLOR_MAGENTA        0xFF00FF
#define COLOR_GREEN          0x00FF00
#define COLOR_CYAN           0x00FFFF
#define COLOR_YELLOW         0xFFFF00


/**========================================================================
 *                           exported macro
 *========================================================================**/

/**========================================================================
 *                           exported variables
 *========================================================================**/

/**========================================================================
 *                           exported functions
 *========================================================================**/
void color_led_init(void);
void color_led_turn_off(uint8_t num);
void color_led_turn_on(uint8_t num);
void color_led_set_value(uint8_t num, uint8_t r, uint8_t g, uint8_t b);
void color_led_set_rgb(uint8_t num, uint32_t rgb);

void color_led_single_set (uint8_t num, uint8_t led, uint8_t brightness);
void color_led_set_blink_disable (uint8_t num);
void color_led_set_blink_enable (uint8_t num, uint32_t blink_ms);
void color_led_blink_polling (void);


#ifdef __cplusplus
}
#endif

#endif