#ifndef _KEY_FIFO_H_
#define _KEY_FIFO_H_

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
   * Includes
*******************************************************************************/
#include <stdint.h>
#include "stm32g4xx_hal.h"
  
/******************************************************************************
* Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Configuration Constants
*******************************************************************************/
#define KEY_POLLING_BASE_TIME   10  // 10ms 
#define KEY_FILTER_TIME         (20 / KEY_POLLING_BASE_TIME)    /* 按鍵 debounce time */
#define KEY_LONG_TIME           (1000 / KEY_POLLING_BASE_TIME)  /* 按鍵長按時間 */

/******************************************************************************
* Macros
*******************************************************************************/
#define KEY_TIME_MS(x)          (uint16_t)(x / KEY_POLLING_BASE_TIME)  

/******************************************************************************
* Typedefs
*******************************************************************************/
/* 按鍵ID(實體 + combo), 主要用於 KeyState()函数的入口参数 */
typedef enum
{
   /* 實體鍵 */
   KID_K1 = 0,
   KID_K2,
   KID_K3,
   
   /* 組合鍵 */
   KID_X1,
} KEY_ID_E;

//* key event code (實體 + combo) 
typedef enum
{
   KEY_NONE = 0,			/* 0 表示按鍵事件 */

   /* 實體鍵 */
   KEY_1_DOWN,				/* 1鍵按下 */
   KEY_1_UP,				/* 1鍵彈起 */
   KEY_1_LONG,				/* 1鍵長按 */

   KEY_2_DOWN,				/* 2鍵按下 */
   KEY_2_UP,				/* 2鍵彈起 */
   KEY_2_LONG,				/* 2鍵長按 */

   KEY_3_DOWN,				/* 3鍵按下 */
   KEY_3_UP,				/* 3鍵彈起 */
   KEY_3_LONG,				/* 3鍵長按 */

   /* 組合鍵 */
   KEY_X1_DOWN,		   /* 9鍵按下 */
   KEY_X1_UP,				/* 9鍵彈起 */
   KEY_X1_LONG,			/* 9鍵長按 */
}KEY_ENUM;

/* 根據應用定義按鍵  */
#define KEY_DOWN_PWR       KEY_1_DOWN
#define KEY_UP_PWR         KEY_1_UP
#define KEY_LONG_PWR       KEY_1_LONG

#define KEY_DOWN_RAMP      KEY_2_DOWN
#define KEY_UP_RAMP        KEY_2_UP
#define KEY_LONG_RAMP      KEY_2_LONG

#define KEY_DOWN_BLE       KEY_3_DOWN
#define KEY_UP_BLE         KEY_3_UP
#define KEY_LONG_BLE       KEY_3_LONG

#define KEY_DOWM_K2K3      KEY_X1_DOWN
#define KEY_UP_K2K3        KEY_X1_UP
#define KEY_LONG_K2K3      KEY_X1_LONG


/* key structure */
typedef struct
{
   //uint8_t (*IsKeyDownFunc)(void);   /* 函式指針, 指向判斷按鍵是否按下的函式, 1表示按下 */
   uint8_t  Count;			            /* 濾波器計數器 */
   uint16_t LongCount;		            /* 長按計數器 */
   uint16_t LongTime;		            /* 按鍵按下持續時間, 0表示不檢測長按 */
   uint8_t  State;			            /* 按鍵目前狀態（按下還是彈起） */
   uint8_t  RepeatSpeed;	            /* 連續按鍵週期 */
   uint8_t  RepeatCount;	            /* 連續按鍵計數器 */
}KEY_T;

/* key fifo buffer structure */
#define KEY_FIFO_SIZE	10
typedef struct
{
   uint8_t Buf[KEY_FIFO_SIZE];		   /* 鍵值緩衝區 */
   uint8_t Read;					        /* 緩衝區讀指針1 */
   uint8_t Write;					        /* 緩衝區寫指針 */
   uint8_t Read2;					        /* 緩衝區讀指針2 */
}KEY_FIFO_T;

/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/
void KeyInit(void);
uint8_t GetKey(void);
uint8_t GetKey2(void);
uint8_t GetKeyState(KEY_ID_E keyID);
void ClearKey(void);
void KeyScan10ms(void);
void SetPingKeyLocked(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /*File_H_*/

/*** End of File **************************************************************/