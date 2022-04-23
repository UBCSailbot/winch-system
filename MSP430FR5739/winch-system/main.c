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

	char command[2] = "";
	while (1) {
	    if (isReady()) {
	        getCommand(command);
	        err = setMainMotorPosition((int) command[0]);
	        V_PRINTF("ERROR: %d\r\n", err);
	        clearReady();
	    }

	    //-- 10 ms
	    __delay_cycles(10000);
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
