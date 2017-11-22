
#include "ecos.h"
#include "led_thread.h"

/*---------------------------------------------------------------------------*/
PROCESS(led_process, "led process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(led_process, ev, data)
{
	static struct etimer timer_etimer;

    PROCESS_BEGIN();

	while(1)
	{    
		printf("Led Toggle!\n");
		gpio_toggle_pin(GREEN_LED_PIN);	
		etimer_set(&timer_etimer,CLOCK_SECOND);		
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
	}	

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
