
#include "ecos.h"
#include "cpu_thread.h"

/*---------------------------------------------------------------------------*/
PROCESS(cpu_process, "cpu process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cpu_process, ev, data)
{
	static struct etimer timer_etimer;

    PROCESS_BEGIN();
	
	Restart_Cpu_Measurement();

	while(1)
	{    
		etimer_set(&timer_etimer,CLOCK_SECOND);		
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);			
		Send_to_Cpu_Information();
	}	

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
