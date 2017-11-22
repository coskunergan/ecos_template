
#include "ecos.h"
#include "start_thread.h"


PROCINIT(	&etimer_process,    \				
			&adp_process,		\
			&led_process,		\			
			&buzzer_process 	\
			);
/*---------------------------------------------------------------------------*/
PROCESS(start_process, "start process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(start_process, ev, data)
{
	int i;
	
    PROCESS_BEGIN();

	printf("Start the Operation System.\n");
	
    printf("Creating Threads.\n");
	
	for(i = 0; procinit[i] != NULL; ++i) 
	{
		process_start((struct process *)procinit[i], NULL);// tüm thread leri çalýþtýrýr.
	}
	
	PROCESS_EXIT();
	
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
