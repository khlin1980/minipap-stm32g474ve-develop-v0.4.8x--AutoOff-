
/******************************************************************************
* Includes
*******************************************************************************/
#include "key_fifo.h"

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
#define HARD_KEY_NUM	        3	   						                /* 實體按鍵個數 */
#define COMBO_KEY_NUM	        1   						                /* 組合按鍵個數 */
#define KEY_COUNT   	 	      (HARD_KEY_NUM + COMBO_KEY_NUM)	/* 所有按鍵個數 */

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/
//* enable the key port clock 
#define ALL_KEY_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE();	
                                  	

/******************************************************************************
* Module Typedefs
*******************************************************************************/
typedef struct
{
  GPIO_TypeDef* gpio;
  uint16_t pin;
  uint8_t ActiveLevel;	// 按鍵觸發準位
}X_GPIO_T;

/* GPIO和PIN定義 */
static const X_GPIO_T s_gpio_list[HARD_KEY_NUM] = {
  {GPIOB, GPIO_PIN_13,  0},		/* K1 - Power */
  {GPIOB, GPIO_PIN_7,   1},	  /* K2 - Ramp */
  {GPIOB, GPIO_PIN_12,  0},	  /* K3 - BLE */
};	

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
static KEY_T s_tBtn[KEY_COUNT] = {0};
static KEY_FIFO_T s_tKey;		/* 按鍵FIFO變數,結構體 */
static uint8_t pingKeyLocked = 0;

/******************************************************************************
* Function Prototypes
*******************************************************************************/
static uint8_t KeyPinActive(uint8_t id);
static uint8_t IsKeyDownFunc(uint8_t id);
void KeyInit(void);
void KeyVarInit(void);
void KeyHardInit(void);
void PutKey(uint8_t keyCode);
uint8_t GetKey(void);
uint8_t GetKey2(void);
uint8_t GetKeyState(KEY_ID_E keyID);
void SetKeyParam(uint8_t keyID, uint16_t longTime, uint8_t  repeatSpeed);
void ClearKey(void);
void DetectKey(uint8_t i);
void KeyScan10ms(void);
static uint8_t IsAllKeyReleased(void);
void SetPingKeyLocked(void);


