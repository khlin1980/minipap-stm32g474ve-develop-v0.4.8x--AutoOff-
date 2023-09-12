/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

#include "motorcontrol.h"

#include "stm32g4xx_ll_ucpd.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_dma.h"

#include "stm32g4xx_ll_exti.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void user_timer_tick_inc (void);
uint32_t get_user_timer_tick (void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BLE_RST_Pin GPIO_PIN_2
#define BLE_RST_GPIO_Port GPIOE
#define BLE_EN_Pin GPIO_PIN_3
#define BLE_EN_GPIO_Port GPIOE
#define LED_RAMP_Pin GPIO_PIN_5
#define LED_RAMP_GPIO_Port GPIOE
#define M1_BUS_VOLTAGE_Pin GPIO_PIN_0
#define M1_BUS_VOLTAGE_GPIO_Port GPIOA
#define M1_CURR_SHUNT_U_Pin GPIO_PIN_1
#define M1_CURR_SHUNT_U_GPIO_Port GPIOA
#define M1_OPAMP1_OUT_Pin GPIO_PIN_2
#define M1_OPAMP1_OUT_GPIO_Port GPIOA
#define M1_OPAMP1_INT_GAIN_Pin GPIO_PIN_3
#define M1_OPAMP1_INT_GAIN_GPIO_Port GPIOA
#define M1_OPAMP2_INT_GAIN_Pin GPIO_PIN_5
#define M1_OPAMP2_INT_GAIN_GPIO_Port GPIOA
#define M1_OPAMP2_OUT_Pin GPIO_PIN_6
#define M1_OPAMP2_OUT_GPIO_Port GPIOA
#define M1_CURR_SHUNT_V_Pin GPIO_PIN_7
#define M1_CURR_SHUNT_V_GPIO_Port GPIOA
#define M1_CURR_SHUNT_W_Pin GPIO_PIN_0
#define M1_CURR_SHUNT_W_GPIO_Port GPIOB
#define M1_PWM_UL_Pin GPIO_PIN_8
#define M1_PWM_UL_GPIO_Port GPIOE
#define M1_PWM_UH_Pin GPIO_PIN_9
#define M1_PWM_UH_GPIO_Port GPIOE
#define M1_PWM_VL_Pin GPIO_PIN_10
#define M1_PWM_VL_GPIO_Port GPIOE
#define M1_PWM_VH_Pin GPIO_PIN_11
#define M1_PWM_VH_GPIO_Port GPIOE
#define M1_PWM_WL_Pin GPIO_PIN_12
#define M1_PWM_WL_GPIO_Port GPIOE
#define M1_PWM_WH_Pin GPIO_PIN_13
#define M1_PWM_WH_GPIO_Port GPIOE
#define KEY_BLE_Pin GPIO_PIN_12
#define KEY_BLE_GPIO_Port GPIOB
#define KEY_PWR_Pin GPIO_PIN_13
#define KEY_PWR_GPIO_Port GPIOB
#define MT_TEMPERATURE_Pin GPIO_PIN_14
#define MT_TEMPERATURE_GPIO_Port GPIOB
#define GP_40DET_Pin GPIO_PIN_8
#define GP_40DET_GPIO_Port GPIOD
#define DP_IRQ_Pin GPIO_PIN_9
#define DP_IRQ_GPIO_Port GPIOD
#define SENSOR_EN_Pin GPIO_PIN_10
#define SENSOR_EN_GPIO_Port GPIOD
#define LED_PWR_R_Pin GPIO_PIN_12
#define LED_PWR_R_GPIO_Port GPIOD
#define LED_PWR_G_Pin GPIO_PIN_13
#define LED_PWR_G_GPIO_Port GPIOD
#define LED_PWR_B_Pin GPIO_PIN_14
#define LED_PWR_B_GPIO_Port GPIOD
#define I2C4_SCL_SENSOR_Pin GPIO_PIN_6
#define I2C4_SCL_SENSOR_GPIO_Port GPIOC
#define I2C4_SDA_SENSOR_Pin GPIO_PIN_7
#define I2C4_SDA_SENSOR_GPIO_Port GPIOC
#define I2C3_SCL_UCPD_Pin GPIO_PIN_8
#define I2C3_SCL_UCPD_GPIO_Port GPIOC
#define I2C3_SDA_UCPD_Pin GPIO_PIN_9
#define I2C3_SDA_UCPD_GPIO_Port GPIOC
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define LED_BLE_R_Pin GPIO_PIN_15
#define LED_BLE_R_GPIO_Port GPIOA
#define PD_PW_GOOD_Pin GPIO_PIN_10
#define PD_PW_GOOD_GPIO_Port GPIOC
#define LED_BLE_G_Pin GPIO_PIN_4
#define LED_BLE_G_GPIO_Port GPIOD
#define UART2_TX_MT_Pin GPIO_PIN_5
#define UART2_TX_MT_GPIO_Port GPIOD
#define UART2_RX_MT_Pin GPIO_PIN_6
#define UART2_RX_MT_GPIO_Port GPIOD
#define LED_BLE_B_Pin GPIO_PIN_7
#define LED_BLE_B_GPIO_Port GPIOD
#define KEY_RAMP_Pin GPIO_PIN_7
#define KEY_RAMP_GPIO_Port GPIOB
#define UART1_TX_BLE_Pin GPIO_PIN_0
#define UART1_TX_BLE_GPIO_Port GPIOE
#define UART1_RX_BLE_Pin GPIO_PIN_1
#define UART1_RX_BLE_GPIO_Port GPIOE
void   MX_ADC1_Init(void);
void   MX_ADC2_Init(void);
void   MX_COMP1_Init(void);
void   MX_COMP2_Init(void);
void   MX_COMP4_Init(void);
void   MX_CORDIC_Init(void);
void   MX_OPAMP1_Init(void);
void   MX_OPAMP2_Init(void);
void   MX_TIM1_Init(void);
void   MX_USART2_UART_Init(void);
void   MX_I2C3_Init(void);
void   MX_I2C4_Init(void);
void   MX_QUADSPI1_Init(void);
void   MX_RTC_Init(void);
void   MX_TIM2_Init(void);
void   MX_TIM4_Init(void);
void   MX_UCPD1_Init(void);
void   MX_USART1_UART_Init(void);
void   MX_TIM7_Init(void);
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
