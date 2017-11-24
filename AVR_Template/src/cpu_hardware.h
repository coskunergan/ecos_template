/*
 * cpu_hardware.h
 *
 * Created: 10.11.2017 14:10:16
 * Author :  COSKUN ERGAN
 */ 

#ifndef _CPU_HARDWARE
#define _CPU_HARDWARE

#include <asf.h>
#include "compiler.h"
#include "xprintf.h"
#include "conf_clock.h"
#include "conf_uart.h"
#include <string.h>
//#include "string.h"
#include "adp.h"
#include "adp_interface.h"

#include "calendar.h"
#include "ring_buffer.h"
#include <avr/interrupt.h>
#include <util/setbaud.h>

#define CPU_PERCENT_AVG (uint8_t)((cpu_avg*100)/(TICK_TIMER_PERIOD-TICK_TIMER_RELOAD))
#define CPU_PERCENT_MAX (uint8_t)((cpu_max*100)/(TICK_TIMER_PERIOD-TICK_TIMER_RELOAD))
#define CPU_PERCENT_MIN (uint8_t)((cpu_min*100)/(TICK_TIMER_PERIOD-TICK_TIMER_RELOAD))

#define BUFFER_SIZE 64

#define TICK_TIMER_PERIOD  255
#define TICK_TIMER_RELOAD  99
#define TICK_TIMER_COUNTER TCNT0

#define CPU_TIMER_PERIOD   255
#define CPU_TIMER_COUNTER  TCNT2

//#define SEC_TIMER_RELOAD  49918 // 1024 prescaler için 1 sn de bir 16MHz de
#define SEC_TIMER_RELOAD  3067 // 512 prescaler için 1 sn de bir 16MHz de
#define SEC_TIMER_COUNTER   TCNT1
#define CPU_AVG_CYCLE  100 // 1 sn

enum adp_example_element_id {
	/* Dashboard example */
	ELEMENT_ID_LABEL_TITEL,
	ELEMENT_ID_LABEL_VALUE_VOL,
	ELEMENT_ID_LABEL_NIGHT_LIGHT,
	ELEMENT_ID_LABEL_LED_CONTROL,
	ELEMENT_ID_BAR,
	ELEMENT_ID_SIGNAL,
	ELEMENT_ID_BUTTON,
	ELEMENT_ID_GRAPH,
	
	/* Dashboard config */
	ELEMENT_ID_LABEL_HYSTERESIS,
	ELEMENT_ID_LABEL_LOW,
	ELEMENT_ID_LABEL_HIGH,
	ELEMENT_ID_LABEL_VALUE_ADC,
	ELEMENT_ID_SLIDER_LOW,
	ELEMENT_ID_SLIDER_HIGH,
	ELEMENT_ID_SEGMENT,
};

extern struct calendar_date real_date;

extern void Tick_Timer_Process(void);
extern void Second_Timer_Process(void);
void Send_to_Cpu_Information(void);
void Restart_Cpu_Measurement(void);
void Cpu_Measurement(void);
void Cpu_Hardware_Init(void);
void adp_window_add_dashboard_control(void);
void adp_window_add_terminal(void);

#endif
