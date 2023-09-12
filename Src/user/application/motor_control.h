#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H

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
enum{
    MTRCTRL_IDLE = 0,
    MTRCTRL_STARTUP,   
    MTRCTRL_RUNNING,   
    MTRCTRL_STOP,
    MTRCTRL_ERROR,
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
extern uint32_t motorctrl_error_code;


/**========================================================================
 *                           exported functions
 *========================================================================**/
void motorctrl_set_motor_speed (uint32_t rpm, uint16_t duration_ms);
uint32_t motorctrl_get_motor_speed (void);
uint32_t motorctrl_get_driver_fault (void);
void motorctrl_clean_driver_fault (void);
void motorctrl_set_status (uint8_t status);
uint8_t motorctrl_get_status (void);
void motorctrl_init(void);
bool motorctrl_set_start(void);
bool motorctrl_set_stop(void);
void motorctrl_polling (void);
bool is_motorctrl_fault (void);
void motorctrl_clean_errors(void);


#ifdef __cplusplus
}
#endif

#endif