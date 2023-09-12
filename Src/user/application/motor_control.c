/**========================================================================
 *                           includes
 *========================================================================**/
#include "motor_control.h"
#include "mc_api.h"

/**========================================================================
 *                           define
 *========================================================================**/
//* for loop control
#define POLLING_BASETIME            (uint32_t)(10)                      // ms
#define SET_BASETIME_CNT_ms(x)      (uint32_t)(x / POLLING_BASETIME)    // ms  

//* for speed fulat 
#define MAX_SPEED                   (45000)  
#define MC_USER_FAULT_SPEED         (0x80000000)  

//* for dedug used
#define DEBUG_MOTOR_CONTROL         (1u)

/**========================================================================
 *                           typedef
 *========================================================================**/


/**========================================================================
 *                           macro
 *========================================================================**/


/**========================================================================
 *                           variables
 *========================================================================**/
uint8_t motorctrl = 0;
static bool motorctrl_on = false;
static bool motorctrl_fault = false;
static uint32_t motorctrl_basetime_cnt = 0;

static uint32_t motorctrl_curr_rpm = 0;
uint32_t motorctrl_error_code = 0;

//* for motor control debug used
uint8_t db_update_rpm = 0; 
uint16_t db_rpm = 6000, db_ms = 1000;


/**========================================================================
 *                           extern variables
 *========================================================================**/

/**========================================================================
 *                           functions
 *========================================================================**/
static bool set_motor_start (void);
static bool set_motor_stop (void);
static void speed_fault_detection(void);
static void fault_polling(void);
static uint32_t get_motor_speed(void);
static void debug_set_motor_speed (void);


/**
 * @brief 
 * 
 */
static bool set_motor_start (void)
{
    //* every time clear fault before motor startup 
    motorctrl_fault = false;
    motorctrl_error_code = 0;
    motorctrl_clean_driver_fault();

    //* set init speed @ 6000RPM 
    motorctrl_set_motor_speed(6000, 1000);
    
    /* Ramp parameters should be tuned for the actual motor */
    return MC_StartMotor1();
}


/**
 * @brief 
 * 
 */
static bool set_motor_stop (void)
{
    return MC_StopMotor1();
}



/**
 * @brief 
 * 
 */
static void speed_fault_detection(void)
{
    //! this function can detect remove motor cable during motor run,
    //! but can not detect if cable not installed brefore motor start
    //! need to be improvement this function.
    
    static uint8_t spd_fault_cnt = 0;
    
    if(motorctrl_fault == true) return;
    
    if(get_motor_speed() > (MAX_SPEED + 3000))
    {
        //* debounce 100ms
        if(++spd_fault_cnt >= 10)
        {
            spd_fault_cnt = 0;
            
            motorctrl_error_code |= MC_USER_FAULT_SPEED; 
            motorctrl_fault = true;
            motorctrl_on = false;
            set_motor_stop();

            motorctrl_set_status(MTRCTRL_ERROR);
        }
    }
    else
    {
        spd_fault_cnt = 0;
    }
}


/**
 * @brief 
 * 
 */
static void fault_polling(void)
{
    if(motorctrl == MTRCTRL_IDLE || motorctrl == MTRCTRL_ERROR)   
        return;

    //* detect errors during motor running 
    motorctrl_error_code = motorctrl_get_driver_fault();

    if(motorctrl_error_code != 0)
    {
        motorctrl_fault = true;
        motorctrl_on = false;
        set_motor_stop();

        motorctrl_set_status(MTRCTRL_ERROR);
    }
}


/**
 * @brief 
 * 
 */
static uint32_t get_motor_speed(void)
{
    return MC_GetMecSpeedAverageMotor1() * 6;   
}


/**
 * @brief 
 * 
 */
void motorctrl_set_motor_speed (uint32_t rpm, uint16_t duration_ms)
{
    int16_t _01hz = (int16_t)(rpm / 6);
    
    MC_ProgramSpeedRampMotor1(_01hz, duration_ms);
}


