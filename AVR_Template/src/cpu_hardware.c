/*
 * cpu_hardware.c
 *
 * Created: 10.11.2017 14:10:16
 * Author :  COSKUN ERGAN
 */ 

#include "cpu_hardware.h"

//#define Assert(expr) ((void) 0) 
volatile uint8_t cpu_max;
volatile uint8_t cpu_min;
volatile uint32_t cpu_avg;

bool cpu_measurement_restart=true;

uint8_t out_buffer[BUFFER_SIZE];
uint8_t in_buffer[BUFFER_SIZE];

#define DASHBOARD_ID_CPU_USAGE    0x00
#define STREAM_ID_STATUS_MESSAGE  0x00
#define STREAM_ID_CPU_USAGE       0x01
 
/* Terminal configuration. Global, so we can change it later */
struct adp_msg_conf_terminal terminal_config = {
	.terminal_id = STREAM_ID_STATUS_MESSAGE,
	.width = 80,
	.height = 50,
	.background_color = {ADP_COLOR_WHITE},
	.foreground_color = {ADP_COLOR_RED}
};

uint8_t receive_packet_data[MSG_RES_PACKET_DATA_MAX_LEN] = {0,};

struct calendar_date real_date = {
	.second = 0,
	.minute = 56,
	.hour = 15,
	.date =18, //0-30 ayýn ilk günü 0 dýr 1 deðil( gerçek gün için +1)
	.month = 9,//0 11 yýlýn ilk günü 0 dýr 1 deðil( gerçek ay için +1)
	.year = 2017
};

