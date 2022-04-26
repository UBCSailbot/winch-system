#include <msp430.h> 
#include "motor.h"
#include "debug.h"
#include "uart.h"
#include "spi.h"

//-- Types of tests
#define MOVE_MOTOR 1
#define MOTOR_POS  2

//-- CONTROL
#define TEST_SEL MOTOR_POS
#define INCREMENT 11

void test_mainMotorIncrement(void);
void test_mainMotorPosition(void);

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	init_uart();
	init_spi();
	init_Main_Motor();

	V_PRINTF("MAIN \r\n");

	__enable_interrupt();

	switch(TEST_SEL) {
	case MOVE_MOTOR:
	    test_mainMotorIncrement();
	    break;
	case MOTOR_POS:
	    test_mainMotorPosition();
	    break;
	}

	while(1);
}

void test_mainMotorIncrement(void) {

    V_PRINTF("TEST main motor movement \r\n");

    while(1) {
        if(incrementMainMotor(ANTICLOCKWISE, INCREMENT) < 0) {
            V_PRINTF("UNSUCCESSFUL\r\n");
        } else {
            V_PRINTF("SUCCESSFUL\r\n");
        }

        while (isMotorOn());

        __delay_cycles(2500000);

        if(incrementMainMotor(CLOCKWISE, INCREMENT) < 0) {
            V_PRINTF("UNSUCCESSFUL\r\n");
        } else {
            V_PRINTF("SUCCESSFUL\r\n");
        }

        while (isMotorOn());

        __delay_cycles(2500000);
    }

}

void test_mainMotorPosition(void) {
    V_PRINTF("TEST main motor position \r\n");
    int err = setMainMotorPosition(170);

    __delay_cycles(1000000);

    setMainMotorPosition(185);

    __delay_cycles(1000000);

    setMainMotorPosition(200);

    __delay_cycles(1000000);

    V_PRINTF("ERROR: %d", err);
}




