#ifndef __RTC_H
#define __RTC_H

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
typedef struct _calendar_
{
    uint8_t year;       /* value, 0 ~ 255, 2000 ~ 2255 */
    uint8_t month;      /* value, 1 ~ 12 */
    uint8_t day;        /* value, 1 ~ 31 */    
    uint8_t hours;      /* value 0 ~ 23 */
    uint8_t minutes;    /* value 0 ~ 59 */
    uint8_t seconds;    /* value 0 ~ 59 */

}calendar_t;


/**========================================================================
 *                           exported constants
 *========================================================================**/

/**========================================================================
 *                           exported macro
 *========================================================================**/

/**========================================================================
 *                           exported variables
 *========================================================================**/
extern RTC_TimeTypeDef g_rtc_time;
extern RTC_DateTypeDef g_rtc_date;

/**========================================================================
 *                           exported functions
 *========================================================================**/
bool get_rtc (RTC_DateTypeDef *sdate, RTC_TimeTypeDef *stime);
calendar_t uinx_to_time (time_t unixNum);
time_t time_to_unix (calendar_t calendar);
bool set_rtc_time (calendar_t calendar);
bool get_rtc_time (calendar_t *calendar);
void unix_to_string (time_t utc, char *str);
bool set_rtc_time_default (void);

#ifdef __cplusplus
}
#endif

#endif