/**
 * @brief 
 * 
 */
uint32_t motorctrl_get_motor_speed (void)
{
    return motorctrl_curr_rpm;
}


/**
 * @brief 
 * 
 */
uint32_t motorctrl_get_driver_fault (void)
{
    uint16_t u16_bits = 0;
    uint32_t fault_bits = 0;

    u16_bits = MC_GetCurrentFaultsMotor1();
    fault_bits = u16_bits << 16;  

    u16_bits = MC_GetOccurredFaultsMotor1();
    fault_bits |= u16_bits;

    return fault_bits;
}


/**
 * @brief 
 * 
 */
void motorctrl_clean_driver_fault (void)
{
    MC_AcknowledgeFaultMotor1();
}


/**
 * @brief 
 * 
 */
void motorctrl_set_status (uint8_t status)
{
    motorctrl = status;
    motorctrl_basetime_cnt = 0;
}


/**
 * @brief 
 * 
 */
uint8_t motorctrl_get_status (void)
{
    return motorctrl;
}


/**
 * @brief 
 * 
 */
void motorctrl_init(void)
{    
    motorctrl_on = false;
    motorctrl_fault = false;

    motorctrl_set_status(MTRCTRL_IDLE);
}


/**
 * @brief 
 * 
 */
bool motorctrl_set_start(void)
{
    //* force motor startup motor in idle status
    if(motorctrl == MTRCTRL_IDLE)
    {
        motorctrl_on = true;
        motorctrl_set_status(MTRCTRL_STARTUP);
        set_motor_start();
        
        return true; 
    }

    return false;
}


/**
 * @brief 
 * 
 */
bool motorctrl_set_stop(void)
{
    if(motorctrl == MTRCTRL_STARTUP || motorctrl == MTRCTRL_RUNNING)
    {
        motorctrl_on = false;
        motorctrl_set_status(MTRCTRL_STOP);
        set_motor_stop();
        
        return true; 
    }
    
    return false;
}


/**
 * @brief 
 * 
 */
void motorctrl_polling (void)
{
    //* polling per 10ms 
    fault_polling();
    speed_fault_detection();
    motorctrl_curr_rpm = get_motor_speed();
    
    switch(motorctrl)
    {
        case MTRCTRL_IDLE:
        break;

        case MTRCTRL_STARTUP:
        //* delay few second, turn to running status
        if(++motorctrl_basetime_cnt >= SET_BASETIME_CNT_ms(5000))
        {
            motorctrl_set_status(MTRCTRL_RUNNING);
        }
        break;

        case MTRCTRL_RUNNING:
        debug_set_motor_speed();
        break;

        case MTRCTRL_STOP:
        //* delay few second, turn to idle
        if(++motorctrl_basetime_cnt >= SET_BASETIME_CNT_ms(1000))
        {
            motorctrl_set_status(MTRCTRL_IDLE);
        }
        break;

        case MTRCTRL_ERROR:
        //* do nothing when motor fault until the error clean
        break;
    }
}


/**
 * @brief 
 * 
 */
bool is_motorctrl_fault (void)
{
    return motorctrl_fault;
}


/**
 * @brief 
 * 
 */
void motorctrl_clean_errors(void)
{
    if(motorctrl == MTRCTRL_ERROR)
    {
        motorctrl_fault = false;
        motorctrl_error_code = 0;
        motorctrl_clean_driver_fault();

        motorctrl_set_status(MTRCTRL_IDLE);
    }
}


/**
 * @brief 
 * 
 */
static void debug_set_motor_speed (void)
{
 #if(DEBUG_MOTOR_CONTROL != 0)
    
    if(db_update_rpm)
    {
        db_update_rpm = 0;
        motorctrl_set_motor_speed(db_rpm, db_ms);
    }

 #endif
}

