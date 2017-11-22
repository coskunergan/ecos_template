/*
*
* ECOS V-1-0-0 ( Ergan Coskun Operation System)
* Coskun ERGAN
* 07.09.2017
*
 */

//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>

#include "ecos.h"
#include "clock.h"

/*-----------------------------------------------------------------------------------*/
clock_time_t
clock_time(void)
{
  unsigned long long time;

//  GetSystemTimeAsFileTime((PFILETIME)&time);
  return (clock_time_t)(time / 10000);
}
/*-----------------------------------------------------------------------------------*/
unsigned long
clock_seconds(void)
{
  unsigned long long time;

 // GetSystemTimeAsFileTime((PFILETIME)&time);
  return (clock_time_t)(time / 10);
}
/*-----------------------------------------------------------------------------------*/
