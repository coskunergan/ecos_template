
#include "ecos.h"
#include "buzzer_thread.h"

/*---------------------------------------------------------------------------*/
PROCESS(buzzer_process, "buzzer process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(buzzer_process, ev, data)
{
   static struct etimer timer_etimer;

    PROCESS_BEGIN();

    etimer_set(&timer_etimer,1*CLOCK_SECOND);

    printf("Buzzer worked!\n");

    //PROCESS_WAIT_UNTIL(etimer_expired(&etimer_timer));
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

    printf("Buzzer Wait!\n");

    etimer_set(&timer_etimer,1*CLOCK_SECOND);

    //PROCESS_WAIT_UNTIL(timer_expired(&etimer_timer));
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);

    printf("Buzzer End!\n");

    process_start(&led_process,NULL);

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
