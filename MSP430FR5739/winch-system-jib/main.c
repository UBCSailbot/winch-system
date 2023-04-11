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
	WDTCTL = WDT_ARST_1000;	// start WDT

	init();

	V_PRINTF("MAIN\r\n");

	handle_commands();
	for(;;);
}

void init(void) {
    init_spi();
    init_gearmotor();
    init_uart();

    //-- Always call this last
    init_Main_Motor();

    __enable_interrupt();
}
