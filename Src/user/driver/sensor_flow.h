#ifndef __SENSOR_FLOW_H
#define __SENSOR_FLOW_H

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
bool SDP3x_enable_measurement();
bool SDP3x_read_results(uint8_t* buffer, uint16_t readSize);
uint8_t DP_get_error(void);
void DP_clean_error(void);
int16_t DP_get_value(void);
bool DP_measure_start(void);
void DP_master_rx_cplt_callback(I2C_HandleTypeDef *hi2c);
//int16_t DP_convert_raw_to_LPM(int16_t raw);
//uint16_t DP_convert_raw_to_LPM(uint16_t raw);
float DP_convert_raw_to_LPM(int16_t raw);

float DP_convert_raw_to_Pa(int16_t raw);

#ifdef __cplusplus
}
#endif

#endif