/*******************************************************************************************************
*	函 數 名: KeyPinActive
*	功能說明: 判斷按鍵是否按下
*	形    參: 無
*	返 回 值: 返回值1 表示按下(導通），0表示未按下（釋放）
*******************************************************************************************************/
static uint8_t KeyPinActive(uint8_t id)
{
  uint8_t level;
  
  if ((s_gpio_list[id].gpio->IDR & s_gpio_list[id].pin) == 0)
  {
    level = 0;
  }
  else
  {
    level = 1;
  }

  if (level == s_gpio_list[id].ActiveLevel)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/*******************************************************************************************************
*	函 數 名: IsKeyDownFunc
*	功能說明: 判斷按鍵是否按下。單鍵和組合鍵區分。
*	形    參: 無
*	返 回 值: 返回值1 表示按下(導通），0表示未按下（釋放）
*******************************************************************************************************/
static uint8_t IsKeyDownFunc(uint8_t id)
{
  /* 實體單鍵 */
  if (id < HARD_KEY_NUM)
  {    
    /* 判斷按鍵按下 */
    if (KeyPinActive(id)) 
    {
      return 1;
    }	
    else
    {
      return 0;
    }
  }
  
  /* 組合鍵1 - K2K3 */
  if (id == HARD_KEY_NUM + 0)
  {
    if (KeyPinActive(KID_K2) && KeyPinActive(KID_K3))
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }

  // /* 組合鍵2 */
  // if (id == HARD_KEY_NUM + 1)
  // {
  // }

  return 0;
}

/*******************************************************************************************************
*	函 數 名: KeyInit
*	功能說明: 初始化按鍵. 
*	形    參:  無
*	返 回 值: 無
*******************************************************************************************************/
void KeyInit(void)
{
  KeyVarInit();		
  KeyHardInit();		
}

/*******************************************************************************************************
*	函 數 名: KeyVarInit
*	功能說明: 初始化按鍵變數
*	形    參:  無
*	返 回 值: 無
*******************************************************************************************************/
void KeyVarInit(void)
{
  uint8_t i;

  /* 對按鍵FIFO讀寫指針清零 */
  s_tKey.Read = 0;
  s_tKey.Write = 0;
  s_tKey.Read2 = 0;

  /* 給每個按鍵結構體成員變數賦一組預設值 */
  for (i = 0; i < KEY_COUNT; i++)
  {
    s_tBtn[i].LongTime = KEY_LONG_TIME;			  /* 長按時間 0 表示不檢測長按鍵事件 */
    s_tBtn[i].Count = KEY_FILTER_TIME / 2;		/* 計數器設定為濾波時間的一半 */
    s_tBtn[i].State = 0;							        /* 按鍵預設狀態，0為未按下 */
    s_tBtn[i].RepeatSpeed = 0;						    /* 按鍵連發的速度，0表示不支援連發 */
    s_tBtn[i].RepeatCount = 0;						    /* 連發計數器 */
  }

  /* 如果需要單獨更改某個按鍵的參數，可以在此單獨重新賦值 */
  SetKeyParam(KID_K1, KEY_TIME_MS(1000), 0);
  SetKeyParam(KID_K2, KEY_TIME_MS(1000), 0);
  SetKeyParam(KID_K3, KEY_TIME_MS(1000), 0);
  SetKeyParam(KID_X1, KEY_TIME_MS(2000), 0);
}

/*******************************************************************************************************
*	函 數 名: KeyHardInit
*	功能說明: 配置按鍵對應的GPIO
*	形    參:  無
*	返 回 值: 無
*******************************************************************************************************/
void KeyHardInit(void)
{	
  GPIO_InitTypeDef gpio_init = {0};
  uint8_t i;

  /* 第1步：打開GPIO時鐘 */
  ALL_KEY_GPIO_CLK_ENABLE();
  
  /* 第2步：配置所有的按鍵GPIO為浮動輸入模式(實際上CPU復位后就是輸入狀態) */
  gpio_init.Mode = GPIO_MODE_INPUT;   			    /* 設定輸入 */
  gpio_init.Pull = GPIO_NOPULL;                 /* 上下拉電阻不使能 */
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;        /* GPIO速度等級 */
  
  for (i = 0; i < HARD_KEY_NUM; i++)
  {
    gpio_init.Pin = s_gpio_list[i].pin;
    HAL_GPIO_Init(s_gpio_list[i].gpio, &gpio_init);	
  }
}

/*******************************************************************************************************
*	函 數 名: PutKey
*	功能說明: 將1個鍵值壓入按鍵FIFO緩衝區。可用於模擬一個按鍵。
*	形    參:  KeyCode : 按鍵程式碼
*	返 回 值: 無
*******************************************************************************************************/
void PutKey(uint8_t keyCode)
{
  s_tKey.Buf[s_tKey.Write] = keyCode;

  if (++s_tKey.Write >= KEY_FIFO_SIZE)
  {
    s_tKey.Write = 0;
  }
}

/*******************************************************************************************************
*	函 數 名: GetKey
*	功能說明: 從按鍵FIFO緩衝區讀取一個鍵值。
*	形    參: 無
*	返 回 值: 按鍵程式碼
*******************************************************************************************************/
uint8_t GetKey(void)
{
  uint8_t ret;

  if (s_tKey.Read == s_tKey.Write)
  {
    return KEY_NONE;
  }
  else
  {
    ret = s_tKey.Buf[s_tKey.Read];

    if (++s_tKey.Read >= KEY_FIFO_SIZE)
    {
      s_tKey.Read = 0;
    }
    return ret;
  }
}

/*******************************************************************************************************
*	函 數 名: GetKey2
*	功能說明: 從按鍵FIFO緩衝區讀取一個鍵值。獨立的讀指針。
*	形    參:  無
*	返 回 值: 按鍵程式碼
*******************************************************************************************************/
uint8_t GetKey2(void)
{
  uint8_t ret;

  if (s_tKey.Read2 == s_tKey.Write)
  {
    return KEY_NONE;
  }
  else
  {
    ret = s_tKey.Buf[s_tKey.Read2];

    if (++s_tKey.Read2 >= KEY_FIFO_SIZE)
    {
      s_tKey.Read2 = 0;
    }
    return ret;
  }
}

/*******************************************************************************************************
*	函 數 名: GetKeyState
*	功能說明: 讀取按鍵的狀態
*	形    參:  KeyID : 按鍵ID，從0開始
*	返 回 值: 1 表示按下， 0 表示未按下
*******************************************************************************************************/
uint8_t GetKeyState(KEY_ID_E keyID)
{
  return s_tBtn[keyID].State;
}

/*******************************************************************************************************
*	函 數 名: SetKeyParam
*	功能說明: 設定按鍵參數
*	形    參：keyID : 按鍵ID,從0開始
*			     longTime : 長按事件時間
*		       repeatSpeed : 連發速度
*	返 回 值: 無
*******************************************************************************************************/
void SetKeyParam(uint8_t keyID, uint16_t longTime, uint8_t  repeatSpeed)
{
  s_tBtn[keyID].LongTime = longTime;			    /* 長按時間 0 表示不檢測長按鍵事件 */
  s_tBtn[keyID].RepeatSpeed = repeatSpeed;	  /* 按鍵連發的速度，0表示不支援連發 */
  s_tBtn[keyID].RepeatCount = 0;						  /* 連發計數器 */
}

/*******************************************************************************************************
*	函 數 名: ClearKey
*	功能說明: 清空按鍵FIFO緩衝區
*	形    參：無
*	返 回 值: 按鍵程式碼
*******************************************************************************************************/
void ClearKey(void)
{
  s_tKey.Read = s_tKey.Write;
}

/*******************************************************************************************************
*	函 數 名: DetectKey
*	功能說明: 檢測一個按鍵。非阻塞狀態，必須被週期性的呼叫。
*	形    參: IO的id， 從0開始編碼
*	返 回 值: 無
*******************************************************************************************************/
void DetectKey(uint8_t i)
{
  KEY_T *pBtn;

  pBtn = &s_tBtn[i];
  if (IsKeyDownFunc(i))
  {
    if (pBtn->Count < KEY_FILTER_TIME)
    {
      pBtn->Count = KEY_FILTER_TIME;
    }
    else if(pBtn->Count < 2 * KEY_FILTER_TIME)
    {
      pBtn->Count++;
    }
    else
    {
      if (pBtn->State == 0)
      {
        pBtn->State = 1;

        /* 發送按鈕按下的訊息 */
        PutKey((uint8_t)(3 * i + 1));
      }

      if (pBtn->LongTime > 0)
      {
        if (pBtn->LongCount < pBtn->LongTime)
        {
          /* 發送按鈕持續按下的訊息 */
          if (++pBtn->LongCount == pBtn->LongTime)
          {
            /* 鍵值放入按鍵FIFO */
            PutKey((uint8_t)(3 * i + 3));
          }
        }
        else
        {
          if (pBtn->RepeatSpeed > 0)
          {
            if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
            {
              pBtn->RepeatCount = 0;
              /* 長按後，每隔10ms發送1個按鍵 */
              PutKey((uint8_t)(3 * i + 1));
            }
          }
        }
      }
    }
  }
  else
  {
    if(pBtn->Count > KEY_FILTER_TIME)
    {
      pBtn->Count = KEY_FILTER_TIME;
    }
    else if(pBtn->Count != 0)
    {
      pBtn->Count--;
    }
    else
    {
      if (pBtn->State == 1)
      {
        pBtn->State = 0;

        /* 發送按鈕彈起的訊息 */
        PutKey((uint8_t)(3 * i + 2));
      }
    }

    pBtn->LongCount = 0;
    pBtn->RepeatCount = 0;
  }
}

/********************************************************************************************************
*	函 數 名: KeyScan10ms
*	功能說明: 掃瞄所有按鍵。非阻塞，被systick中斷週期性的呼叫，10ms一次
*	形    參: 無
*	返 回 值: 無
********************************************************************************************************/
void KeyScan10ms(void)
{
  uint8_t i;
 
  for (i = 0; i < KEY_COUNT; i++)
    DetectKey(i);

  // if pin key is locked, detect all keys released then unlock it 
  if(pingKeyLocked)
  {
    // when pinkey is locked, clear key event 
    ClearKey();
    
    if(IsAllKeyReleased()){
      pingKeyLocked = 0;
    }
  }
}

/*******************************************************************************************************
*	函 數 名: IsAllKeyReleased
*	功能說明: 判斷按鍵是否全部釋放 (未按壓狀態)
*	形    參: 無
*	返 回 值: 返回值1 表示按下(釋放），0表示未按下（未釋放）
*******************************************************************************************************/
static uint8_t IsAllKeyReleased(void)
{
  /* 實體單鍵 */
  uint8_t i;
  uint8_t count = 0;
  
  /* 判斷有幾個實體鍵釋放 */
  for (i = 0; i < HARD_KEY_NUM; i++)
  {
    if (s_tBtn[i].State == 0)
    { 
      count++;						       
    }
  }
  
  if (count == HARD_KEY_NUM)
  {
    return 1;	/* 全部鍵按釋放時才有效 */
  }		

  return 0;
}

/********************************************************************************************************
*	函 數 名: SetPingKeyLocked
*	功能說明: 上鎖按鍵偵測功能
*	形    參: 無
*	返 回 值: 無
********************************************************************************************************/
void SetPingKeyLocked(void)
{
  pingKeyLocked = 1;
  ClearKey();
}

/*************** END OF FUNCTIONS ***************************************************************************/
