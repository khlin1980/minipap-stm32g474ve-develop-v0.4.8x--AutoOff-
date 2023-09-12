
/**========================================================================
 *                           includes
 *========================================================================**/
#include "color_led.h"


/**========================================================================
 *                           define
 *========================================================================**/
  #define COLOR_PWM_DUTY_MAX	(255u)

//* (0u) normal:  active(HI), inactive(LO)
//* (1u) inverse: active(LO), inactive(HI)
#define PWM_INVERSE 	(1u)
 #if(PWM_INVERSE == 0)
	#define COLOR_V(x)  (uint8_t)(x)
 #else
 	#define COLOR_V(x)  (uint8_t)(255 - x)
 #endif	

#define POLLING_BASETIME            (uint32_t)(1)   					 // ms
#define SET_BASETIME_CNT_ms(x)      (uint32_t)(x / POLLING_BASETIME)     // ms    


/**========================================================================
 *                           typedef
 *========================================================================**/
typedef struct{
    TIM_HandleTypeDef *htim;
    uint32_t Channel;
    
}led_pwm_handler_t;


typedef struct{
    bool en;
    bool blink_en;
    bool blink_inv;
    uint32_t blink_time;
    uint32_t blink_time_cnt;
    uint32_t prev_blink_time;
    
    led_pwm_handler_t pwm_handler[3];

}rgb_led_t;


/**========================================================================
 *                           macro
 *========================================================================**/


/**========================================================================
 *                           variables
 *========================================================================**/
static rgb_led_t rgb_leds[NUM_OF_RGB_LEDs] = {0};


/**========================================================================
 *                           extern variables
 *========================================================================**/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;


/**========================================================================
 *                           functions
 *========================================================================**/


/**
 * @brief 
 * 
 */
void color_led_init(void) 
{
	//* the led pwm cycle is => 170/30/255 = 22.2Khz
	
	//* hooks the rgb leds pwm handler
	rgb_leds[RGB_PWR].pwm_handler[LED_R].htim 	 = &htim4;
	rgb_leds[RGB_PWR].pwm_handler[LED_R].Channel = TIM_CHANNEL_1;
	rgb_leds[RGB_PWR].pwm_handler[LED_G].htim 	 = &htim4;
	rgb_leds[RGB_PWR].pwm_handler[LED_G].Channel = TIM_CHANNEL_2;
	rgb_leds[RGB_PWR].pwm_handler[LED_B].htim 	 = &htim4;
	rgb_leds[RGB_PWR].pwm_handler[LED_B].Channel = TIM_CHANNEL_3;

	rgb_leds[RGB_BLE].pwm_handler[LED_R].htim 	= &htim2;
	rgb_leds[RGB_BLE].pwm_handler[LED_R].Channel  = TIM_CHANNEL_1;
	rgb_leds[RGB_BLE].pwm_handler[LED_G].htim 	 = &htim2;
	rgb_leds[RGB_BLE].pwm_handler[LED_G].Channel = TIM_CHANNEL_2;
	rgb_leds[RGB_BLE].pwm_handler[LED_B].htim 	 = &htim2;
	rgb_leds[RGB_BLE].pwm_handler[LED_B].Channel = TIM_CHANNEL_3;

	for(uint8_t i=0; i<NUM_OF_RGB_LEDs; i++){
		color_led_turn_off(i);
		color_led_set_blink_disable(i);
	}
}


/**
 * @brief 
 * 
 */
void color_led_turn_off(uint8_t num)
{
	rgb_leds[num].en = false;

	HAL_TIM_PWM_Stop(rgb_leds[num].pwm_handler[LED_R].htim, rgb_leds[num].pwm_handler[LED_R].Channel);
	HAL_TIM_PWM_Stop(rgb_leds[num].pwm_handler[LED_G].htim, rgb_leds[num].pwm_handler[LED_G].Channel);
	HAL_TIM_PWM_Stop(rgb_leds[num].pwm_handler[LED_B].htim, rgb_leds[num].pwm_handler[LED_B].Channel);
}


/**
 * @brief 
 * 
 */
