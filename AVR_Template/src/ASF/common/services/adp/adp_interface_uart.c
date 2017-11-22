/**
 * Coþkun ERGAN
 * 
 * \file
 *
 * \brief ADP service uart implementation 09.11.2017
 *
 *
 */

#include <compiler.h>

#include <asf.h>
#include "adp_interface.h"

void uart_putchar(uint8_t data);

static enum status_code adp_interface_send(uint8_t* tx_buf, uint16_t length)
{
	enum status_code status;
	
	while(length--)
	{
		uart_putchar(*tx_buf++);	 
	}
	
	return status;
}

/**
* \brief Read response on UART from PC
*/
enum status_code adp_interface_read_response(uint8_t* rx_buf, uint16_t length)
{
	uint16_t i=0;
	uint8_t get_buffer_size;

	if(ring_buffer_is_empty(&ring_buffer_in))
	{
		return ERR_TIMEOUT;
	}	
	//--------------------
	if(ring_buffer_in.write_offset < ring_buffer_in.read_offset )  
	{
		get_buffer_size =  ring_buffer_in.read_offset   -    ring_buffer_out.write_offset;
		get_buffer_size =  ring_buffer_out.size          -    get_buffer_size;
	}
	else
	{
		get_buffer_size =  ring_buffer_in.write_offset  -    ring_buffer_in.read_offset;
	}
	//--------------------
	if(get_buffer_size-1 != length)
	{
		return ERR_BUSY;
	}
	//--------------------
	for(i=0;i<=length;i++)
	{
			*rx_buf=ring_buffer_get(&ring_buffer_in);
			rx_buf++;
	}
	//--------------------
	return STATUS_OK;
}

/**
* \brief Sends and reads protocol packet data byte on UART
*/
void adp_interface_transceive_procotol(uint8_t* tx_buf, uint16_t length, uint8_t* rx_buf)
{
	adp_interface_send(tx_buf, length);	
}
