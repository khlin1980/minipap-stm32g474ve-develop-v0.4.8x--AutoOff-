#ifndef __SENSOR_H
#define __SENSOR_H

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

/**========================================================================
 *                           exported constants
 *========================================================================**/

/**========================================================================
 *                           exported macro
 *========================================================================**/

/**========================================================================
 *                           exported functions
 *========================================================================**/
bool smate_39dl_measure(uint8_t* buffer, uint16_t dataSize);
uint8_t GP_get_error(void);
void GP_clean_error(void);
uint16_t GP_get_value(void);
bool GP_measure_start(void); 
void GP_master_rx_cplt_callback(I2C_HandleTypeDef *hi2c);
int16_t GP_convert_raw_to_mmh2o(uint16_t raw);


#ifdef __cplusplus
}
#endif

#endif