void color_led_turn_on(uint8_t num)
{
	rgb_leds[num].en = true;

	HAL_TIM_PWM_Start(rgb_leds[num].pwm_handler[LED_R].htim, rgb_leds[num].pwm_handler[LED_R].Channel);
	HAL_TIM_PWM_Start(rgb_leds[num].pwm_handler[LED_G].htim, rgb_leds[num].pwm_handler[LED_G].Channel);
	HAL_TIM_PWM_Start(rgb_leds[num].pwm_handler[LED_B].htim, rgb_leds[num].pwm_handler[LED_B].Channel);
}


/**
 * @brief 
 * 
 */
void color_led_set_value(uint8_t num, uint8_t r, uint8_t g, uint8_t b)
{	
	if(rgb_leds[num].en == false) 
		color_led_turn_on(num);
	
	//* update duty
	__HAL_TIM_SET_COMPARE(rgb_leds[num].pwm_handler[LED_R].htim, rgb_leds[num].pwm_handler[LED_R].Channel, COLOR_V(r));
	__HAL_TIM_SET_COMPARE(rgb_leds[num].pwm_handler[LED_G].htim, rgb_leds[num].pwm_handler[LED_G].Channel, COLOR_V(g));
	__HAL_TIM_SET_COMPARE(rgb_leds[num].pwm_handler[LED_B].htim, rgb_leds[num].pwm_handler[LED_B].Channel, COLOR_V(b));		
}


/**
 * @brief 
 * 
 */
void color_led_set_rgb(uint8_t num, uint32_t rgb)
{
	uint8_t r, g, b;
	
	r = (uint8_t)(rgb >> 16);			
	g = (uint8_t)(rgb >> 8);	     
	b = (uint8_t)rgb;				

	color_led_set_value(num, r, g, b);
}


/**
 * @brief 
 * 
 */
void color_led_single_set (uint8_t num, uint8_t led, uint8_t brightness)
{
	if(rgb_leds[num].en == false) 
		color_led_turn_on(num);
	
	//* just turn a single "led" on (specify), others will be turn off
	switch(led)
	{
		case LED_R:	color_led_set_value(num, brightness, 0, 0);	break;
		case LED_G:	color_led_set_value(num, 0, brightness, 0);	break;
		case LED_B:	color_led_set_value(num, 0, 0, brightness);	break;
	}
}


/**
 * @brief 
 * 
 */
void color_led_set_blink_disable (uint8_t num)
{
	if(rgb_leds[num].blink_en)
	{
		rgb_leds[num].blink_en = false;
		rgb_leds[num].blink_inv = false;
		rgb_leds[num].blink_time_cnt = 0;
		rgb_leds[num].blink_time = rgb_leds[num].prev_blink_time = 0;
		
		color_led_turn_on(num);
	}
}


/**
 * @brief 
 * 
 */
void color_led_set_blink_enable (uint8_t num, uint32_t blink_ms)
{
	if(blink_ms == 0)
	{
		color_led_set_blink_disable(num);
	}
	else
	{
		//* enable blink
		rgb_leds[num].blink_en = true;
		
		if(rgb_leds[num].prev_blink_time != SET_BASETIME_CNT_ms(blink_ms))
		{
			rgb_leds[num].blink_inv = false;
			rgb_leds[num].blink_time_cnt = 0;
			rgb_leds[num].blink_time = rgb_leds[num].prev_blink_time = SET_BASETIME_CNT_ms(blink_ms);
		}
	}
}


/**
 * @brief 
 * 
 */
void color_led_blink_polling (void)
{
	//* put this to some timer tick int routine to handle it 

	for(uint8_t i=0; i<NUM_OF_RGB_LEDs; i++){
	
		if(rgb_leds[i].blink_en){
			
			if(++rgb_leds[i].blink_time_cnt >= rgb_leds[i].blink_time){
				rgb_leds[i].blink_time_cnt = 0;
				
				rgb_leds[i].blink_inv ^= 1;

				if(rgb_leds[i].blink_inv){
					color_led_turn_off(i);
				}else{
					color_led_turn_on(i);
				}
			}
		}
		else{
			rgb_leds[i].blink_time_cnt = 0;
		}
	}
}


/*********************************************END OF FILE**********************/
