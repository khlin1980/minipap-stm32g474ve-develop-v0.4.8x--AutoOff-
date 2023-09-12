#ifndef __BSP_H 
#define __BSP_H

#ifdef __cplusplus
 extern "C" {
#endif

//* BSP (Board Support Package)

/**========================================================================
 *                           includes
 *========================================================================**/
//! here, put the standard libary file (low level)
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h> 
#include <string.h> 
#include <time.h>
#include <math.h>
#include <time.h>

//! here, put the third party libary (middleware)
#include "stm32g4xx_hal.h"

//! here, this define is for debug use 
#define USE_DEBUG_IO                    1   // use PD1 as the debug io pin 

//! here, this define is for debug message print
#define USE_DEBUG_PRINT                 1
#if(USE_DEBUG_PRINT == 1)
 #include "debug_io.h"
#endif  

#ifdef __cplusplus
}
#endif

#endif