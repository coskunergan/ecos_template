
#include "ecos.h"

#include "adp_thread.h"

/*---------------------------------------------------------------------------*/
PROCESS(adp_process, "adp process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(adp_process, ev, data)
{
   static struct etimer timer_etimer;
	static uint8_t retry=10;
	
    PROCESS_BEGIN();

    printf("ADP Wait!\n");

	while( (adp_wait_for_handshake() != ADP_HANDSHAKE_ACCEPTED) && (--retry > 0))
	{
		etimer_set(&timer_etimer,15);// 150 mSn		 
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);			
	}
	
	if(retry != 0)
	{			
		adp_window_add_dashboard_control();

		adp_window_add_terminal();
	}

	PROCESS_EXIT();
	
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
