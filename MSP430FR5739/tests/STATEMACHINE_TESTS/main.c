#include <msp430.h> 
#include "debug.h"
#include "uart.h"
#include "statemachine.h"
#include "spi.h"

//-- Types of tests
enum test_type
{
    STATE_TRANSITION,
    WATCH_INPUT_MODE
};

//-- SELECT TEST TO BE PERFORMED HERE
enum test_type test_sel = STATE_TRANSITION;


void init(void);
void test_state_transition(void);

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	init();

	V_PRINTF("MAIN \r\n");

	switch(test_sel)
	{
	case STATE_TRANSITION:
	    test_state_transition();
	    break;

	case WATCH_INPUT_MODE:
        #define ECHO_UART
	    break;

	default:
	    V_PRINTF("INVALID TEST \r\n");
	    break;
	}

	for(;;);

	return 0;
}

void init(void) {
    init_spi();
    init_uart();

    __enable_interrupt();
}

void test_state_transition(void)
{
    V_PRINTF("--- State transition test ---\r\n");
    handle_commands();
}
