#include <msp430.h> 
#include "pawl.h"
#include "spi.h"
#include "gearmotor.h"
#include "debug.h"
#include "uart.h"
#include "motor.h"
#include "statemachine.h"



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
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    init();

    V_PRINTF("MAIN \r\n");

    switch(test_sel)
    {
    case STATE_TRANSITION:
        test_state_transition();
        break;

    case WATCH_INPUT_MODE:
        V_PRINTF("WATCH_INPUT_MODE TEST \r\n");
        switch_to_echo_mode();
        break;

    default:
        V_PRINTF("INVALID TEST \r\n");
        break;
    }

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

void test_state_transition(void)
{
    V_PRINTF("--- State transition test ---\r\n");
    handle_commands();
}
