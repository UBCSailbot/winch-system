#include <msp430.h> 
#include "motor.h"
#include "debug.h"
#include "uart.h"
#include "spi.h"

//-- Types of tests
enum test_type
{
    MOVE_MOTOR,
    MOTOR_POS,
    GET_POS
};

//-- SELECT TEST TO BE PERFORMED HERE
enum test_type test_sel = MOVE_MOTOR;

//-- CONTROL
#define INCREMENT 44

void test_mainMotorIncrement(void);
void test_mainMotorPosition(void);
void test_getCurrentPosition(void);

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

	switch(test_sel) {
	case MOVE_MOTOR:
	    test_mainMotorIncrement();
	    break;
	case MOTOR_POS:
	    test_mainMotorPosition();
	    break;
	case GET_POS:
	    test_getCurrentPosition();
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

    int err = setMainMotorPosition(360);

    V_PRINTF("ERROR: %d", err);
}

void test_getCurrentPosition(void) {
    while (1) {
        V_PRINTF("Current Position: %d\r\n", getCurrentPosition());

        //-- delay one sec
        __delay_cycles(1000000);
    }

}



