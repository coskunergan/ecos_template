/*
*
* ECOS V-1-0-0 ( Ergan Coskun Operation System)
* Coskun ERGAN
* 07.09.2017
*
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#define CLOCK_CONF_SECOND 1000
typedef unsigned long clock_time_t;

#ifdef CLOCK_CONF_SECOND
#define CLOCK_SECOND CLOCK_CONF_SECOND
#else
#define CLOCK_SECOND (clock_time_t)32
#endif

void clock_init(void);
clock_time_t clock_time(void);
unsigned long clock_seconds(void);
void clock_set_seconds(unsigned long sec);
void clock_wait(clock_time_t t);
void clock_delay_usec(int dt);
int clock_fine_max(void);
unsigned short clock_fine(void);
void clock_delay(unsigned int delay);

#endif /* CLOCK_H_ */

/** @} */
