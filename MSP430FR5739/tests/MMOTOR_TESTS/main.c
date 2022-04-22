#include <msp430.h> 
#include "motor.h"
#include "debug.h"
#include "uart.h"

//-- Types of tests
#define MOVE_MOTOR 1

//-- CONTROL
#define TEST_SEL MOVE_MOTOR
#define INCREMENT 2000

void test_mainMotorMovement(void);

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	init_uart();
	init_Main_Motor();

	__enable_interrupt();

	switch(TEST_SEL) {
	case MOVE_MOTOR:
	    test_mainMotorMovement();
	    break;
	}

	while(1);
}

void test_mainMotorMovement(void) {

    V_PRINTF("TEST main motor movement \r\n");

    if(incrementMainMotor(CLOCKWISE, INCREMENT) < 0) {
        V_PRINTF("UNSUCCESSFUL\r\n");
    } else {
        V_PRINTF("SUCCESSFUL\r\n");
    }
}


