/**========================================================================
 *                           includes
 *========================================================================**/
#include "sensor_if.h"  


/**========================================================================
 *                           typedef
 *========================================================================**/
enum{
  SENSORS_DEINIT = 0,
  SENSORS_POWER_ON,
  SENSORS_INIT,
  SENSORS_MEASURE_START,
  SENSORS_READING_FLOW,
  SENSORS_READING_PRESSURE,
  SENSORS_ERROR,

};

enum{
  ALTERNATE_FLOW = 0,
  ALTERNATE_PRESSURE,

};


/**========================================================================
 *                           define
 *========================================================================**/
#define SENSOR_EN_Pin           GPIO_PIN_10
#define SENSOR_EN_GPIO_Port     GPIOD

#define POLLING_BASETIME        (uint32_t)(5)                       // ms
#define SET_BASETIME_CNT_ms(x)  (uint32_t)(x / POLLING_BASETIME)    // ms    


/**========================================================================
 *                           macro
 *========================================================================**/


/**========================================================================
 *                           variables
 *========================================================================**/
float curr_flow_pa = 0;
//int16_t curr_flow_lpm = 0;
//uint16_t curr_flow_lpm;
float curr_flow_lpm;
int16_t curr_mmH2O = 0;

static uint8_t ctrl_status = 0; 
static uint32_t ctrl_timecnt = 0; 

static uint8_t sensor_error = 0; 
static uint8_t sensor_alt = 0;
static uint8_t init_err_cnt = 0; 


/**========================================================================
 *                           extern variables (from other files)
 *========================================================================**/
extern I2C_HandleTypeDef hi2c4;
extern void MX_I2C4_Init(void);


/**========================================================================
 *                           functions
 *========================================================================**/
static void set_control_status (uint8_t status, uint32_t time_ms);
static void sensors_error_polling (void);


/**
 * @brief 
 * 
 */
static void set_control_status (uint8_t status, uint32_t time_ms)
{
  ctrl_status = status;
  ctrl_timecnt = SET_BASETIME_CNT_ms(time_ms);
}


/**
 * @brief 
 * 
 */
static void sensors_error_polling (void)
{ 
  if(sensor_error == SENSOR_NO_ERR)
  {
    if(DP_get_error() != 0)
    {
      sensor_error = SENSOR_ERR_DP;
      set_control_status((uint8_t)SENSORS_ERROR, 0);
    }
    else if(GP_get_error() != 0)
    {
      sensor_error = SENSOR_ERR_GP;
      set_control_status((uint8_t)SENSORS_ERROR, 0);
    }
  }
}


/**
 * @brief 
 * 
 */
void sensors_power_enable (bool enable)
{
  if(enable)
    HAL_GPIO_WritePin(SENSOR_EN_GPIO_Port, SENSOR_EN_Pin, GPIO_PIN_RESET);
  
  else
    HAL_GPIO_WritePin(SENSOR_EN_GPIO_Port, SENSOR_EN_Pin, GPIO_PIN_SET);
} 


/**
 * @brief 
 * 
 */
void sensors_control_init (void)
{
  init_err_cnt = 0; 
  sensor_error = SENSOR_NO_ERR; 
  set_control_status(SENSORS_DEINIT, 0);
}


/**
 * @brief 
 * 
 */
uint8_t sensors_get_errors (void)
{ 
  return sensor_error; 
}


/**
 * @brief 
 * 
 */
void sensors_control_reset (void)
{ 
  DP_clean_error();
  GP_clean_error();
  
  sensors_control_init();
}


/**
 * @brief 
 * 
 */
void sensors_control_polling (void)
{
  //* flow and pressure share i2c bus, so need to alternative polling (base_time 5ms)   

  if(ctrl_timecnt > 0)
  {
    ctrl_timecnt--;
    return;
  }

  //* sensor status check
  sensors_error_polling();

  //* control routine
  switch(ctrl_status)
  {
    case SENSORS_DEINIT:
      //* re-init the i2c module
      HAL_I2C_DeInit(&hi2c4);
      HAL_I2C_Init(&hi2c4);
      sensors_power_enable(false);
      set_control_status((uint8_t)SENSORS_POWER_ON, 500);
      break;

    case SENSORS_POWER_ON:
      //* do not init i2c before sensor power on, that will cause flow sensor no working 
      sensors_power_enable(true);
      set_control_status((uint8_t)SENSORS_INIT, 100);
      break;

    case SENSORS_INIT:
      //* wait power stable then init i2c module 
      MX_I2C4_Init();
      set_control_status((uint8_t)SENSORS_MEASURE_START, 0);
      break;
    
    case SENSORS_MEASURE_START:      
      //* set dp(flow) sensor measure start (gp sensor no need)
      if(SDP3x_enable_measurement() == true)
      {
        init_err_cnt = 0;
        //* according to SPEC, the first measurement result is available after 8ms.
        set_control_status((uint8_t)SENSORS_READING_FLOW, 20);
        sensor_alt = ALTERNATE_FLOW;
      }
      else
      {
        if(++init_err_cnt > 3)
        {
          sensor_error = SENSOR_ERR_INIT;
          set_control_status((uint8_t)SENSORS_ERROR, 0);
        }
        else{
          //* retry again
          set_control_status((uint8_t)SENSORS_DEINIT, 0);
        }
      }
      break;

    case SENSORS_READING_FLOW:
      //* convert dp value to flow value
      if(DP_get_error() == false)
      {
        curr_flow_lpm = DP_convert_raw_to_LPM(DP_get_value());
        curr_flow_pa = DP_convert_raw_to_Pa(DP_get_value());
      }
      
      //* start pressure measurement
      GP_measure_start();
      set_control_status((uint8_t)SENSORS_READING_PRESSURE, 0); 
      sensor_alt = ALTERNATE_PRESSURE;
      break;

    case SENSORS_READING_PRESSURE:
      //* convert gp value to pressure value
      if(GP_get_error() == false)
        curr_mmH2O = GP_convert_raw_to_mmh2o(GP_get_value());
      
      //* start flow measurement
      DP_measure_start();
      set_control_status((uint8_t)SENSORS_READING_FLOW, 0);
      sensor_alt = ALTERNATE_FLOW;
      break;

    case SENSORS_ERROR:
      //* keep in here when error occurs, call reset to recover sensors keep reading
      break;
  }
}


/**
 * @brief 
 * 
 * @param hi2c 
 */
void sensors_rx_cplt_handler(I2C_HandleTypeDef *hi2c)
{
  //* dp(flow) and gp(pressure) are share i2c bus, so the irq need to know 
  //* how to handle which sensor is be executed.  

  if(sensor_alt == ALTERNATE_FLOW)
  {
    DP_master_rx_cplt_callback(hi2c);
  }
  else if(sensor_alt == ALTERNATE_PRESSURE)
  {
    GP_master_rx_cplt_callback(hi2c);
  }
}