void uart_putchar(uint8_t data);
struct ring_buffer ring_buffer_out;
struct ring_buffer ring_buffer_in;
/****************************************************************************/
ISR(TIMER0_OVF_vect)// tick timer (10ms)
{
	TICK_TIMER_COUNTER=TICK_TIMER_RELOAD;
	Tick_Timer_Process();
}
/****************************************************************************/
ISR(TIMER1_OVF_vect) // second timer (1sn)
{
	SEC_TIMER_COUNTER = SEC_TIMER_RELOAD;
	
	Second_Timer_Process();
	
	calendar_add_second_to_date(&real_date);
}
/****************************************************************************/
void Send_to_Cpu_Information(void)
{
	uint8_t temp=CPU_PERCENT_AVG;
			
	adp_transceive_single_stream(STREAM_ID_CPU_USAGE,&temp , 1, receive_packet_data);
	sprintf(receive_packet_data,"Cpu_Max:%%%d\r",CPU_PERCENT_MAX);
	adp_transceive_single_stream(STREAM_ID_STATUS_MESSAGE,receive_packet_data,strlen(receive_packet_data),receive_packet_data);
	sprintf(receive_packet_data,"Cpu_Min:%%%d\r\r",CPU_PERCENT_MIN);
	adp_transceive_single_stream(STREAM_ID_STATUS_MESSAGE,receive_packet_data,strlen(receive_packet_data),receive_packet_data);
}
/****************************************************************************/
void Restart_Cpu_Measurement(void)
{
	cpu_measurement_restart=true;	
}
/****************************************************************************/
ISR(UART0_DATA_EMPTY_IRQ)
{
	// if there is data in the ring buffer, fetch it and send it
	if (!ring_buffer_is_empty(&ring_buffer_out)) {
		UDR0 = ring_buffer_get(&ring_buffer_out);
	}
	else {
		// no more data to send, turn off data ready interrupt
		UCSR0B &= ~(1 << UDRIE0);
	}
}
/****************************************************************************/
ISR(UART0_RX_IRQ)
{
	ring_buffer_put(&ring_buffer_in, UDR0);
}
/****************************************************************************/
static void uart_init(void)
{
	#if defined UBRR0H
	// get the values from the setbaud tool
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	#else
	#error "Device is not supported by the driver"
	#endif

	#if USE_2X
	UCSR0A |= (1 << U2X0);
	#endif

	// enable RX and TX and set interrupts on rx complete
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);

	// 8-bit, 1 stop bit, no parity, asynchronous UART
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00) | (0 << USBS0) |
	(0 << UPM01) | (0 << UPM00) | (0 << UMSEL01) |
	(0 << UMSEL00);

	// initialize the in and out buffer for the UART
	ring_buffer_out = ring_buffer_init(out_buffer, BUFFER_SIZE);
	ring_buffer_in = ring_buffer_init(in_buffer, BUFFER_SIZE);
}
/****************************************************************************/
void uart_putchar(uint8_t data)
{
	uint8_t buffer_use_size;

	do{
		
		if(ring_buffer_out.write_offset < ring_buffer_out.read_offset ) // ring buffer overback olmuþ ise
		{
			buffer_use_size =  ring_buffer_out.read_offset   -    ring_buffer_out.write_offset;
			buffer_use_size =  ring_buffer_out.size          -    buffer_use_size;
		}else
		{
			buffer_use_size =  ring_buffer_out.write_offset  -    ring_buffer_out.read_offset;
		}
		
	}while( ring_buffer_out.size-1 ==  buffer_use_size );// buffer da yeterince yer açýlana kadar bekle.
	
	// Disable interrupts to get exclusive access to ring_buffer_out.
	cli();
	if (ring_buffer_is_empty(&ring_buffer_out)) {
		// First data in buffer, enable data ready interrupt
		UCSR0B |=  (1 << UDRIE0);
	}
	// Put data in buffer
	ring_buffer_put(&ring_buffer_out, data);

	// Re-enable interrupts
	sei();
}
/****************************************************************************/
static inline void uart_putstr(uint8_t *data, uint8_t data_size)
{
	//uint8_t buffer_use_size;

	//do{
	//
	//if(ring_buffer_out.write_offset < ring_buffer_out.read_offset ) // ring buffer overback olmuþ ise
	//{
	//buffer_use_size =  ring_buffer_out.read_offset   -    ring_buffer_out.write_offset;
	//buffer_use_size =  ring_buffer_out.size          -    buffer_use_size;
	//
	//}else
	//{
	//buffer_use_size =  ring_buffer_out.write_offset  -    ring_buffer_out.read_offset;
	//}
	//
	//}while( ( ring_buffer_out.size -  buffer_use_size ) <= data_size );// buffer da yeterince yer açýlana kadar bekle.
	
	//while(ring_buffer_out.read_offset - ring_buffer_out.write_offset); // düzenlenecek ve hýz için buffer kalan hesaplanýp doldurma islemi devam ettirilecek

	for(uint8_t i=0; i<data_size;i++)
	{
		uart_putchar(*data++);
	}
}
/****************************************************************************/
static inline uint8_t uart_getchar(void)
{
	return ring_buffer_get(&ring_buffer_in);
}
/****************************************************************************/
static inline void timers_init(void)
{
	TCNT0=TICK_TIMER_RELOAD;//for 10mS  at 16 MHz Tick Timer
	TCCR0A=0x00;
	TCCR0B=0x05;  // Timer mode with 1024 prescler (clock böleni en yavas haline ayarlý daha fazla yavaslatilamaz)
	TIMSK0=1;// int enable

	TCNT1 = SEC_TIMER_RELOAD;  //for 1 sec at 16 MHz
	TCCR1A=0x00;
	TCCR1B=0x04;  // Timer mode with 512 prescler (clock böleni 16 bit de 1 sn yi hesaplayan en iyi degerde )
	TIMSK1=0x01;  // int enable
	
	TCNT2=0; //for 10mS  at 16 MHz CPU timer
	TCCR2A=0x00;
	TCCR2B=0x07;  // Timer mode with 1024 prescler (clock böleni en tick timer böleni ile ayný olmalýdýr.)
}
/****************************************************************************/
void adp_window_add_terminal(void)
{
	struct adp_msg_configure_stream stream;
	
	//terminal_config.terminal_id = 0;
	//terminal_config.width = 80;
	//terminal_config.height = 50;
	//memcpy(terminal_config.background_color, ADP_COLOR_WHITE, 3);
	//memcpy(terminal_config.foreground_color, ADP_COLOR_GREEN, 3);
	
	/* Add terminal stream */
	stream.stream_id = STREAM_ID_STATUS_MESSAGE;
	stream.type = ADP_STREAM_UINT_8;
	stream.mode = ADP_STREAM_OUT;
	stream.state = ADP_STREAM_ON;
	
	adp_configure_stream(&stream, "Status messages");
	
	adp_configure_terminal(&terminal_config, "Status terminal");
	
	/* Connect stream and terminal */
	struct adp_msg_add_stream_to_terminal conf_stream = {
		.terminal_id = STREAM_ID_STATUS_MESSAGE,
		.stream_id = STREAM_ID_STATUS_MESSAGE,
		.mode = 0xFF,
		.text_color = {ADP_COLOR_RED},
		.tag_text_color = {ADP_COLOR_BLACK}
	};
	adp_add_stream_to_terminal(&conf_stream, "Status messages");
}
/****************************************************************************/
void adp_window_add_dashboard_control(void)
{
	/* Add a dashboard */
	struct adp_msg_conf_dashboard dashboard_con = {
		.dashboard_id = DASHBOARD_ID_CPU_USAGE,
		.color = {ADP_COLOR_WHITE},
		.height = 300,
	};
	adp_add_dashboard(&dashboard_con, "Genel");
	
	struct adp_msg_configure_stream stream;
	struct adp_conf_add_stream_to_element element_stream = {
		.dashboard_id = DASHBOARD_ID_CPU_USAGE,
	};
	
	/* Add segment to dashboard, use light sensor stream */
	/* Add segment stream(light sensor ADC value) */
	stream.stream_id = STREAM_ID_CPU_USAGE;
	stream.type = ADP_STREAM_UINT_8;
	stream.mode = ADP_STREAM_OUT;
	stream.state = ADP_STREAM_ON;
	adp_configure_stream(&stream, "CPU Usage Value");
	
	/* Add a graph to dashboard */
	struct adp_msg_conf_dashboard_element_graph graph_value = {
		.dashboard_id = DASHBOARD_ID_CPU_USAGE,
		.element_id = ELEMENT_ID_GRAPH,
		.z_index = 0,
		.x = 310,
		.y = 10,
		.width = 320,
		.height = 240,
		.element_type = ADP_ELEMENT_TYPE_GRAPH,
		.title_color = {ADP_COLOR_WHITE},
		.background_color = {ADP_COLOR_BLACK},
		.graph_background_color = {ADP_COLOR_BLACK},
		.plot_count = 1,
		.x_min = 0,
		.x_max = 10,
		.y_min = 0,
		.y_max = 255,
		.mode.bit.mouse = 0,
		.mode.bit.fit_graph = 0,
	};
	adp_add_graph_to_dashboard(&graph_value, "CPU Usage");
	
	element_stream.element_id = ELEMENT_ID_GRAPH;
	element_stream.stream_id = STREAM_ID_CPU_USAGE;
	adp_add_stream_to_element(&element_stream);
	
}
/****************************************************************************/
void Cpu_Measurement(void)
{
	static uint8_t cpu_timer_t;
	static uint32_t cpu_avg_t,cpu_cycle;
	uint8_t temp,cpu_timer;

	cpu_timer=CPU_TIMER_COUNTER;// timer counter hep buradan okunur

	if(cpu_measurement_restart==false)// eger resetleme olursa yada ilk baslangýc ise ilk deger temp de tutulacagý için flag kullanýlýr
	{
		if(cpu_timer >= cpu_timer_t)
		{
			temp= cpu_timer - cpu_timer_t;
		}
		else
		{
			temp= cpu_timer_t - cpu_timer;
			temp= CPU_TIMER_PERIOD - temp;
		}
		if(temp < cpu_min)
		{
			cpu_min=temp;
		}
		if(temp > cpu_max)
		{
			cpu_max=temp;
		}

		cpu_avg_t += temp;

		if(++cpu_cycle > CPU_AVG_CYCLE)
		{
			cpu_avg = cpu_avg_t/cpu_cycle;
			cpu_avg_t=0;
			cpu_cycle=0;
		}
	}
	else// registerlarýn reset durumlarý
	{
		cpu_min=CPU_TIMER_PERIOD;
		cpu_max=0;
		cpu_avg=0;
		cpu_avg_t=0;
		cpu_cycle=0;
		cpu_measurement_restart=false;
	}
	cpu_timer_t=cpu_timer;
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void Cpu_Hardware_Init(void)
{
	// iþlemci 16 Mhz ile koþmaktadýr
	// usart 115200 e ayarlanmýþtýr.
	sysclk_init();

	sysclk_enable_peripheral_clock(&UCSR0A);// usart peripheral clock enable
	sysclk_enable_peripheral_clock(&TCCR2A);// timer2 peripheral clock enable
	sysclk_enable_peripheral_clock(&TCCR1A);// timer1 peripheral clock enable
	sysclk_enable_peripheral_clock(&TCCR0A);// timer0 peripheral clock enable
	
	board_init();
	
	cli();
	uart_init();
	timers_init();
	sei();
	
	Calc_WeekDay(&real_date);
	
	xdev_out(uart_putchar);

	gpio_set_pin_high(BUZZER_PIN);
	delay_ms(5);
	gpio_set_pin_low(BUZZER_PIN);
	delay_ms(100);
	gpio_set_pin_high(BUZZER_PIN);
	delay_ms(5);
	gpio_set_pin_low(BUZZER_PIN);
	delay_ms(100);
	gpio_set_pin_high(BUZZER_PIN);
	delay_ms(5);
	gpio_set_pin_low(BUZZER_PIN);
	
	//button_2=ioport_get_pin_level(BUTTON_2);
	//uart_putstr(test_string,strlen(test_string));
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
