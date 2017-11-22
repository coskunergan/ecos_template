/*
*
* ECOS V-1-0-0 ( Ergan Coskun Operation System)
* Coskun ERGAN
* 07.09.2017
*
 */

#ifndef CTIMER_H_
#define CTIMER_H_

#include "etimer.h"

struct ctimer {
  struct ctimer *next;
  struct etimer etimer;
  struct process *p;
  void (*f)(void *);
  void *ptr;
};

void ctimer_reset(struct ctimer *c);
void ctimer_restart(struct ctimer *c);
void ctimer_set(struct ctimer *c, clock_time_t t,void (*f)(void *), void *ptr);
void ctimer_set_with_process(struct ctimer *c, clock_time_t t,void (*f)(void *), void *ptr, struct process *p);
void ctimer_stop(struct ctimer *c);
int ctimer_expired(struct ctimer *c);
void ctimer_init(void);

#endif /* CTIMER_H_ */

/** @} */

