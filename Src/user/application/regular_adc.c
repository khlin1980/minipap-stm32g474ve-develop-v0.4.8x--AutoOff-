/**========================================================================
 *                           includes
 *========================================================================**/
#include "regular_adc.h"


/**========================================================================
 *                           define
 *========================================================================**/
#define SAMPLING_AVG_SHIFT_BITS   3
#define TOTAL_SAMPLING_TIMES      (1 << SAMPLING_AVG_SHIFT_BITS)


/**========================================================================
 *                           typedef
 *========================================================================**/
//* user regular adc channel definition
enum {
  ADC_MOTOR_TEMPERATURE = 0,
  ADC_VBAT,

  NUM_OF_REGULAR_ADC_CHANNEL,
};


/**========================================================================
 *                           macro
 *========================================================================**/


/**========================================================================
 *                           variables
 *========================================================================**/
RegConv_t reg_conv[NUM_OF_REGULAR_ADC_CHANNEL] = {0};
uint8_t reg_handler[NUM_OF_REGULAR_ADC_CHANNEL] = {0}; 
uint8_t reg_conv_ch = 0;

uint32_t adc_temperature_sum = 0;
uint8_t adc_temperature_sample_cnt = 0;

uint32_t adc_vbat_sum = 0;
uint8_t adc_vbat_sample_cnt = 0;

int32_t motor_temperature = 250;  //* default 25.0 degree
uint16_t vbat_voltage = 330;      //* default 3.3v

/**========================================================================
 *                           extern variables
 *========================================================================**/


/**========================================================================
 *                           functions
 *========================================================================**/
static int NTC_ADC2Temperature(unsigned char adc_value);
static void motor_temperature_conversion_cb(uint8_t handle, uint16_t data, void *UserData);
static void vbat_conversion_cb(uint8_t handle, uint16_t data, void *UserData);


/**
* The NTC table has 256 interpolation points.
* Unit:0.1 °C
*
*/
const int NTC_table[256] = {
  2653, 2262, 1871, 1669, 1536, 1437, 1360, 
  1296, 1242, 1196, 1155, 1119, 1086, 1057, 
  1029, 1004, 981, 959, 939, 920, 902, 885, 
  869, 854, 839, 825, 812, 799, 787, 775, 763, 
  752, 741, 731, 721, 711, 702, 693, 684, 675, 
  667, 658, 650, 643, 635, 627, 620, 613, 606, 
  599, 592, 586, 579, 573, 566, 560, 554, 548, 
  542, 537, 531, 525, 520, 514, 509, 504, 498, 
  493, 488, 483, 478, 473, 468, 464, 459, 454, 
  449, 445, 440, 436, 431, 427, 423, 418, 414, 
  410, 405, 401, 397, 393, 389, 385, 381, 377, 
  373, 369, 365, 361, 357, 353, 350, 346, 342, 
  338, 335, 331, 327, 324, 320, 316, 313, 309, 
  306, 302, 298, 295, 291, 288, 284, 281, 277, 
  274, 270, 267, 264, 260, 257, 253, 250, 247, 
  243, 240, 237, 233, 230, 226, 223, 220, 216, 
  213, 210, 206, 203, 200, 196, 193, 190, 187, 
  183, 180, 177, 173, 170, 167, 163, 160, 157, 
  153, 150, 147, 143, 140, 136, 133, 130, 126, 
  123, 120, 116, 113, 109, 106, 102, 99, 95, 
  92, 88, 85, 81, 78, 74, 70, 67, 63, 60, 56, 
  52, 48, 45, 41, 37, 33, 29, 26, 22, 18, 14, 
  10, 6, 2, -3, -7, -11, -15, -20, -24, -28, 
  -33, -37, -42, -46, -51, -56, -61, -66, -71, 
  -76, -81, -86, -91, -97, -102, -108, -114, 
  -120, -126, -132, -138, -145, -151, -158, 
  -165, -172, -180, -187, -195, -204, -212, 
  -221, -231, -241, -251, -262, -273, -286, 
  -299, -313, -329, -346, -365, -386, -411, 
  -440, -477, -527, -577
};
 
 
/**
* \brief    Converts the ADC result into a temperature value.
*
*           The temperature values are read from the table.
*
* \param    adc_value  The converted ADC result
* \return              The temperature in 0.1 °C
*
*/
static int NTC_ADC2Temperature(unsigned char adc_value){
 
  //* this code is generated form web => "sebulli Table based NTC code generator"
  /*
    pullup type, pullup resistance 10Kohm, 
    NTC spec: B-value 4100, 10Kohm@25degree
    releated setting:
    num of interpolating points: 256
    unit in 'C: 0.1
    adc resolution: 8 bits 
  */
  
  /* Read values directly from the table. */
  return NTC_table[ adc_value ];
};


