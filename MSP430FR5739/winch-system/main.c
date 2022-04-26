#include <msp430.h> 
#include "pawl.h"
#include "spi.h"
#include "gearmotor.h"
#include "debug.h"
#include "uart.h"
#include "motor.h"


void init(void);

/**
 * main.c
 */
int main(void)
    {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	init();
	V_PRINTF("MAIN\r\n");
	int err = 0;
	int position = 130;

	char command[2] = "";

	move_pawl(REST);
	__delay_cycles(1000000);
	move_pawl(CLOCKWISE);

	while (1) {


	    //-- 5000 ms
	    __delay_cycles(5000000);
	}

	return 0;
}

void init(void) {
    init_spi();
    init_gearmotor();
    init_uart();
    init_Main_Motor();

    __enable_interrupt();
}
