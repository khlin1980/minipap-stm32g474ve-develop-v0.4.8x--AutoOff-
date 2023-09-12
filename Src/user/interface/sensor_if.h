#ifndef __SENSOR_INTERFACE_H
#define __SENSOR_INTERFACE_H

#ifdef __cplusplus
 extern "C" {
#endif

/**========================================================================
 *                           includes
 *========================================================================**/
#include "bsp.h"
#include "sensor_flow.h"
#include "sensor_pressure.h"

/**========================================================================
 *                           exported types
 *========================================================================**/
enum{
  SENSOR_NO_ERR = 0,
  SENSOR_ERR_INIT,
  SENSOR_ERR_DP,
  SENSOR_ERR_GP,
};

/**========================================================================
 *                           exported constants
 *========================================================================**/

/**========================================================================
 *                           exported macro
 *========================================================================**/

/**========================================================================
 *                           exported variables
 *========================================================================**/
extern int16_t curr_flow;
extern int16_t curr_cmH2o;

/**========================================================================
 *                           exported functions
 *========================================================================**/
void sensors_power_enable (bool enable);
void sensors_control_init (void);
uint8_t sensors_get_errors (void);
void sensors_control_reset (void);
void sensors_control_polling (void);
void sensors_rx_cplt_handler(I2C_HandleTypeDef *hi2c);


#ifdef __cplusplus
}
#endif

#endif