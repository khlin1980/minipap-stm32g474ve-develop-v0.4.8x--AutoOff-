/**========================================================================
 *                           includes
 *========================================================================**/
#include "sensor_pressure.h"  


/**========================================================================
 *                           typedef
 *========================================================================**/


/**========================================================================
 *                           define
 *========================================================================**/
//* gage pressure sensor (pressure)
#define SM9541_ADDRESS          (0x28 << 1)

//* for communication attempt max times
#define COMM_MAX_ATTEMPT_TIMES  (20u)

//* choose interrupt or dma type for transit and receive
#define USE_DMA                 (0u)

/**========================================================================
 *                           macro
 *========================================================================**/


/**========================================================================
 *                           variables
 *========================================================================**/
static uint8_t gp_buf[7] = { 0 };
static uint16_t gp_raw_value = 0;
static uint32_t gp_err_flag = 0;
static uint32_t gp_err_cnt = 0;


/**========================================================================
 *                           extern variables (from other files)
 *========================================================================**/
extern I2C_HandleTypeDef hi2c4;
#define GP_I2C  hi2c4   // gage pressure sensor (pressure)


/**========================================================================
 *                           functions
 *========================================================================**/


/**
 * @brief 
 * 
 */
bool smate_39dl_measure(uint8_t* buffer, uint16_t dataSize) 
{
  #if(USE_DMA == 0)
    return HAL_I2C_Master_Receive_IT(&GP_I2C, (SM9541_ADDRESS | 0x01), buffer, dataSize) == HAL_OK;
  
  #else
    return HAL_I2C_Master_Receive_DMA(&GP_I2C, (SM9541_ADDRESS | 0x01), buffer, dataSize) == HAL_OK;
  
  #endif
}


/**
 * @brief 
 * 
 */
uint8_t GP_get_error(void)
{
  return gp_err_flag; 
}


/**
 * @brief 
 * 
 */
void GP_clean_error(void)
{
  gp_err_flag = 0;
  gp_err_cnt = 0;
}


/**
 * @brief 
 * 
 */
uint16_t GP_get_value(void) 
{
  return gp_raw_value;
}


/**
 * @brief 
 * 
 */
bool GP_measure_start(void) 
{
  if(gp_err_flag) return false;

  // if comm fail up to attempt times, set sensor comm err
  if(++gp_err_cnt >= COMM_MAX_ATTEMPT_TIMES)
  {
    gp_err_flag = 1;
    return false;
  }

  if(smate_39dl_measure(gp_buf, sizeof(gp_buf)) == false)
  {
    //* re-init the i2c module
    HAL_I2C_DeInit(&GP_I2C);
    HAL_I2C_Init(&GP_I2C);
    return false;
  }

  return true;
}


/**
 * @brief 
 * 
 * @param hi2c 
 */
void GP_master_rx_cplt_callback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c == &GP_I2C)
  {
    gp_err_cnt = 0;
    
    //* get raw value (resolution is 24 bits, but just only get the highest 16 bits)
    gp_raw_value = gp_buf[2] + gp_buf[1] * 256;
  }
}


/**
 * @brief 
 * 
 */
int16_t GP_convert_raw_to_mmh2o(uint16_t raw)
{
  //* convert formula
  //? (value - offset[0x1999]) / (value_max[0xE666] - value_min[0x1999]) * (p_max[40cmh2o] - P_min[-20cmh2o]) - p_min  
  /*
  float cmh2o = raw;
      
  cmh2o = (cmh2o - 0x1999) * 600 / 52429 - 200;  // x10 to get one decimal place 
  */
  float cmH2O = (float)(raw - 0x1999) * 60 / 52429 - 20;
  float mmH2O = cmH2O * 10;

  return (int16_t)mmH2O;
}