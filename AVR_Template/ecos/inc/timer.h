/*
*
* ECOS V-1-0-0 ( Ergan Coskun Operation System)
* Coskun ERGAN
* 07.09.2017
*
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "clock.h"

struct timer {
  clock_time_t start;
  clock_time_t interval;
};

/*CCIF*/void timer_set(struct timer *t, clock_time_t interval);
void timer_reset(struct timer *t);
void timer_restart(struct timer *t);
/*CCIF*/ int timer_expired(struct timer *t);
clock_time_t timer_remaining(struct timer *t);

#endif /* TIMER_H_ */

/** @} */
/** @} */
