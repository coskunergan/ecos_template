/**
 * \file
 *
 * \brief User board configuration template
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

#define BUZZER_PIN IOPORT_CREATE_PIN(PORTC,2)

#define LED1_PIN  IOPORT_CREATE_PIN(PORTB,5)
#define RED_LED_PIN  IOPORT_CREATE_PIN(PORTD,2)
#define GREEN_LED_PIN  IOPORT_CREATE_PIN(PORTD,3)

#define BUTTON_1   IOPORT_CREATE_PIN(PORTC,0)
#define BUTTON_2   IOPORT_CREATE_PIN(PORTC,1)




#endif // CONF_BOARD_H
