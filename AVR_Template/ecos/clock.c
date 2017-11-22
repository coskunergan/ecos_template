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

/*---------------------------------------------------------------------------*/
static volatile unsigned long seconds = 0;
static volatile clock_time_t ticks;
/*---------------------------------------------------------------------------*/
void ecos_tick(void)
{
	ticks++;
	if((ticks % CLOCK_SECOND) == 0) {
		seconds++;
	}
	
	if(etimer_pending()) {
		etimer_request_poll();
	}	
}
/*---------------------------------------------------------------------------*/
void clock_init(void)
{
	ticks = 0;
}
/*---------------------------------------------------------------------------*/
unsigned long clock_seconds(void)
{
	return seconds;
}
/*---------------------------------------------------------------------------*/
void clock_set_seconds(unsigned long sec)
{
	seconds = sec;
}
/*---------------------------------------------------------------------------*/
clock_time_t clock_time(void)
{
	return ticks;
}
/*---------------------------------------------------------------------------*/
void clock_delay(unsigned int i)
{
	for(; i > 0; i--) {
		unsigned int j;
		for(j = 50; j > 0; j--) {
			asm ("nop");
		}
	}
}
/*---------------------------------------------------------------------------*/
/* Wait for a multiple of clock ticks (7.8 ms at 128 Hz). */
void clock_wait(clock_time_t t)
{
	clock_time_t start;
	start = clock_time();
	while(clock_time() - start < (clock_time_t)t) ;
}
/*---------------------------------------------------------------------------*/
