/**
 * \file
 *
 * \brief User board initialization template
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include <asf.h>
#include <board.h>
#include <conf_board.h>

void board_init(void)
{
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
	gpio_configure_pin(BUZZER_PIN,IOPORT_DIR_OUTPUT|IOPORT_INIT_LOW);
	
	gpio_configure_pin(LED1_PIN,IOPORT_DIR_OUTPUT|IOPORT_INIT_LOW);
	
	gpio_configure_pin(RED_LED_PIN,IOPORT_DIR_OUTPUT|IOPORT_INIT_LOW);
	gpio_configure_pin(GREEN_LED_PIN,IOPORT_DIR_OUTPUT|IOPORT_INIT_LOW);
	
	ioport_configure_pin(BUTTON_1,IOPORT_DIR_INPUT | IOPORT_PULL_UP);
	ioport_configure_pin(BUTTON_2,IOPORT_DIR_INPUT | IOPORT_PULL_UP);
	
}
