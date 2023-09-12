/**========================================================================
 *                           includes
 *========================================================================**/
#include "sensor_flow.h"  


/**========================================================================
 *                           typedef
 *========================================================================**/


/**========================================================================
 *                           define
 *========================================================================**/
//* differential pressure sensor (flow)  
#define SDP31_DEV_ID            (uint8_t)(0x21 << 1) 

//* for communication attempt max times
#define COMM_MAX_ATTEMPT_TIMES  (20u)

//* choose interrupt or dma type for transit and receive
#define USE_DMA (0u)

/**========================================================================
 *                           macro
 *========================================================================**/


/**========================================================================
 *                           variables
 *========================================================================**/
const uint8_t start_measurement[] = {0x36, 0x03};
const uint8_t stop_measurement[]  = {0x3F, 0xF9};

static uint8_t dp_buf[4] = {0};
//static int16_t dp_raw_value = 0;
static uint16_t dp_raw_value = 0;
static bool dp_err_flag = 0;
static uint32_t dp_err_cnt = 0;


/**========================================================================
 *                           extern variables (from other files)
 *========================================================================**/
extern I2C_HandleTypeDef hi2c4;
#define DP_I2C  hi2c4   // differential pressure sensor (flow)


/**========================================================================
 *                           functions
 *========================================================================**/
static bool CheckCrc(uint8_t data[], uint8_t nBytes, uint8_t checksum);


/**
 * @brief 
 * 
 */
static bool CheckCrc(uint8_t data[], uint8_t nBytes, uint8_t checksum) 
{
  const uint16_t POLYNOMIAL = 0x31; //P(x)=x^8+x^5+x^4+1 = 100110001
  
  uint8_t crc = 0xFF;

  //calculates 8-Bit checksum with given polynomial
  for (uint8_t byteCtr = 0; byteCtr < nBytes; ++byteCtr) {
    crc ^= (data[byteCtr]);

    for (uint8_t bit = 8; bit > 0; --bit) {
      if (crc & 0x80)
        crc = (crc << 1) ^ POLYNOMIAL;
      else
        crc = (crc << 1);
    }
  }

  return crc == checksum;
}


/**
 * @brief 
 * 
 */
bool SDP3x_enable_measurement() 
{
  //! need to be consider change to interrupt type 
  if (HAL_I2C_Master_Transmit(&DP_I2C, SDP31_DEV_ID, (uint8_t *)start_measurement, 2, 100) == HAL_OK) 
  {
    //* according to SPEC, the first measurement result is available after 8ms.
    //HAL_Delay(10);
    return true;
  }

  return false;
}


/**
 * @brief 
 * 
 */
bool SDP3x_read_results(uint8_t* buffer, uint16_t readSize) 
{
  #if(USE_DMA == 0)
    return HAL_I2C_Master_Receive_IT(&DP_I2C, (SDP31_DEV_ID | 0x01), buffer, readSize) == HAL_OK;
  
  #else
    return HAL_I2C_Master_Receive_DMA(&DP_I2C, (SDP31_DEV_ID | 0x01), buffer, readSize) == HAL_OK;
  
  #endif
}


/**
 * @brief 
 * 
 */
uint8_t DP_get_error(void)
{
  return dp_err_flag; 
}


/**
 * @brief 
 * 
 */
void DP_clean_error(void)
{
  dp_err_flag = 0;
  dp_err_cnt = 0; 
}


/**
 * @brief 
 * 
 */
int16_t DP_get_value(void) 
{
  return dp_raw_value; 
}


/**
 * @brief 
 * 
 */
bool DP_measure_start(void) 
{
  if(dp_err_flag) return false;

  // if comm fail up to attempt times, set sensor comm err
  if(++dp_err_cnt >= COMM_MAX_ATTEMPT_TIMES)
  {
    dp_err_flag = 1;
    return false;
  }

  if(SDP3x_read_results(dp_buf, sizeof(dp_buf)) == false)
  {
    //* re-init the i2c module
    HAL_I2C_DeInit(&DP_I2C);
    HAL_I2C_Init(&DP_I2C);
    return false;
  }

  return true;
}


/**
 * @brief 
 * 
 * @param hi2c 
 */
  
void DP_master_rx_cplt_callback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c == &DP_I2C) 
  {
    dp_err_cnt = 0;

    // get raw value
    if (CheckCrc(dp_buf, 2, dp_buf[2]) == true)
      dp_raw_value = 0xFFFF & ((dp_buf[0] << 8) | dp_buf[1]);
    
    //rawflow = (int16_t)dp_raw_value;
    
    //ttt(rawflow);
  }
}

/**
 * @brief 
 * 
 */
float DP_convert_raw_to_LPM(int16_t raw)
{
  //* convert formula, (according the system, need to through experiment to know)
  float pa = (float)(raw)/60;
  float spa = sqrt(pa<0 ? -pa : pa);
        
  //float flow = -0.0067 *spa* spa*spa + 0.2616*spa*spa + 7.4291*spa-0.9404;
  float flow = -0.0067 * pow(spa, 3) + 0.2616 * pow(spa,2) + 7.4291 * spa - 0.9404;

  if(pa<0) flow = -flow;
  
  return flow;

}


/**
 * @brief 
 * 
 */
float DP_convert_raw_to_Pa(int16_t raw)
{
  //* convert differential pressure to Pascal, "SDP31" scale factor is 60  

  float Pa = raw / (float)60.0f;
      
  return Pa;
}