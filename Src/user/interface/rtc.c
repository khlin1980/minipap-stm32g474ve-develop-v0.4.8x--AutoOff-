/**========================================================================
 *                           includes
 *========================================================================**/
#include "rtc.h"


/**========================================================================
 *                           define
 *========================================================================**/
//* rtc default time is 2023/JAN/01 00:00:00
#define DEF_YEAR      23U
#define DEF_MONTH     01U
#define DEF_DAY       01U
#define DEF_HOUR      00U
#define DEF_MIN       00U
#define DEF_SEC       00U


/**========================================================================
 *                           typedef
 *========================================================================**/


/**========================================================================
 *                           macro
 *========================================================================**/


/**========================================================================
 *                           variables
 *========================================================================**/
  RTC_TimeTypeDef g_rtc_time = {0};
  RTC_DateTypeDef g_rtc_date = {0};


/**========================================================================
 *                           extern variables
 *========================================================================**/
extern RTC_HandleTypeDef hrtc;


/**========================================================================
 *                           functions
 *========================================================================**/


/**
 * @brief Get the mcu rtc time 
 * 
 * @param sdate 
 * @param stime 
 * @return true :success, else fail
 */
bool get_rtc (RTC_DateTypeDef *sdate, RTC_TimeTypeDef *stime)
{
  if(HAL_RTC_GetTime(&hrtc, stime, RTC_FORMAT_BIN) != HAL_OK) return false; 

  if(HAL_RTC_GetDate(&hrtc, sdate, RTC_FORMAT_BIN) != HAL_OK) return false;

  return true;
}


/**
 * @brief convert unix time to cleandar format
 * 
 * @param unixNum : UTC
 * @return calendar_t : data and time params 
 */
calendar_t uinx_to_time (time_t unixNum)
{
  struct tm *ptm = NULL;
 
  time_t count = unixNum;

  calendar_t calendar = {0};

  ptm = localtime(&count);

  calendar.year     = ptm->tm_year - 100;
  calendar.month    = ptm->tm_mon + 1;
  calendar.day      = ptm->tm_mday;
  calendar.hours    = ptm->tm_hour;
  calendar.minutes  = ptm->tm_min;
  calendar.seconds  = ptm->tm_sec;

  print_debug_msg("%02d/%02d/%02d\r\n", 2000 + calendar.year, calendar.month, calendar.day);
  print_debug_msg("%02d:%02d:%02d\r\n", calendar.hours, calendar.minutes, calendar.seconds);

  return calendar;
}


/**
 * @brief convert clendar to unix time format
 * 
 * @param calendar : data and time params
 * @return time_t : unix time format 
 */
time_t time_to_unix (calendar_t calendar)
{
  struct tm stm = {0};
  time_t utc = 0;
  char buf[50] = {0};

  stm.tm_year = calendar.year + 100;
  stm.tm_mon  = calendar.month - 1;
  stm.tm_mday = calendar.day;
  stm.tm_hour = calendar.hours;
  stm.tm_min  = calendar.minutes;
  stm.tm_sec  = calendar.seconds;

  utc = mktime(&stm);

  unix_to_string(utc, buf);

  return utc; 
}


/**
 * @brief Set the rtc time 
 * 
 * @param calendar : data and time params
 * @return true : success
 * @return false : fail
 */
bool set_rtc_time (calendar_t calendar)
{
  RTC_DateTypeDef sdate = {0};
  RTC_TimeTypeDef stime = {0};

  sdate.Year    = calendar.year;
  sdate.Month   = calendar.month;
  sdate.Date    = calendar.day;
  stime.Hours   = calendar.hours;
  stime.Minutes = calendar.minutes;
  stime.Seconds = calendar.seconds;

  if(HAL_RTC_SetDate(&hrtc, &sdate, RTC_FORMAT_BIN) != HAL_OK) return false;
  
  if(HAL_RTC_SetTime(&hrtc, &stime, RTC_FORMAT_BIN) != HAL_OK) return false;
  
  return true;
}


/**
 * @brief Get the rtc time 
 * 
 * @param calendar : pointer to datatime structure  
 * @return true : success
 * @return false : fail
 */
bool get_rtc_time (calendar_t *calendar)
{
  RTC_DateTypeDef sdate = {0};
  RTC_TimeTypeDef stime = {0};

  if(get_rtc(&sdate, &stime) == false) 
    return false;

  calendar->year    = sdate.Year;
  calendar->month   = sdate.Month;
  calendar->day     = sdate.Date;
  calendar->hours   = stime.Hours;
  calendar->minutes = stime.Minutes;
  calendar->seconds = stime.Seconds;

  return true;
}


/**
 * @brief convert unix to string
 * 
 * @param utc 
 * @param str pointer to string buf
 */
void unix_to_string (time_t utc, char *str)
{
  sprintf(str, "%u", utc);

  print_debug_msg("UTC:%s \r\n", str);
}


/**
 * @brief Set the rtc time default 
 * 
 */
bool set_rtc_time_default (void)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  
  /** Initialize RTC and set the Time and Date */
  sTime.Hours = DEF_HOUR;
  sTime.Minutes = DEF_MIN;
  sTime.Seconds = DEF_SEC;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    return false;
  
  sDate.Year = DEF_YEAR;
  sDate.Month = DEF_MONTH;
  sDate.Date = DEF_DAY;
  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    return false;

  return true;  
}


