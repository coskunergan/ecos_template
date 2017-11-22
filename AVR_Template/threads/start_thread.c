
#include "ecos.h"
#include "start_thread.h"

/*---------------------------------------------------------------------------*/
PROCESS(start_process, "start process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(start_process, ev, data)
{
   static struct etimer timer_etimer;

    PROCESS_BEGIN();

	printf("Restart the Operation System.\n");
	
    printf("Creating Threads.\n");

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
