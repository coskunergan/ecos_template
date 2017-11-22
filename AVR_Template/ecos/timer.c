/*
*
* ECOS V-1-0-0 ( Ergan Coskun Operation System)
* Coskun ERGAN
* 07.09.2017
*
 */

#include "ecos.h"
#include "timer.h"

/*---------------------------------------------------------------------------*/
void timer_set(struct timer *t, clock_time_t interval)
{
  t->interval = interval;
  t->start = clock_time();
}
/*---------------------------------------------------------------------------*/
void timer_reset(struct timer *t)
{
  t->start += t->interval;
}
/*---------------------------------------------------------------------------*/
void timer_restart(struct timer *t)
{
  t->start = clock_time();
}
/*---------------------------------------------------------------------------*/
int timer_expired(struct timer *t)
{
  /* Note: Can not return diff >= t->interval so we add 1 to diff and return
     t->interval < diff - required to avoid an internal error in mspgcc. */
  clock_time_t diff = (clock_time() - t->start) + 1;
  return t->interval < diff;

}
/*---------------------------------------------------------------------------*/
clock_time_t timer_remaining(struct timer *t)
{
  return t->start + t->interval - clock_time();
}
/*---------------------------------------------------------------------------*/

/** @} */
