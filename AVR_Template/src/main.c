/*
 * main.c
 *
 * Created: 28.09.2017 14:10:16
 * Author :  COSKUN ERGAN
 */ 

#include "cpu_hardware.h"
#include "ecos.h"

extern const struct process *procinit[];

/****************************************************************************/
void Tick_Timer_Process(void)
{
	ecos_tick();
}
/****************************************************************************/
void Second_Timer_Process(void)
{

}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
int main (void)
{
	Cpu_Hardware_Init();
	
    printf("Restart\n");

    process_init();
	
    process_start(&start_process,NULL); // start ilk thread "start_thread"
	
	printf("loop!\n");
	
	while(1)
	{
		 	Cpu_Measurement();

			//xprintf("time = %d.%d.%d - %02d:%02d:%02d wd=%d\r",real_date.year,real_date.month+1,real_date.date+1,real_date.hour,real_date.minute,real_date.second,real_date.dayofweek);	
					
			//delay_us(250);// + %3
			
			process_run();
			
			gpio_toggle_pin(RED_LED_PIN);		
	}

}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/