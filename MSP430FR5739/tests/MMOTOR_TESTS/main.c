#include <msp430.h> 
#include "motor.h"
#include "debug.h"

int cur_direction;

//-- Types of tests
#define MOVE_MOTOR 1

//-- CONTROL
#define TEST_SEL MOVE_MOTOR
#define VERBOSE 1
#define INCREMENT 2000

#define V_PRINT(str) if(VERBOSE) {putString(str);}


void test_mainMotorMovement(void);

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	init_UART_DBG();
	init_Main_Motor();

	__enable_interrupt();

	switch(TEST_SEL) {
	case MOVE_MOTOR:
	    test_mainMotorMovement();
	    break;
	}

	while(1);

	return 0;
}

void test_mainMotorMovement(void) {

    V_PRINT("Test Main Motor Movement - Clockwise\r\n");
    cur_direction = CLOCKWISE;

    if(startMainMotor(INCREMENT) < 0) {
        V_PRINT("UNSUCCESSFUL\r\n");
    } else {
        V_PRINT("SUCCESSFUL\r\n");
    }
}


