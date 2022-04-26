#include <msp430.h> 
#include "pawl.h"
#include "spi.h"
#include "gearmotor.h"
#include "debug.h"
#include "uart.h"
#include "motor.h"
#include "statemachine.h"


void init(void);

/**
 * main.c
 */
int main(void)
    {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	init();
	V_PRINTF("MAIN\r\n");

	while (1) {

	}
}

void init(void) {
    init_spi();
    init_gearmotor();
    init_uart();
    init_Main_Motor();

    __enable_interrupt();
}