/**
 * @brief register rugular adc measurement form MCSDK
 * 
 */
void user_regular_adc_init (void)
{
  reg_conv_ch = ADC_MOTOR_TEMPERATURE;
  
  //* motor temperature: adc1_5
  reg_conv[ADC_MOTOR_TEMPERATURE].regADC = ADC1;
  reg_conv[ADC_MOTOR_TEMPERATURE].channel = 5; 
  reg_conv[ADC_MOTOR_TEMPERATURE].samplingTime = ADC_SAMPLETIME_47CYCLES_5;
  reg_handler[ADC_MOTOR_TEMPERATURE] = RCM_RegisterRegConv_WithCB(&reg_conv[ADC_MOTOR_TEMPERATURE], motor_temperature_conversion_cb, NULL);

  //* Vbat: adc1_17
  reg_conv[ADC_VBAT].regADC = ADC1;
  reg_conv[ADC_VBAT].channel = 17; 
  reg_conv[ADC_VBAT].samplingTime = ADC_SAMPLETIME_47CYCLES_5;
  reg_handler[ADC_VBAT] = RCM_RegisterRegConv_WithCB(&reg_conv[ADC_VBAT], vbat_conversion_cb, NULL);
}    


/**
 * @brief user regular adc measure requirement polling
 * 
 */
void user_regular_adc_measure_requirement (void)
{
  /* Check regular conversion readiness */
  if (RCM_GetUserConvState() == RCM_USERCONV_IDLE)
  {
    /* if Idle, then program a new conversion request */
    if(RCM_RequestUserConv(reg_handler[reg_conv_ch]) == true)
    {
      if(++reg_conv_ch >= NUM_OF_REGULAR_ADC_CHANNEL)
        reg_conv_ch = ADC_MOTOR_TEMPERATURE;
    }
  }
  // else if (RCM_GetUserConvState() == RCM_USERCONV_EOC)
  // {
  //   /* if Done, then read the captured value */
  // }
}    


/**
 * @brief motor temperature adc measurememt conversion callback
 * 
 * @param handle 
 * @param data 
 * @param UserData 
 */
static void motor_temperature_conversion_cb(uint8_t handle, uint16_t data, void *UserData)
{
  adc_temperature_sum += data;
  
  if(++adc_temperature_sample_cnt >= TOTAL_SAMPLING_TIMES)
  {
    //* convert 16bits to 8bits resolution, sampling total n times 
    uint8_t adc_avg = (uint8_t)(adc_temperature_sum >> (8 + SAMPLING_AVG_SHIFT_BITS));

    //* get temperature via ntc table
    motor_temperature = NTC_ADC2Temperature(adc_avg);
    
    //* reset variables
    adc_temperature_sum = 0;
    adc_temperature_sample_cnt = 0;
  }
}


/**
 * @brief vbat adc measurememt conversion callback
 * 
 * @param handle 
 * @param data 
 * @param UserData 
 */
static void vbat_conversion_cb(uint8_t handle, uint16_t data, void *UserData)
{
  adc_vbat_sum += data;
  
  if(++adc_vbat_sample_cnt >= TOTAL_SAMPLING_TIMES)
  {
    //* convert 16bits to 12bits resolution, sampling total n times 
    uint16_t adc_avg = (uint16_t)(adc_vbat_sum >> (4 + SAMPLING_AVG_SHIFT_BITS));

    //* get real vbat = vbat * 3 / 4096 * 3.3
    vbat_voltage = adc_avg * 3 * 330 / 4096;  //* keep the first two digits after the decimal point
    
    //* reset variables
    adc_vbat_sum = 0;
    adc_vbat_sample_cnt = 0;
  }
}


/**
 * @brief Get the motor temperature 
 * 
 * @return int (unit: .1 celsius degree)
 */
int get_motor_temperature (void)
{
  return motor_temperature;
}


/**
 * @brief Get the vbat voltage 
 * 
 * @return uint16_t (unit: .01 voltage)
 */
uint16_t get_vbat_voltage (void)
{
  return vbat_voltage;
}