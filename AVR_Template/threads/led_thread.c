
#include "ecos.h"
#include "led_thread.h"

/*---------------------------------------------------------------------------*/
PROCESS(led_process, "led process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(led_process, ev, data)
{
   static struct etimer timer_etimer;

    PROCESS_BEGIN();

    etimer_set(&timer_etimer,1.5*CLOCK_SECOND);


    printf("Led ON!\n");

    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

    printf("Led OFF!\n");

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
