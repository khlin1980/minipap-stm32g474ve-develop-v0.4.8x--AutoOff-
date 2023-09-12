/**========================================================================
 *                           includes
 *========================================================================**/
#include "led.h"


/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
#define LED_NUM	        1	   						               

//* enable the led port clock 
#define ALL_LED_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOE_CLK_ENABLE();	//\
                                    //__HAL_RCC_GPIOC_CLK_ENABLE();	


/**========================================================================
 *                           typedef
 *========================================================================**/
enum
{
  LED_OFF = 0,
  LED_ON,
  LED_BLINK,
};


typedef struct
{
  uint8_t type;  
  uint8_t en;
  struct
  {
    uint8_t invert;
    uint16_t cnt;
    uint16_t time;
  }blink;
  
}LED_t;


typedef struct
{
  GPIO_TypeDef* gpio;
  uint16_t pin;
  uint8_t ActiveLevel;
}X_GPIO_T;


/**========================================================================
 *                           define
 *========================================================================**/


/**========================================================================
 *                           macro
 *========================================================================**/


/**========================================================================
 *                           variables
 *========================================================================**/
LED_t leds[LED_NUM] = {0};
static X_GPIO_T leds_gpio_list[LED_NUM] = {
  {GPIOE, GPIO_PIN_5, 0},		/* L1 - Ramp */
  //{GPIOB, GPIO_PIN_14, 0},	  /* L2 -  */
  // {GPIOC, GPIO_PIN_8,  0},	  /* L3 -  */
  // {GPIOC, GPIO_PIN_7,  0},	  /* L4 -  */
  // {GPIOC, GPIO_PIN_6,  0},	  /* L5 -  */
};	


/**========================================================================
 *                           functions
 *========================================================================**/
static void led_var_init (void);
static void led_hard_init (void);
static void led_on(uint8_t led);
static void led_off(uint8_t led);


static void led_var_init (void)
{
  for(uint8_t i=0; i<LED_NUM; i++)
  {
    leds[i].type = LED_OFF;
    leds[i].blink.cnt = 0;
    leds[i].blink.time = 0;
    leds[i].blink.invert = 0;
  }
}


static void led_hard_init (void)
{
  GPIO_InitTypeDef gpio_init = {0};
  uint8_t i;

  /* 第1步：打開GPIO時鐘 */
  ALL_LED_GPIO_CLK_ENABLE();
  
  /* 第2步：配置所有的按鍵GPIO為輸出模式 */
  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;   			  
  gpio_init.Pull = GPIO_NOPULL;                 
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;       
  
  for (i = 0; i < LED_NUM; i++)
  {
    gpio_init.Pin = leds_gpio_list[i].pin;
    HAL_GPIO_Init(leds_gpio_list[i].gpio, &gpio_init);	
  }
}


void led_init (void)
{
  led_var_init();
  led_hard_init();

  for (uint8_t i = 0; i < LED_NUM; i++)
    led_set_off(i);
}


static void led_on(uint8_t led)
{
  if(led >= LED_NUM)  return;

  leds[led].en = 1;
  
  if(leds_gpio_list[led].ActiveLevel)
    leds_gpio_list[led].gpio->BSRR = leds_gpio_list[led].pin;
  else
    leds_gpio_list[led].gpio->BSRR = (uint32_t)leds_gpio_list[led].pin << 16;
}


static void led_off(uint8_t led)
{
  if(led >= LED_NUM)  return;

  leds[led].en = 0;
  
  if(leds_gpio_list[led].ActiveLevel)
    leds_gpio_list[led].gpio->BSRR = (uint32_t)leds_gpio_list[led].pin << 16;
  else
    leds_gpio_list[led].gpio->BSRR = leds_gpio_list[led].pin;
}


void led_set_on(uint8_t led)
{
  if(led >= LED_NUM)  return;

  leds[led].type = LED_ON; 
  led_on(led);
}


void led_set_off(uint8_t led)
{
  if(led >= LED_NUM)  return;

  leds[led].type = LED_OFF;
  led_off(led);
}


void led_set_blink(uint8_t led, uint16_t blink_time)
{
  if(led >= LED_NUM)  return;
  
  leds[led].type = LED_BLINK;
    
  leds[led].blink.time = blink_time;
  leds[led].blink.cnt = 0;
  leds[led].blink.invert = 0;
  led_on(led);
}


void led_control (void)
{
  //* polling per 10ms
  for(uint8_t i=0; i<LED_NUM; i++)
  { 
    if(leds[i].type == LED_BLINK)
    {
      //* counting the blink time
      if(++leds[i].blink.cnt >= leds[i].blink.time)
      {
        leds[i].blink.cnt = 0;
        leds[i].blink.invert ^= 1;
      }

      if(leds[i].blink.invert)
        led_off(i);
      else
        led_on(i);
    }
  }
}


uint8_t led_get_status (uint8_t led)
{
  return leds[led].en; 
}